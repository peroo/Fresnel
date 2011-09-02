#ifndef FLACDECODER_H
#define FLACDECODER_H

#include "AudioDecoder.h"

#include <FLAC++/decoder.h>

class AudioEncoder;

class FLACDecoder: public AudioDecoder, FLAC::Decoder::File {
    public:
        explicit FLACDecoder(std::string path) : AudioDecoder(path) {}
        ~FLACDecoder() {}
        bool start();
    private:
        FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
        void error_callback(FLAC__StreamDecoderErrorStatus status);
};

#endif
