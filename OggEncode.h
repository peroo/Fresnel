#ifndef OGG_H
#define OGG_H

#include <vorbis/vorbisenc.h>
#include <cstdio>

class OggEncode {
    public:
        OggEncode() {}
        bool init();
        void abort() {}
        bool addStream();
        bool feed(const int * const buffer[], int num);
        void closeStream();
    private:
        ogg_stream_state os;
        ogg_page og;
        ogg_packet op;
        vorbis_info vi;
        vorbis_comment vc;
        vorbis_dsp_state vd;
        vorbis_block vb;

        FILE *output;
        int eos;
};

#endif
