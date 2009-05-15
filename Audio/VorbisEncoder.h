#ifndef VORBISENCODER_H
#define VORBISENCODER_H

#include "AudioEncoder.h"

#include <vorbis/vorbisenc.h>

class VorbisEncoder: public AudioEncoder {
    public:
        bool start();
        
    private:
        void close();

        ogg_stream_state os;
        ogg_page og;
        ogg_packet op;
        vorbis_info vi;
        vorbis_comment vc;
        vorbis_dsp_state vd;
        vorbis_block vb;

        bool eos;
};

#endif
