#ifndef IMAGE_H
#define IMAGE_H

#include "../Resource.h"

#include <string>
#include <vector>

enum imgFormat {JPEG, PNG};
enum interpolation {BILINEAR, BILINEAR_SSE, BILINEAR_SSE_FLOAT, BICUBIC, BOX_FILTER, BOX_FILTER2};

struct pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
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
        pixel **bitmap;
        std::vector<char> output;

        bool open();
        bool resize(int x, int y, const int interpolation);
        //std::string scanBarcode();

        bool decodeJPEG(const std::string filename);
        //bool decodePNG(const std::string filename);

        void encodeJPEG();



        /*inline int weighted_sum(const float dx, const float dy, const int p0, const int p1, const int p2, const int p3);
        inline float cubic_spline_fit(float dx, int pt0, int pt1, int pt2, int pt3);
        void bilinear(const int sx, const int sy, const float xfrac, const float yfrac, pixel *point);
        void bilinear_sse(const int,const int, const int,const int,pixel*);
        void bilinear_sse_float(const int,const int, const float,const float,pixel*);
        void bicubic(int sx, int sy, float xfrac, float yfrac, pixel *point, int newWidth, int newHeight);*/
        unsigned char clamp(double num);
        unsigned char clamp(float num);

        void box2_horizontal(uint16_t* temp, int x, int xcount);



        void box2_vertical(uint16_t** temp, int y, int ystart, int ycount, int xcount, int newHeight);

    
        //void box_average_h(pixel* temp, int ystart, int ycount, int xstep, int newWidth);
        //void box_average_h2(pixel* temp, int ystart, int count, float xstep);
};

#endif
