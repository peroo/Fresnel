#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include <vector>
#include <pthread.h>

class AudioEncoder {
    public:
        virtual bool start() = 0;
        bool init(int channels, float quality);
        void feed(int count, const int * const buffer[]);
        int  read(int pos, int max, char *buffer);
        void feedingDone();

    protected:
        void saveData(unsigned char *_buffer, int count);
        virtual void close() = 0;

        std::vector< std::vector<int> > buffer;
        std::vector<unsigned char> data;
        pthread_mutex_t mutex;
        float quality;
        int  channels;
        bool feeding;
        bool active;
};

#endif
