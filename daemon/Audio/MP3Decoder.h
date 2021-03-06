#ifndef MP3DECODER_H
#define MP3DECODER_H

#include "AudioDecoder.h"

#include <mpg123.h>

class AudioEncoder;

class MP3Decoder: public AudioDecoder {
    public:
        explicit MP3Decoder(std::string path) : AudioDecoder(path), mh(NULL), buffer(NULL){}
        ~MP3Decoder(); 
        bool start();
    private:
        mpg123_handle *mh;
        unsigned char *buffer;
};

#endif
