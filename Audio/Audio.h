#ifndef AUDIO_H
#define AUDIO_H

#include "../Resource.h"
#include "AudioDecoder.h"
#include "AudioEncoder.h"

#define SLING_FLAC 0
#define SLING_VORBIS 1
#define SLING_MP3 2

class Metadata;

class Audio : public Resource {
    public:
        virtual bool load(int output);
        int read(int pos, int max, char *buffer);
        Metadata* getMetadata();

    private:
        AudioDecoder *decoder;
        AudioEncoder *encoder;
};

#endif
