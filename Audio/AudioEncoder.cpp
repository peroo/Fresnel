#include "AudioEncoder.h"

#include <pthread.h>
#include <iostream>

bool AudioEncoder::start() {}

bool AudioEncoder::init(int _channels, float _quality) {
    channels = _channels;
    quality = _quality;
    buffer.resize(channels);
    pthread_mutex_init(&mutex, NULL);

    return true;
}

void AudioEncoder::feed(int count, const int * const _buffer[]) {
    int i;
    pthread_mutex_lock(&mutex);
    for(i = 0; i < channels; i++) {
        buffer[i].insert(buffer[i].end(), _buffer[i], _buffer[i] + count);
    }
    pthread_mutex_unlock(&mutex);
}

void AudioEncoder::feedingDone() {
    feeding = false;
}

bool AudioEncoder::isFeeding() {
    return feeding;
}
