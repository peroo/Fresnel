#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include <vector>

class AudioEncoder {
    public:
        virtual bool start() const = 0;
        bool init(int channels, float quality);
        void feed(int count, const int *buffer);
        int  read(int pos, int max, char *buffer);
        void feedingDone();

    protected:
        void saveData(unsigned char *_buffer, int count);
        virtual void close() const;

        std::vector< std::vector<int> > buffer;
        std::vector<unsigned char> data;
        float quality;
        int  channels;
        bool feeding;
        bool active;
};

#endif
