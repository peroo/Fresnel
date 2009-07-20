#ifndef AUDIO_H
#define AUDIO_H

#include "../Resource.h"
#include "AudioDecoder.h"
#include "AudioEncoder.h"

#include <pthread.h>

#define SLING_FLAC 0
#define SLING_VORBIS 1
#define SLING_MP3 2

class Metadata;

class Audio : public Resource {
    public:
        Audio() : encoder(NULL), decoder(NULL) {}
        ~Audio();
        bool load(int output);
        bool load();
        std::string getMimetype();
        int read(int pos, int max, char *buffer);
        void saveData(unsigned char *buffer, int count);
        void encodingFinished();
        Metadata* getMetadata();

    private:
        std::string mimetype;
        AudioDecoder *decoder;
        AudioEncoder *encoder;
        pthread_mutex_t mutex;
        bool encoding;
        std::vector<unsigned char> data;
};

#endif
