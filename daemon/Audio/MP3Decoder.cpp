#include "MP3Decoder.h"
#include "AudioEncoder.h"

#include <iostream>

MP3Decoder::~MP3Decoder() {
    /*delete buffer;
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();*/
}

bool MP3Decoder::start()
{
    size_t buffer_size = 0;
    size_t done = 0;
    int channels = 0;
    int encoding = 0;
    long rate = 0;
    int err = MPG123_OK;

    err = mpg123_init();
    if( err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL
        || mpg123_open(mh, file.c_str()) != MPG123_OK
        || mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK ) {
        //std::cout << "Trouble with mpg123:" << mh==NULL ? mpg123_plain_strerror(err) : mpg123_strerror(mh) << std::endl;
        return false;
    }

    if(encoding != MPG123_ENC_SIGNED_16) {
        std::cout << "Bad encoding: " << encoding << std::endl;
        return false;
    }

    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);
    std::cout << "Rate: " << rate << " - Channels: " << channels << std::endl;

    buffer_size = mpg123_outblock(mh);
    buffer = new unsigned char(buffer_size);

    do {
        err = mpg123_read(mh, buffer, buffer_size, &done);
        encoder->feed(done/4, buffer);
    } while(err == MPG123_OK);

    if(err != MPG123_DONE) {
        //std::cout << "Warning: Decoding ended prematurely because: " << 
        //    err == MPG123_ERR ? mpg123_strerror(mh) : mpg123_plain_strerror(err) << std::endl;
        return false;
    }

    encoder->feedingDone();

    return true;
}
