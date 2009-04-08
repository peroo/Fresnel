#ifndef IMAGE_H
#define IMAGE_H

#include <string>

enum imgFormat {JPEG, PNG};

class Image {
    public:
        Image() {}
        bool open(const std::string filename);
        bool write(int type, const char *filename);
        bool convert(int format);
        bool resize(int x, int y);
    private:
        int width;
        int height;
        unsigned char *bitmap;

        void decodeJPEG(const std::string filename);
        void decodePNG(const std::string filename);

        void writeJPEG(const char *filename);

        void bilinear();
        void bicubic();
};

#endif
