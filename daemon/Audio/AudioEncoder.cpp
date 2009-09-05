#include "AudioEncoder.h"

#include <boost/thread/mutex.hpp>
#include <iostream>

bool AudioEncoder::start() {}

bool AudioEncoder::init(int _channels, float _quality) {
    channels = _channels;
    quality = _quality;
    buffer.resize(channels);

    return true;
}

void AudioEncoder::feed(int count, const int * const _buffer[]) {
    boost::mutex::scoped_lock lock(mutex);
    for(int i = 0; i < channels; i++) {
        for(int j=0; j < count; j++) {
            buffer[i].push_back(_buffer[i][j] / 32768.f);
        }
    }
}

void AudioEncoder::feed(int count, unsigned char *_buffer) {
    int c = 0;
    short *bu = reinterpret_cast<short*>(_buffer);
    boost::mutex::scoped_lock lock(mutex);
    for(int i=0; i < count; i++) {
        for(int j=0; j < 2; j++) {
            //buffer[j].push_back(((_buffer[c+1]<<8)|(0x00ff&(int)_buffer[c])) / 32768.f);
            buffer[j].push_back(bu[c] / 32768.f);
            c++;
        }
    }
}

void AudioEncoder::feedingDone() {
    feeding = false;
}

bool AudioEncoder::isFeeding() {
    return feeding;
}
