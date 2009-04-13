#include "Image.h"

#include <jpeglib.h>
#include <png.h>
#include <zebra.h>

#include <cstdlib>
#include <iostream>

bool Image::open(const std::string filename) {
    std::string extension = filename.substr(filename.find_last_of(".")+1);

    if(extension == "jpg" || extension == "JPG" || 
       extension == "jpeg" || extension == "JPEG") {
    } else if(extension == "png" || extension == "PNG") {
        decodePNG(filename);
    }
    return true;
}

bool Image::write(const char *filename, int type) {
    switch(type) {
        case JPEG:
            writeJPEG(filename);
            break;
        case PNG:
            break;
    }
}

void Image::decodePNG(std::string filename) {
    char header[8];
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;

    FILE *fp = fopen(filename.c_str(), "rb");
    if(!fp) {
        std::cout << "Unable to open file " << filename << std::endl;
        return;
    }

    fread(header, 1, 8, fp);
    /*if(png_sig_cmp(header, 0, 8)) {
        std::cout << "File " << filename << " is not a valid PNG file." << std::endl;
        return;
    }*/

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    setjmp(png_jmpbuf(png_ptr));

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = info_ptr->width;
    height = info_ptr->height;

    png_read_update_info(png_ptr, info_ptr);

    if(setjmp(png_jmpbuf(png_ptr))) {
        std::cout << "Error during read_image" << std::endl;
        return;
    }

    int x, y, offset;
    bitmap = new pixel[width * height];
    png_byte *row = new png_byte[info_ptr->rowbytes];
    for(y = 0; y < height; y++) {
        png_read_row(png_ptr, row, NULL);
        for(x=0; x < width; x++) {
            offset = x * 3;
            bitmap[width * y + x] = (pixel){row[offset], row[offset + 1], row[offset + 2]};
        }
    }
    delete row;

    fclose(fp);
}

void Image::writeJPEG(const char *filename)
{
    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;

    FILE *output;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    output = fopen(filename, "wb");
    jpeg_stdio_dest(&cinfo, output);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, FALSE);

    jpeg_start_compress(&cinfo, TRUE);

    // TODO: Remove bitmap buffering in favour of direct access
    unsigned char *temp = new unsigned char[width*height*3];
    for(int y=0; y < height; y++) {
        for(int x=0; x < width; x++) {
            pixel px = bitmap[y*width + x];
            int point = y*width*3 + x*3;
            temp[point]     = px.r;
            temp[point + 1] = px.g;
            temp[point + 2] = px.b;
        }
    }

    JSAMPROW row_pointer[1];
    while(cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &temp[cinfo.next_scanline * width * 3];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    };
    delete temp;

    jpeg_finish_compress(&cinfo);

    fclose(output);

    jpeg_destroy_compress(&cinfo);
}

inline double Image::weighted_sum(const double dx,
                           const double dy,
                           const int    s00,
                           const int    s10,
                           const int    s01,
                           const int    s11)
{
    return ((1 - dy) * ((1 - dx) * s00 + dx * s10) + dy * ((1 - dx) * s01 + dx * s11));
}


void Image::bilinear(int sx, int sy, double xfrac, double yfrac, pixel *point)
{
    int row2 = sy * width;
    int row1 = sy == 0 ? row2 : row2 - width;
    int col2 = sx;
    int col1 = sx == 0 ? col2 : col2 - 1;

    pixel p0 = bitmap[row1+col1];
    pixel p1 = bitmap[row1+col2];
    pixel p2 = bitmap[row2+col1];
    pixel p3 = bitmap[row2+col2];

    point->r = clamp(weighted_sum(xfrac, yfrac, p0.r, p1.r, p2.r, p3.r));
    point->g = clamp(weighted_sum(xfrac, yfrac, p0.g, p1.g, p2.g, p3.g));
    point->b = clamp(weighted_sum(xfrac, yfrac, p0.b, p1.b, p2.b, p3.b));
}

inline double Image::cubic_spline_fit(double dx, double pt0, double pt1, double pt2, double pt3)
{
    return (double) ((( ( -pt0 + 3 * pt1 - 3 * pt2 + pt3 ) *   dx + ( 2 * pt0 - 5 * pt1 + 4 * pt2 - pt3 ) ) * dx + ( -pt0 + pt2 ) ) * dx + (pt1 + pt1) ) / 2.0;
}

void Image::bicubic(int sx, int sy, double xfrac, double yfrac, pixel *point, int newWidth, int newHeight)
{
    int row1,row2,row3,row4,
        col1,col2,col3,col4;
    double p1,p2,p3,p4;

    if(sy <= 1) {
        row1 = row2 = 0;
    } else {
        row1 = (sy - 2) * width;
        row2 = (sy - 1) * width;
    }
    row3 = sy * width;
    row4 = sy == newHeight ? row3 : row3 + width;

    if(sx <= 1) {
        col1 = col2 = 0;
    } else {
        col1 = sx - 2;
        col2 = sx - 1;
    }
    col3 = sx;
    col4 = sx == newWidth ? newWidth : sx + 1;

    // TODO: Ugh...
    point->r = clamp(cubic_spline_fit(yfrac,
        cubic_spline_fit(xfrac, bitmap[row1+col1].r, bitmap[row1+col2].r, bitmap[row1+col3].r, bitmap[row1+col4].r),
        cubic_spline_fit(xfrac, bitmap[row2+col1].r, bitmap[row2+col2].r, bitmap[row2+col3].r, bitmap[row2+col4].r),
        cubic_spline_fit(xfrac, bitmap[row3+col1].r, bitmap[row3+col2].r, bitmap[row3+col3].r, bitmap[row3+col4].r),
        cubic_spline_fit(xfrac, bitmap[row4+col1].r, bitmap[row4+col2].r, bitmap[row4+col3].r, bitmap[row4+col4].r)
    ));

    point->g = clamp(cubic_spline_fit(yfrac,
        cubic_spline_fit(xfrac, bitmap[row1+col1].g, bitmap[row1+col2].g, bitmap[row1+col3].g, bitmap[row1+col4].g),
        cubic_spline_fit(xfrac, bitmap[row2+col1].g, bitmap[row2+col2].g, bitmap[row2+col3].g, bitmap[row2+col4].g),
        cubic_spline_fit(xfrac, bitmap[row3+col1].g, bitmap[row3+col2].g, bitmap[row3+col3].g, bitmap[row3+col4].g),
        cubic_spline_fit(xfrac, bitmap[row4+col1].g, bitmap[row4+col2].g, bitmap[row4+col3].g, bitmap[row4+col4].g)
    ));

    point->b = clamp(cubic_spline_fit(yfrac,
        cubic_spline_fit(xfrac, bitmap[row1+col1].b, bitmap[row1+col2].b, bitmap[row1+col3].b, bitmap[row1+col4].b),
        cubic_spline_fit(xfrac, bitmap[row2+col1].b, bitmap[row2+col2].b, bitmap[row2+col3].b, bitmap[row2+col4].b),
        cubic_spline_fit(xfrac, bitmap[row3+col1].b, bitmap[row3+col2].b, bitmap[row3+col3].b, bitmap[row3+col4].b),
        cubic_spline_fit(xfrac, bitmap[row4+col1].b, bitmap[row4+col2].b, bitmap[row4+col3].b, bitmap[row4+col4].b)
    ));
}

bool Image::resize(int newWidth, int newHeight, const int interpolation)
{
    double scalex = (double)width / (double)newWidth;
    double scaley = (double)height / (double)newHeight;
    int x,y,sx,sy;
    double xfrac, yfrac;

    pixel *temp = new pixel[newHeight*newWidth];

    for(y=0; y < newHeight; y++) {
        yfrac = (y + 0.5) * scaley - 0.5;
        sy = (int) yfrac;
        yfrac = yfrac - sy;

        for(x=0; x < newWidth; x++) {
            xfrac = (x + 0.5) * scalex - 0.5;
            sx = (int) xfrac;
            xfrac = xfrac - sx;

            if(interpolation == BILINEAR) {
                bilinear(sx, sy, xfrac, yfrac, &temp[y*newWidth + x]);
            } else if (interpolation == BICUBIC) {
                bicubic(sx, sy, xfrac, yfrac, &temp[y*newWidth + x], newWidth, newHeight);
            }
        }
    }

    delete bitmap;
    bitmap = temp;
    width = newWidth;
    height = newHeight;

    return true;
}

unsigned char Image::clamp(double num)
{
    return num > 255 ? 255 : num < 0 ? 0 : num;
}

std::string Image::scanBarcode()
{
    std::string barcode = "";
    unsigned char *data = new unsigned char[width*height];
    int x, y, position;
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            position = y*width + x;
            pixel px = bitmap[position];
            data[position] = clamp(px.r*0.3 + px.g*0.59 + px.b*0.11);
        }
    }

    zebra::Image image = zebra::Image(width, height, "Y800", data, width*height);
    zebra::ImageScanner scanner = zebra::ImageScanner();

    scanner.set_config(zebra::ZEBRA_NONE, zebra::ZEBRA_CFG_ENABLE, 0);
    scanner.set_config(zebra::ZEBRA_EAN13, zebra::ZEBRA_CFG_ENABLE, 1);

    scanner.scan(image);

    zebra::Image::SymbolIterator iter = image.symbol_begin();
    zebra::Image::SymbolIterator iterEnd = image.symbol_end();

    for(; iter != iterEnd; ++iter) {
        std::string temp = iter->get_data().c_str();
        if(iter->get_location_size() > 1 && temp > barcode)
            barcode = temp;
    }

    delete data;

    return barcode;
}
