#include "VorbisEncoder.h"

#include <iostream>
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <google/profiler.h>


bool VorbisEncoder::start()
{
    data.reserve(5000000);

    eos = false;
    feeding = true;
    active = true;

    vorbis_info_init(&vi);
    if(vorbis_encode_init_vbr(&vi, channels, 44100, quality)) {
        // throw error
        return false;
    }

    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "Slingshot v0.01");

    srand(time(NULL));
    ogg_stream_init(&os, rand());

    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header);
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);

        while(!eos) {
            int result = ogg_stream_flush(&os, &og);
            if(result == 0) break;

            saveData(og.header, og.header_len);
            saveData(og.body, og.body_len);
        }
    }

    int i, j, count, size;
    int pos = 0;
    int max = 65536;
    std::vector< std::vector<int> >::iterator channelIter;
    std::vector<int>::iterator sampleIter;

    time_t start = time(NULL);
    ProfilerStart("/home/peroo/server/out.prof");
    while(feeding || (buffer.front().size() - pos) > 0) {
        size = buffer.front().size();
        count = size - pos < max ? size - pos : max;
            
        float **buf = vorbis_analysis_buffer(&vd, count);
    
        pthread_mutex_lock(&mutex);
        for(i = 0; i < channels; ++i) {
            for(j = 0; j < count; ++j) {
                buf[i][j] = buffer[i][pos + j] / 32768.f;
            }
        }
        pos += count;
        pthread_mutex_unlock(&mutex);

        vorbis_analysis_wrote(&vd, j);

        while(vorbis_analysis_blockout(&vd, &vb) == 1) {
            vorbis_analysis(&vb, NULL);
            vorbis_bitrate_addblock(&vb);
    
            while(vorbis_bitrate_flushpacket(&vd, &op)) {
                ogg_stream_packetin(&os, &op);
    
                while(!eos) {
                    int result = ogg_stream_pageout(&os, &og);
                    if(result==0) break;
                    saveData(og.header, og.header_len);
                    saveData(og.body, og.body_len);
    
                    if(ogg_page_eos(&og)) eos = 1;
                }
            }
        }
    }
    ProfilerStop();
    time_t end = time(NULL);

    std::cout << "encoding finished in " << end - start << "s." << std::endl;

    active = false;
    close();

    return true;
}

void VorbisEncoder::close() {
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
}
