#ifndef AUDIO_H
#define AUDIO_H

#include "../Resource.h"
#include "AudioDecoder.h"
#include "AudioEncoder.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#define SLING_FLAC 0
#define SLING_VORBIS 1
#define SLING_MP3 2

class Metadata;

class Audio : public Resource {
    public:
        Audio() : encoder(NULL), decoder(NULL), metadata(NULL) {}
        ~Audio();
        bool load(int output);
        bool load();
        bool done();
        std::string getMimetype();
        int getSize();
        int read(int pos, int max, char *buffer);
        void saveData(unsigned char *buffer, int count);
        void encodingFinished();
        Metadata* getMetadata();

    private:
        Metadata *metadata;
        std::string mimetype;
        AudioDecoder *decoder;
        AudioEncoder *encoder;
        boost::mutex mutex;
        boost::thread decThread;
        boost::thread encThread;
        bool encoding;
        std::vector<unsigned char> data;
};

#endif
