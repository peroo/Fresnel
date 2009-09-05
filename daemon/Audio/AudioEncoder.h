#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include <boost/thread/mutex.hpp>
#include <string>
#include <vector>

class Audio;

class AudioEncoder {
    public:
        AudioEncoder(std::string _mimetype, Audio* parent) : mimetype(_mimetype), parent(parent), die(false) {}
        virtual ~AudioEncoder() {};
        virtual bool start() = 0;
        bool init(int channels, float quality);
        void feed(int count, const int * const buffer[]);
        void feed(int count, unsigned char *_buffer);
        bool isFeeding();
        void feedingDone();

        const std::string mimetype;
    protected:
        std::vector< std::vector<float> > buffer;
        boost::mutex mutex;
        Audio *parent;
        float quality;
        int  channels;
        bool feeding;
        bool active;
        bool die;
};

#endif
