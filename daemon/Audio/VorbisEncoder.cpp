#include "VorbisEncoder.h"
#include "Audio.h"
#include "Metadata.h"

#include <iostream>
#include <algorithm>

#include <map>

VorbisEncoder::~VorbisEncoder()
{
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
}

bool VorbisEncoder::start()
{
    eos = false;
    feeding = true;
    _encoding = true;

    vorbis_info_init(&vi);
    if(vorbis_encode_init_vbr(&vi, channels, 44100, quality)) {
        // throw error
        return false;
    }

    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "Fresnel v0.01");
    Metadata meta = parent->getMetadata();
    std::map<const char*, std::string> metaArray = meta.getFields();

    for(auto it = metaArray.begin(); it != metaArray.end(); ++it) {
        vorbis_comment_add_tag(&vc, it->first, it->second.c_str());
    }

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

            parent->saveData(og.header, og.header_len);
            parent->saveData(og.body, og.body_len);
        }
    }

    int i, count, size;
    int pos = 0;
    int max = 65536;

    //TODO: Quickfix to prevent encoder from preempting decoder.
    //      0b buffer causes the encoder to fall on its face.
    while(buffer.front().size() == 0) {
        std::cout << "Sleeping..." << std::endl;
#ifdef _WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }

    while(feeding || (buffer.front().size() - pos) > 0) {
        size = buffer.front().size();
        count = size - pos < max ? size - pos : max;
            
        float **buf = vorbis_analysis_buffer(&vd, count);
    
        inputMutex.lock();
        for(i = 0; i < channels; ++i) {
            std::copy(buffer[i].begin() + pos, buffer[i].begin() + pos+count, buf[i]);
        }
        inputMutex.unlock();
        pos += count;

        vorbis_analysis_wrote(&vd, count);

        while(vorbis_analysis_blockout(&vd, &vb) == 1) {
            vorbis_analysis(&vb, NULL);
            vorbis_bitrate_addblock(&vb);
    
            while(vorbis_bitrate_flushpacket(&vd, &op)) {
                ogg_stream_packetin(&os, &op);
    
                while(!eos) {
                    int result = ogg_stream_pageout(&os, &og);
                    if(result==0) break;

                    parent->saveData(og.header, og.header_len);
                    parent->saveData(og.body, og.body_len);
    
                    if(ogg_page_eos(&og)) eos = 1;
                }
            }
        }
    }

    _encoding = false;
    parent->encodingFinished();

    return true;
}
