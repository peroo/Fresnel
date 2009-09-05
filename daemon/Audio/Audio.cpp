#include "Audio.h"
#include "Metadata.h"
#include "FLACDecoder.h"
#include "MP3Decoder.h"
#include "VorbisEncoder.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

void killDecode(AudioDecoder *decoder)
{
    delete decoder;
    decoder = NULL;
}
void decode(AudioDecoder *decoder)
{
    try {
        decoder->start();
    }
    catch(boost::thread_interrupted &exception) {}
    killDecode(decoder);
}

void killEncode(AudioEncoder *encoder)
{
    delete encoder;
    encoder = NULL;
}
void encode(AudioEncoder *encoder)
{
    try {
        encoder->start();
    }
    catch(boost::thread_interrupted &exception) {}
    killEncode(encoder);
}

Audio::~Audio()
{
    if(decoder)
        decThread.interrupt();
    if(encoder)
        encThread.interrupt();
    delete metadata;
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
    decThread = boost::thread(decode, decoder);
    encThread = boost::thread(encode, encoder);

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
    boost::mutex::scoped_lock lock(mutex);
    data.insert(data.end(), buffer, buffer + count);
}

void Audio::encodingFinished()
{
    encoding = false;
}

int Audio::read(int pos, int max, char *buffer)
{
    while(pos == data.size() && encoding)
        usleep(10000);

    boost::mutex::scoped_lock lock(mutex);
    int count = pos + max > data.size() ? data.size() - pos : max;
    memcpy(buffer, &data[pos], count);

    if(count > 0 || encoding)
        return count;
    else 
        return -1;
}
