#ifndef FLAC_H
#define FLAC_H

#include <FLAC++/decoder.h>

class FLACDecoder: public FLAC::Decoder::File {
    public:
        FLACDecoder(FILE *f_, void (*func)(const FLAC__int32 * const buffer[], int num)): FLAC::Decoder::File(), f(f_), sink(func) {}
    private:
        void (*sink)(const FLAC__int32 * const buffer[], int num);
        FILE *f;

        FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
        void metadata_callback(const ::FLAC__StreamMetadata *metadata);
        void error_callback(FLAC__StreamDecoderErrorStatus status);
};

#endif
