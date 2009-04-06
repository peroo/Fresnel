#include "OggEncode.h"

#include <iostream>
#include <cstdlib>

bool OggEncode::init() {
    vorbis_info_init(&vi);
    if(vorbis_encode_init_vbr(&vi, 2, 44100, .2)) {
        // throw error
        return false;
    }

    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    output = fopen("out.ogg", "wb");

    return true;
}

// TODO: Add tags to stream
bool OggEncode::addStream() {
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "Slingshot v0.0");

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
        
        fwrite(og.header, 1, og.header_len, output);
        fwrite(og.body, 1, og.body_len, output);
    }
    return true;
}

bool OggEncode::feed(const int * const buff[], int num) {
    //std::cout << "feed" << num << std::endl;
    float **buffer = vorbis_analysis_buffer(&vd, num);
    
    int j;
    for(j=0; j < num; j++) {
        buffer[0][j] = ((buff[0][j]) | (0x00ff&(int)buff[0][j])) / 32768.f;
        buffer[1][j] = ((buff[1][j]) | (0x00ff&(int)buff[1][j])) / 32768.f;
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
                fwrite(og.header, 1, og.header_len, output);
                fwrite(og.body, 1, og.body_len, output);

                if(ogg_page_eos(&og)) eos = 1;
            }
        }
    }
    return true;
}

void OggEncode::closeStream() {
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    fclose(output);
}
