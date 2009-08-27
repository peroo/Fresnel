#include "FLACDecoder.h"
#include "AudioEncoder.h"

#include <iostream>
#include <pthread.h>

FLAC__StreamDecoderWriteStatus FLACDecoder::write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    if(die)
        pthread_exit(NULL);

    encoder->feed(frame->header.blocksize, buffer);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FLACDecoder::error_callback(FLAC__StreamDecoderErrorStatus status)
{
    std::cout << "Got error callback: " << FLAC__StreamDecoderErrorStatusString[status] << std::endl;
}

bool FLACDecoder::start()
{
    FLAC::Decoder::File::init(file.string());
    int ok = process_until_end_of_stream();
    FLAC::Decoder::File::finish();
    //std::cout << "decoding ended: " << ok << std::endl;
    encoder->feedingDone();

    return ok ? true : false;
}
