#ifndef OGG_H
#define OGG_H

#include <vorbis/vorbisenc.h>
#include <iostream>

class OggEncode {
    public:
        bool init();
        void abort() {}
        bool addStream();
        bool feed(const int * const buffer[], int num);
        int read(int pos, int max, char *buffer);
        void closeStream();
    private:
        ogg_stream_state os;
        ogg_page og;
        ogg_packet op;
        vorbis_info vi;
        vorbis_comment vc;
        vorbis_dsp_state vd;
        vorbis_block vb;

        std::ostream *input;
        std::istringstream *output;
        int eos;
};

#endif
