#include "Image.h"

#include <jpeglib.h>
#include <png.h>

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

bool Image::write(int type, const char *filename) {
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

    int x, y, offset, stride;
    bitmap = new unsigned char[width * height * 3];
    png_byte *row = new png_byte[info_ptr->rowbytes];
    for(y = 0; y < height; y++) {
        stride = width * 3 * y;
        png_read_row(png_ptr, row, NULL);
        for(x=0; x < width; x++) {
            offset = x*3;
            bitmap[stride + offset]     = row[offset];
            bitmap[stride + offset + 1] = row[offset+1];
            bitmap[stride + offset + 2] = row[offset+2];
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
    JSAMPROW row_pointer[1];
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    output = fopen(filename, "wb");
    jpeg_stdio_dest(&cinfo, output);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 85, FALSE);

    jpeg_start_compress(&cinfo, TRUE);
    row_stride = width * 3;
    while(cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = & bitmap[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);

    fclose(output);

    jpeg_destroy_compress(&cinfo);
}

void Image::bilinear()
{
}
