#include "Image.h"

#include <jpeglib.h>
#include <jerror.h>
#include <png.h>
#include <zbar.h>

#include <cstdlib>
#include <iostream>
#include <setjmp.h>

Image::~Image() {
    delete[] bitmap;
}

bool Image::load(int format)
{
    if(loaded)
        return true;
    else 
        loaded = true;
    
    open();
    double ratio = (double)height / 800;
    resize(width/ratio, height/ratio, BICUBIC);
    if(format == JPEG) {
        encodeJPEG();
    }
    return true;
}

bool Image::load()
{
    Image::load(JPEG);
    return true;
}

bool Image::done()
{
    return true;
}

std::string Image::getMimetype()
{
    return "image/jpeg";
}

int Image::getSize()
{
    return output.size();
}

int Image::read(unsigned int pos, unsigned int max, char *buffer)
{
    int count = pos + max > output.size() ? output.size() - pos : max;
    memcpy(buffer, &output[pos], count);
    if(count > 0)
        return count;
    else 
        return -1;
}

bool Image::open()
{

    if(extension == ".jpg" || extension == ".jpeg") {
        decodeJPEG(path.string());
    }
    else if(extension == ".png") {
        decodePNG(path.string());
    }
    else {
        std::cout << "Unsupported image format: " << extension << std::endl;
        return false;
    }
    return true;
}

bool Image::decodePNG(std::string filename) {
    char header[8];
    png_structp png_ptr;
    png_infop info_ptr;

    FILE *fp = fopen(filename.c_str(), "rb");
    if(!fp) {
        std::cout << "Unable to open \"" << filename << "\"" << std::endl;
        return false;
    }

    fread(header, 1, 8, fp);
    /*if(png_sig_cmp(header, 0, 8)) {
        std::cout << "File " << filename << " is not a valid PNG file." << std::endl;
        return false;
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
        return false;
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

    return true;
}

struct my_error_mgr {
      struct jpeg_error_mgr pub;    /* "public" fields */

        jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    (*cinfo->err->output_message) (cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}


bool Image::decodeJPEG(std::string filename)
{
    // TODO: Clean up cruft and retarded error handling
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;        /* source file */
  JSAMPARRAY buffer;        /* Output row buffer */

  /* In this example we want to open the input file before doing anything else,
   * so that the setjmp() error recovery below can assume the file is open.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to read binary files.
   */

  if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  width = cinfo.output_width;
  height = cinfo.output_height;
  int comp = cinfo.output_components;
  int stepOne = comp > 2 ? 1 : 0;
  int stepTwo = comp > 2 ? 2 : 0;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, width * comp, 1);

    bitmap = new pixel[width * height];
    while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        for(int x = 0; x < width; x++) {
            int pos = (cinfo.output_scanline-1)*width + x;
            bitmap[pos] = (pixel) {
                (*buffer)[x*comp], 
                (*buffer)[x*comp + stepOne], 
                (*buffer)[x*comp + stepTwo]};
        }
    }
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  fclose(infile);
    return true;
}

struct imagetojpeg_dst { 
        struct jpeg_destination_mgr jdst; 
        JOCTET *buf; 
        JOCTET *off; 
        size_t sz; 
        size_t used; 

}; 

void imagetojpeg_dst_init(j_compress_ptr cinfo) 
{ 
        imagetojpeg_dst *dst = (imagetojpeg_dst *)cinfo->dest; 

        dst->used = 0; 
        //TODO: allocating full bitmap since resizing later breaks
        dst->sz = cinfo->image_width 
                * cinfo->image_height 
                * cinfo->input_components / 4;  /* 1/8th of raw size */ 
        dst->buf = (JOCTET *)malloc(dst->sz * sizeof dst->buf); 
        dst->off = dst->buf; 
        dst->jdst.next_output_byte = dst->off; 
        dst->jdst.free_in_buffer = dst->sz; 

        return; 

} 

boolean imagetojpeg_dst_empty(j_compress_ptr cinfo) 
{ 
        imagetojpeg_dst *dst = (imagetojpeg_dst *)cinfo->dest; 

        dst->used = dst->sz - dst->jdst.free_in_buffer;
        dst->sz *= 2; 
        dst->buf = (JOCTET *)realloc((void *)dst->buf, dst->sz * sizeof dst->buf); 
        dst->off = dst->buf + dst->used; 
        dst->jdst.next_output_byte = dst->off; 
        dst->jdst.free_in_buffer = dst->sz - dst->used; 

        return TRUE;

} 

void imagetojpeg_dst_term(j_compress_ptr cinfo) 
{ 
        struct imagetojpeg_dst *dst = (imagetojpeg_dst *)cinfo->dest; 

        dst->used += dst->sz - dst->jdst.free_in_buffer; 
        dst->off = dst->buf + dst->used; 

        return; 

} 

void imagetojpeg_dst_set(j_compress_ptr cinfo, struct imagetojpeg_dst *dst) 
{ 
        dst->jdst.init_destination = imagetojpeg_dst_init; 
        dst->jdst.empty_output_buffer = imagetojpeg_dst_empty; 
        dst->jdst.term_destination = imagetojpeg_dst_term; 
        cinfo->dest = (jpeg_destination_mgr *)dst;

        return; 

}

void Image::encodeJPEG()
{
    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;

    imagetojpeg_dst dst;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    imagetojpeg_dst_set(&cinfo, &dst);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 60, FALSE);

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
    delete[] temp;
    delete[] bitmap;
    bitmap = NULL;

    jpeg_finish_compress(&cinfo);

    output.insert(output.begin(), dst.buf, dst.buf + dst.used);

    jpeg_destroy_compress(&cinfo);
    free(dst.buf);
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

    zbar::Image image = zbar::Image(width, height, "Y800", data, width*height);
    zbar::ImageScanner scanner = zbar::ImageScanner();

    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 0);
    scanner.set_config(zbar::ZBAR_EAN13, zbar::ZBAR_CFG_ENABLE, 1);

    scanner.scan(image);

    zbar::Image::SymbolIterator iter = image.symbol_begin();
    zbar::Image::SymbolIterator iterEnd = image.symbol_end();

    for(; iter != iterEnd; ++iter) {
        std::string temp = iter->get_data().c_str();
        if(iter->get_location_size() > 1 && temp > barcode)
            barcode = temp;
    }

    delete data;

    return barcode;
}
