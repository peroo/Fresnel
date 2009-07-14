#include "Audio.h"
#include "Metadata.h"
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
    if(loaded)
        return true;
    else
        loaded = true;

    //std::cout << "EXTENSION: " << extension << std::endl;
    if(extension == ".flac") {
        decoder = new FLACDecoder(path);
    }
    if(output == SLING_VORBIS) {
        std::cout << "Encoding in VORBIS..." << std::endl;
        // TODO: Use channel count from decoder
        encoder = new VorbisEncoder();
        encoder->init(2, 0.2);
    }

    decoder->init(encoder);

    pthread_t encThread;
    pthread_t decThread;

    pthread_create(&decThread, NULL, decode, decoder);
    pthread_create(&encThread, NULL, encode, encoder);

    return true;
}

bool Audio::load()
{
    return Audio::load(SLING_VORBIS);
}

std::string Audio::getMimetype()
{
    return encoder->mimetype;
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
