#ifndef FLAC_H
#define FLAC_H

#include <FLAC++/decoder.h>

class OggEncode;

class FLACDecoder: public FLAC::Decoder::File {
    public:
        FLACDecoder(void (*func)(const FLAC__int32 * const buffer[], int num, OggEncode *ogg), OggEncode *ogg_): FLAC::Decoder::File(), sink(func), ogg(ogg_) {}
        void close();
    private:
        void (*sink)(const FLAC__int32 * const buffer[], int num, OggEncode *ogg);
        OggEncode *ogg;

        FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
        void metadata_callback(const ::FLAC__StreamMetadata *metadata);
        void error_callback(FLAC__StreamDecoderErrorStatus status);
};

#endif
