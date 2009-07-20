#include "Audio.h"
#include "Metadata.h"
#include "FLACDecoder.h"
#include "VorbisEncoder.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <pthread.h>
#include <ctime>
#include <cstring>

namespace fs = boost::filesystem;

void* decode(void *decoder)
{
    time_t start = time(NULL);
    AudioDecoder *dec = static_cast<AudioDecoder*>(decoder);
    dec->start();
    time_t end = time(NULL);
    std::cout << "Decoding finished in: " << end - start << "s." << std::endl;
    delete dec;
}

void* encode(void *encoder)
{
    time_t start = time(NULL);
    AudioEncoder *enc = static_cast<AudioEncoder*>(encoder);
    enc->start();
    time_t end = time(NULL);
    std::cout << "Encoding finished in: " << end - start << "s." << std::endl;
    delete enc;
}

Audio::~Audio()
{
    delete decoder;
    delete encoder;
}

bool Audio::load(int output)
{
    if(loaded)
        return true;
    else
        loaded = true;

    pthread_mutex_init(&mutex, NULL);

    //std::cout << "EXTENSION: " << extension << std::endl;
    if(extension == ".flac") {
        decoder = new FLACDecoder(path);
    }
    if(output == SLING_VORBIS) {
        std::cout << "Encoding in VORBIS..." << std::endl;
        // TODO: Use channel count from decoder
        encoder = new VorbisEncoder(this);
        encoder->init(2, 0.2);
    }

    mimetype = encoder->mimetype;
    decoder->init(encoder);

    pthread_t encThread;
    pthread_t decThread;

    encoding = true;
    pthread_create(&decThread, NULL, decode, decoder);
    pthread_create(&encThread, NULL, encode, encoder);
    encoder = NULL;
    decoder = NULL;

    return true;
}

bool Audio::load()
{
    return Audio::load(SLING_VORBIS);
}

std::string Audio::getMimetype()
{
    return mimetype;
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

void Audio::saveData(unsigned char *buffer, int count)
{
    pthread_mutex_lock(&mutex);
    data.insert(data.end(), buffer, buffer + count);
    pthread_mutex_unlock(&mutex);
}

void Audio::encodingFinished()
{
    encoding = false;
}

int Audio::read(int pos, int max, char *buffer)
{
    while(pos == data.size() && encoding)
        usleep(10000);

    pthread_mutex_lock(&mutex);
    int count = pos + max > data.size() ? data.size() - pos : max;
    memcpy(buffer, &data[pos], count);
    pthread_mutex_unlock(&mutex);

    if(count > 0 || encoding)
        return count;
    else 
        return -1;
}
