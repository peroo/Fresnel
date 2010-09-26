#include "Audio.h"
#include "Metadata.h"
#include "FLACDecoder.h"
#include "MP3Decoder.h"
#include "VorbisEncoder.h"
#include <string.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

void decode(AudioDecoder *decoder)
{
    try {
        decoder->start();
    }
    catch(boost::thread_interrupted &exception) {
        std::cout << "EXCEPTION! -- ";
    }
    delete decoder;
    decoder = NULL;
    std::cout << "Decoder finished" << std::endl;
}

void encode(AudioEncoder *encoder)
{
    try {
        encoder->start();
    }
    catch(boost::thread_interrupted &exception) {
        std::cout << "EXCEPTION! -- ";
    }
    delete encoder;
    encoder = NULL;
    std::cout << "Encoder died." << std::endl;
}

Audio::~Audio()
{
    if(decoder)
        decThread.interrupt();

    boost::mutex::scoped_lock lock(mutex);
    if(encoder)
        encThread.interrupt();
}

bool Audio::load(int output)
{
    if(loaded)
        return true;
    else
        loaded = true;

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

Metadata Audio::getMetadata()
{
    if(!metadata.loaded()) {
        if(indexed)
            metadata.fetchData(fileIndex);
        else
            metadata.loadData(path);
    }
    
    return metadata;
}

int Audio::getSize()
{
    if(encoding)
        return -1;
    else
        return -1;
}

int Audio::read(unsigned int pos, unsigned int max, char *buffer)
{
    while(pos == data.size() && encoding)
#ifdef _WIN32
        Sleep(10);
#else
        usleep(10000);
#endif

    boost::mutex::scoped_lock lock(mutex);
    int count = pos + max > data.size() ? data.size() - pos : max;
    memcpy(buffer, &data[pos], count);

    if(count > 0 || encoding)
        return count;
    else 
        return -1;
}

void Audio::saveData(unsigned char *buffer, int count)
{
    boost::this_thread::interruption_point();
    boost::mutex::scoped_lock lock(mutex);
    data.insert(data.end(), buffer, buffer + count);
}

void Audio::encodingFinished()
{
    encoding = false;
}

