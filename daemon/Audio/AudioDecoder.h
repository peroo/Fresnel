#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <map>
#include <string>

class AudioEncoder;

class AudioDecoder {
    public:
        explicit AudioDecoder(std::string _file) : file(_file), die(false) {}
        virtual ~AudioDecoder() {};
        void init(AudioEncoder *_encoder);
        virtual bool start() = 0;
    protected:
        AudioEncoder *encoder;
        std::string file;        
        bool die;
};

#endif
