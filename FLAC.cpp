#include "FLAC.h"
#include "OggEncode.h"

#include <stdio.h>
#include <iostream>

FLAC__StreamDecoderWriteStatus FLACDecoder::write_callback(const FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    sink(buffer, frame->header.blocksize, ogg);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FLACDecoder::metadata_callback(const ::FLAC__StreamMetadata *metadata)
{
    //metadata->data.vorbis_comment.comments;
}

void FLACDecoder::error_callback(FLAC__StreamDecoderErrorStatus status)
{
    fprintf(stderr, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

void FLACDecoder::close()
{
    ogg->closeStream();
    delete ogg;
}
