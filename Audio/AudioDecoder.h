#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <boost/filesystem/path.hpp>

#include <map>
#include <string>

class AudioEncoder;

class AudioDecoder {
    public:
        AudioDecoder(boost::filesystem::path _file) : file(_file) {}
        void init(AudioEncoder *_encoder);
        virtual bool start();
    protected:
        AudioEncoder *encoder;
        boost::filesystem::path file;        
};

#endif
