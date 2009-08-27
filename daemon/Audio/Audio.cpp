#include "Audio.h"
#include "Metadata.h"
#include "FLACDecoder.h"
#include "MP3Decoder.h"
#include "VorbisEncoder.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <pthread.h>
#include <ctime>
#include <cstring>

namespace fs = boost::filesystem;

void killDecode(void *decoder)
{
    AudioDecoder *dec = static_cast<AudioDecoder*>(decoder);
    delete dec;
    dec = NULL;
}
void* decode(void *decoder)
{
    pthread_cleanup_push(killDecode, decoder);
    AudioDecoder *dec = static_cast<AudioDecoder*>(decoder);
    dec->start();
    pthread_cleanup_pop(1);
}

void killEncode(void *encoder)
{
    AudioEncoder *enc = static_cast<AudioEncoder*>(encoder);
    delete enc;
    enc = NULL;
}
void* encode(void *encoder)
{
    pthread_cleanup_push(killEncode, encoder);
    AudioEncoder *enc = static_cast<AudioEncoder*>(encoder);
    enc->start();
    pthread_cleanup_pop(1);
}

Audio::~Audio()
{
    if(decoder)
        decoder->kill();
    if(encoder)
        encoder->kill();
    delete metadata;
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
    else if(extension == ".mp3") {
        decoder = new MP3Decoder(path);
    }
    if(output == SLING_VORBIS) {
        std::cout << "Encoding in VORBIS..." << std::endl;
        // TODO: Use channel count from decoder
        encoder = new VorbisEncoder(this);
        encoder->init(2, 0.2);
    }

    mimetype = encoder->mimetype;
    decoder->init(encoder);

    encoding = true;
    pthread_create(&decThread, NULL, decode, decoder);
    pthread_create(&encThread, NULL, encode, encoder);

    return true;
}

bool Audio::load()
{
    return Audio::load(SLING_VORBIS);
}

bool Audio::done()
{
    return !encoding;
}

std::string Audio::getMimetype()
{
    return mimetype;
}

int Audio::getSize()
{
    return -1; // TODO
    Metadata *meta = getMetadata();
    return 12500 * meta->length;
}

Metadata* Audio::getMetadata()
{
    if(metadata == NULL) {
        metadata = new Metadata();
        if(indexed) {
            //TODO: Get from db instead of file
            //meta->fetchData(fileIndex);
            metadata->loadData(path);
        }
        else {
            metadata->loadData(path);
        }
    }
    
    return metadata;
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
