#include "Audio.h"
#include "Metadata.h"
#include "AudioDecoder.h"
#include "AudioEncoder.h"
#include "FLACDecoder.h"
#include "VorbisEncoder.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <pthread.h>

namespace fs = boost::filesystem;

void* decode(void *decoder)
{
    AudioDecoder *dec = static_cast<AudioDecoder*>(decoder);
    dec->start();
}

void* encode(void *encoder)
{
    AudioEncoder *enc = static_cast<AudioEncoder*>(encoder);
    enc->start();
}

bool Audio::load(int output)
{
    if(extension == "flac") {
        decoder = new FLACDecoder(path);
    }
    if(output == SLING_VORBIS) {
        // TODO: Use channel count from decoder
        VorbisEncoder *enc = new VorbisEncoder();
    }

    decoder->init(encoder);

    pthread_t *encThread;
    pthread_t *decThread;

    pthread_create(decThread, NULL, decode, decoder);
    pthread_create(encThread, NULL, encode, encoder);

    return true;
}

int Audio::read(int pos, int max, char *buffer)
{
    return encoder->read(pos, max, buffer);
}

Metadata* Audio::getMetadata()
{
    Metadata *meta = new Metadata();
    if(indexed) {
        meta->fetchData(fileIndex);
    }
    else {
        meta->loadData(path);
    }
    
    return meta;
}
