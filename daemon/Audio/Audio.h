#ifndef AUDIO_H
#define AUDIO_H

#include "../Resource.h"
#include "AudioDecoder.h"
#include "AudioEncoder.h"
#include "Metadata.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#define SLING_FLAC 0
#define SLING_VORBIS 1
#define SLING_MP3 2

class Audio : public Resource {
    public:
        Audio() : decoder(NULL), encoder(NULL), encoding(false) {}
        ~Audio();
        bool load(int output);
        bool load();
        bool done();
        std::string getMimetype();
        int getSize();
        int read(unsigned int pos, unsigned int max, char *buffer);
        Metadata getMetadata();
        void saveData(unsigned char *buffer, int count);
        void encodingFinished();

    private:
        std::vector<unsigned char> data;
        Metadata metadata;
        std::string mimetype;
        AudioDecoder *decoder;
        AudioEncoder *encoder;
        boost::thread decThread;
        boost::thread encThread;
        boost::mutex mutex;
        bool encoding;
};

#endif
