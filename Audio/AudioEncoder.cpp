#include "AudioEncoder.h"

#include <cstring>
#include <unistd.h>

bool AudioEncoder::init(int _channels, float _quality) {
    channels = _channels;
    quality = _quality;
    buffer.resize(channels);

    return true;
}

void AudioEncoder::feed(int count, const int *_buffer) {
    int i;
    for(i = 0; i < channels; ++i) {
        buffer[i].insert(buffer[i].end(), _buffer[i], _buffer[i] + count);
    }
}

int AudioEncoder::read(int pos, int max, char *_buffer) {
    if(pos == data.size())
        sleep(1);

    int count = pos + max > data.size() ? data.size() - pos : max;
    memcpy(_buffer, &data[pos], count);

    if(feeding || count > 0)
        return count;
    else {
        close();
        return -1;
    }
}

void AudioEncoder::feedingDone() {
    feeding = false;
}

void AudioEncoder::saveData(unsigned char* _buffer, int count)
{
    data.insert(data.end(), _buffer, _buffer + count);
}
