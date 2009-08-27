#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <boost/filesystem/path.hpp>

#include <map>
#include <string>

class AudioEncoder;

class AudioDecoder {
    public:
        virtual ~AudioDecoder() {};
        AudioDecoder(boost::filesystem::path _file) : file(_file), die(false) {}
        void init(AudioEncoder *_encoder);
        virtual bool start() = 0;
        void kill();
    protected:
        AudioEncoder *encoder;
        boost::filesystem::path file;        
        bool die;
};

#endif
