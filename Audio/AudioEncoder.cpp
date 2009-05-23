#include "AudioEncoder.h"

#include <pthread.h>
#include <cstring>
#include <unistd.h>
#include <iostream>

bool AudioEncoder::start() {}
void AudioEncoder::close() {}

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

int AudioEncoder::read(int pos, int max, char *_buffer) {
    if(pos == data.size())
        sleep(1);

    int count = pos + max > data.size() ? data.size() - pos : max;
    memcpy(_buffer, &data[pos], count);

    if(feeding || count > 0)
        return count;
    else
        return -1;
}

void AudioEncoder::feedingDone() {
    std::cout << "Feeding done." << std::endl;
    feeding = false;
}

void AudioEncoder::saveData(unsigned char* _buffer, int count)
{
    data.insert(data.end(), _buffer, _buffer + count);
}
