#include "AudioDecoder.h"
#include "AudioEncoder.h"

bool AudioDecoder::start() {}

void AudioDecoder::init(AudioEncoder *_encoder)
{
    AudioDecoder::encoder = _encoder;
}
