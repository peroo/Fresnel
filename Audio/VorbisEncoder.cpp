#include "VorbisEncoder.h"

#include <ctime>
#include <cstdlib>
#include <unistd.h>


bool VorbisEncoder::start()
{
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

    while(feeding) {
        if(buffer.front().size() == 0)
            sleep(1);
            
        int count = buffer.front().size();
        float **buf = vorbis_analysis_buffer(&vd, count);
    
        int i, j;
        std::vector< std::vector<int> >::iterator channelIter;
        std::vector<int>::iterator sampleIter;
        for(channelIter = buffer.begin(), i = 0; channelIter < buffer.end(); channelIter++, i++) {
            for(sampleIter = channelIter->begin(), j = 0; sampleIter < channelIter->end(); sampleIter++, j++) {
                buf[i][j] = (*sampleIter) / 32768.f;
            }
        }

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

        for(channelIter = buffer.begin(); channelIter < buffer.end(); channelIter++) {
            channelIter->erase(channelIter->begin(), channelIter->begin() + count);
        }
    }

    active = false;

    return true;
}

void VorbisEncoder::close() {
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
}
