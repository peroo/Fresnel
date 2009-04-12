#ifndef IMAGE_H
#define IMAGE_H

#include <string>

enum imgFormat {JPEG, PNG};
enum interpolation {BILINEAR, BICUBIC};

struct pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

class Image {
    public:
        Image() {}
        bool open(const std::string filename);
        bool write(const char *filename, int format);
        bool resize(int x, int y, const int interpolation);
    private:
        int width;
        int height;
        pixel *bitmap;

        void decodeJPEG(const std::string filename);
        void decodePNG(const std::string filename);

        void writeJPEG(const char *filename);

        inline double weighted_sum(const double dx, const double dy, const int p0, const int p1, const int p2, const int p3);
        inline double cubic_spline_fit(double dx, double pt0, double pt1, double pt2, double pt3);
        void bilinear(int sx, int sy, double xfrac, double yfrac, pixel *point);
        void bicubic(int sx, int sy, double xfrac, double yfrac, pixel *point, int newWidth, int newHeight);
        unsigned char clamp(double num);
};

#endif
