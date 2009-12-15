#ifndef IMAGE_H
#define IMAGE_H

#include "../Resource.h"

#include <string>
#include <vector>

enum imgFormat {JPEG, PNG};
enum interpolation {BILINEAR, BICUBIC};

struct pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

class Image : public Resource {
    public:
        Image() : bitmap(NULL) {}
        ~Image();

        bool load();
        bool load(int index);
        bool done();
        int read(unsigned int pos, unsigned int max, char *buffer);
        std::string getMimetype();
        int getSize();

    private:
        int width;
        int height;
        int bitrate;
        pixel *bitmap;
        std::vector<char> output;

        bool open();
        bool resize(int x, int y, const int interpolation);
        std::string scanBarcode();

        bool decodeJPEG(const std::string filename);
        bool decodePNG(const std::string filename);

        void encodeJPEG();

        inline int weighted_sum(const float dx, const float dy, const int p0, const int p1, const int p2, const int p3);
        inline float cubic_spline_fit(float dx, int pt0, int pt1, int pt2, int pt3);
        void bilinear(const int sx, const int sy, const float xfrac, const float yfrac, pixel *point);
        void bicubic(int sx, int sy, float xfrac, float yfrac, pixel *point, int newWidth, int newHeight);
        unsigned char clamp(double num);
};

#endif
