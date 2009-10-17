#include "AudioDecoder.h"
#include "AudioEncoder.h"

void AudioDecoder::init(AudioEncoder *_encoder)
{
    encoder = _encoder;
}
