#include "Image.h"
#include "../Slingshot.h"

#include <jpeglib.h>
#include <jerror.h>
#include <png.h>
#include <zbar.h>

#include <cstdlib>
#include <iostream>
#include <setjmp.h>
#include <malloc.h>

#include <sys/time.h>
#include <emmintrin.h>
#include <omp.h>

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

    if( false && ratio > 1.5) {
        int interpolation = ratio > 2 ? BOX_FILTER : BILINEAR_SSE;
        struct timeval *Tps, *Tpf;
        Tps = (struct timeval*) malloc(sizeof(struct timeval));
        Tpf = (struct timeval*) malloc(sizeof(struct timeval));
        gettimeofday (Tps, NULL);
        resize(width/ratio, height/ratio, BOX_FILTER2);
        gettimeofday (Tpf, NULL);
        std::cout << "Total Time (usec):" << (Tpf->tv_usec - Tps->tv_usec) / 1000 << "ms" << std::endl;
    }


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
    /*else if(extension == ".png") {
        decodePNG(path.string());
    }*/
    else {
        std::cout << "Unsupported image format: " << extension << std::endl;
        return false;
    }
    return true;
}

/*bool Image::decodePNG(std::string filename) {
    char header[8];
    png_structp png_ptr;
    png_infop info_ptr;

    FILE *fp = fopen(filename.c_str(), "rb");
    if(!fp) {
        std::cout << "Unable to open \"" << filename << "\"" << std::endl;
        return false;
    }

    fread(header, 1, 8, fp);
    *if(png_sig_cmp(header, 0, 8)) {
        std::cout << "File " << filename << " is not a valid PNG file." << std::endl;
        return false;
    }*

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
            bitmap[width * y + x] = (pixel){row[offset], row[offset + 1], row[offset + 2], 0};
        }
    }
    delete[] row;

    fclose(fp);

    return true;
}*/

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

    jpeg_read_header(&cinfo, TRUE);
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

    jpeg_start_decompress(&cinfo);
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
    // TODO: Doesn't handle other colorspaces, greyscale is mangled

    /* Make a one-row-high sample array that will go away when done with image */
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, width * comp, 1);

    bitmap = new pixel*[height];
    for(int i=0; i < height; ++i) {
        bitmap[i] = new pixel[width];
        //bitmap[i] = reinterpret_cast<pixel*>(memalign(16, sizeof(pixel) * (width+16)));  // Append six pixels so we have 16bit to go on in case of SSE overflows
    }

    int x = 0;
    while (cinfo.output_scanline < height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        for(x = 0; x < width; ++x) {
            pixel p;
            p.r = (*buffer)[x*comp];
            p.g = (*buffer)[x*comp + 1];
            p.b = (*buffer)[x*comp + 2];

            bitmap[cinfo.output_scanline - 1][x] = p;
            //bitmap[x][height - cinfo.output_scanline] = p;
        }
    }

    jpeg_finish_decompress(&cinfo);
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
    jpeg_set_quality(&cinfo, 85, FALSE);

    jpeg_start_compress(&cinfo, TRUE);

    unsigned char **temp = reinterpret_cast<unsigned char**>(bitmap);
    JSAMPROW row_pointer[1];
    while(cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = temp[cinfo.next_scanline];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        delete[] row_pointer[0];
    };

    jpeg_finish_compress(&cinfo);

    output.insert(output.begin(), dst.buf, dst.buf + dst.used);

    jpeg_destroy_compress(&cinfo);
    free(dst.buf);
}

/*inline int Image::weighted_sum(const float dx,
                           const float dy,
                           const int    s00,
                           const int    s10,
                           const int    s01,
                           const int    s11)
{
    float val = (1 - dy) * ((1 - dx) * s00 + dx * s10) + dy * ((1 - dx) * s01 + dx * s11);
    return (int) (val+0.5);
}*/

/*void Image::bilinear_sse(
    const int source_x,
    const int source_y,
    const int source_x_fraction,
    const int source_y_fraction,
    pixel* point)
{
    int row2 = source_y * width;
    int row1 = source_y == 0 ? row2 : row2 - width;
    int col2 = source_x;
    int col1 = source_x == 0 ? col2 : col2 - 1;

    __m128i y1_fraction = _mm_set1_epi16(source_y_fraction);
    __m128i y0_fraction = _mm_set1_epi16(256 - source_y_fraction);
    __m128i x1_fraction = _mm_set1_epi16(source_x_fraction);
    __m128i x0_fraction = _mm_set1_epi16(256 - source_x_fraction);

    *pixel *p0 = &bitmap[row1+col1];
    pixel *p1 = &bitmap[row1+col2];
    pixel *p2 = &bitmap[row2+col1];
    pixel *p3 = &bitmap[row2+col2];

    __m128i y0 = _mm_set_epi16(0,0,0,0,0, p0->b, p0->g, p0->r);
    __m128i y1 = _mm_set_epi16(0,0,0,0,0, p1->b, p1->g, p1->r);
    __m128i y2 = _mm_set_epi16(0,0,0,0,0, p2->b, p2->g, p2->r);
    __m128i y3 = _mm_set_epi16(0,0,0,0,0, p3->b, p3->g, p3->r);*
 
    const __m128i* y0_ptr = reinterpret_cast<const __m128i*>(&bitmap[row1+col1]);
    const __m128i* y1_ptr = reinterpret_cast<const __m128i*>(&bitmap[row1+col2]);
    const __m128i* y2_ptr = reinterpret_cast<const __m128i*>(&bitmap[row2+col1]);
    const __m128i* y3_ptr = reinterpret_cast<const __m128i*>(&bitmap[row2+col2]);

    __m128i zero = _mm_setzero_si128();

    __m128i y0 = _mm_loadu_si128(y0_ptr);
    y0 = _mm_unpacklo_epi8(y0, zero);
    __m128i y1 = _mm_loadu_si128(y1_ptr);
    y1 = _mm_unpacklo_epi8(y1, zero);
    __m128i y2 = _mm_loadu_si128(y2_ptr);
    y2 = _mm_unpacklo_epi8(y2, zero);
    __m128i y3 = _mm_loadu_si128(y3_ptr);
    y3 = _mm_unpacklo_epi8(y3, zero);

    y0 = _mm_mullo_epi16(y0, x0_fraction);
    y1 = _mm_mullo_epi16(y1, x1_fraction);
    y2 = _mm_mullo_epi16(y2, x0_fraction);
    y3 = _mm_mullo_epi16(y3, x1_fraction);
    
    y0 = _mm_add_epi16(y0, y1);
    y2 = _mm_add_epi16(y2, y3);
    
    y0 = _mm_srli_epi16(y0, 8);
    y2 = _mm_srli_epi16(y2, 8);

    y0 = _mm_mullo_epi16(y0, y0_fraction);
    y2 = _mm_mullo_epi16(y2, y1_fraction);

    y0 = _mm_add_epi16(y0, y2);
    y0 = _mm_srli_epi16(y0, 8);

    y0 = _mm_packus_epi16(y0, zero);

    __m128i* dest128 = reinterpret_cast<__m128i*>(point);
    _mm_storeu_si128(dest128, y0);
}*/

/*float aligned_float[4] __attribute__((aligned(16))) = {0.0,0.0,0.0,0.0};

void Image::bilinear_sse_float(
    const int source_x,
    const int source_y,
    const float source_x_fraction,
    const float source_y_fraction,
    pixel* point)
{
    int row2 = source_y * width;
    int row1 = source_y == 0 ? row2 : row2 - width;
    int col2 = source_x;
    int col1 = source_x == 0 ? col2 : col2 - 1;

    __m128i zero = _mm_setzero_si128();

    const __m128i* y0_ptr = reinterpret_cast<const __m128i*>(&bitmap[row1+col1]);
    const __m128i* y1_ptr = reinterpret_cast<const __m128i*>(&bitmap[row1+col2]);
    const __m128i* y2_ptr = reinterpret_cast<const __m128i*>(&bitmap[row2+col1]);
    const __m128i* y3_ptr = reinterpret_cast<const __m128i*>(&bitmap[row2+col2]);

    __m128i y0i = _mm_loadu_si128(y0_ptr);
    y0i = _mm_unpacklo_epi8(y0i, zero);
    y0i = _mm_unpacklo_epi16(y0i, zero);
    __m128 y0 = _mm_cvtepi32_ps(y0i);
    __m128i y1i = _mm_loadu_si128(y1_ptr);
    y1i = _mm_unpacklo_epi8(y1i, zero);
    y1i = _mm_unpacklo_epi16(y1i, zero);
    __m128 y1 = _mm_cvtepi32_ps(y1i);
    __m128i y2i = _mm_loadu_si128(y2_ptr);
    y2i = _mm_unpacklo_epi8(y2i, zero);
    y2i = _mm_unpacklo_epi16(y2i, zero);
    __m128 y2 = _mm_cvtepi32_ps(y2i);
    __m128i y3i = _mm_loadu_si128(y3_ptr);
    y3i = _mm_unpacklo_epi8(y3i, zero);
    y3i = _mm_unpacklo_epi16(y3i, zero);
    __m128 y3 = _mm_cvtepi32_ps(y3i);

    // 5ms slower than loading manually
    *pixel *p0 = &bitmap[row1+col1];
    pixel *p1 = &bitmap[row1+col2];
    pixel *p2 = &bitmap[row2+col1];
    pixel *p3 = &bitmap[row2+col2];

    __m128i y0i = _mm_set_epi32(0, p0->b, p0->g, p0->r);
    __m128i y1i = _mm_set_epi32(0, p1->b, p1->g, p1->r);
    __m128i y2i = _mm_set_epi32(0, p2->b, p2->g, p2->r);
    __m128i y3i = _mm_set_epi32(0, p3->b, p3->g, p3->r);
    __m128 y0 = _mm_cvtepi32_ps(y0i);
    __m128 y1 = _mm_cvtepi32_ps(y1i);
    __m128 y2 = _mm_cvtepi32_ps(y2i);
    __m128 y3 = _mm_cvtepi32_ps(y3i);*

    // 3ms faster than subtracting on fpu
    __m128 one = _mm_set1_ps(1.0);
    __m128 y1_fraction = _mm_set1_ps(source_y_fraction);
    __m128 y0_fraction = _mm_sub_ps(one, y1_fraction);
    __m128 x1_fraction = _mm_set1_ps(source_x_fraction);
    __m128 x0_fraction = _mm_sub_ps(one, x1_fraction);

    y0 = _mm_mul_ps(y0, x0_fraction);
    y1 = _mm_mul_ps(y1, x1_fraction);
    y2 = _mm_mul_ps(y2, x0_fraction);
    y3 = _mm_mul_ps(y3, x1_fraction);
    
    y0 = _mm_add_ps(y0, y1);
    y2 = _mm_add_ps(y2, y3);
    
    y0 = _mm_mul_ps(y0, y0_fraction);
    y2 = _mm_mul_ps(y2, y1_fraction);

    y0 = _mm_add_ps(y0, y2);

    _mm_store_ps(aligned_float, y0);
    
    *point = (pixel){clamp(aligned_float[0]), clamp(aligned_float[1]), clamp(aligned_float[2]), 0};
}*/


/*void Image::bilinear(const int sx, const int sy, const float xfrac, const float yfrac, pixel *point)
{
    int row2 = sy * width;
    int row1 = sy == 0 ? row2 : row2 - width;
    int col2 = sx;
    int col1 = sx == 0 ? col2 : col2 - 1;

    pixel *p0 = &bitmap[row1+col1];
    pixel *p1 = &bitmap[row1+col2];
    pixel *p2 = &bitmap[row2+col1];
    pixel *p3 = &bitmap[row2+col2];

    point->r = weighted_sum(xfrac, yfrac, p0->r, p1->r, p2->r, p3->r);
    point->g = weighted_sum(xfrac, yfrac, p0->g, p1->g, p2->g, p3->g);
    point->b = weighted_sum(xfrac, yfrac, p0->b, p1->b, p2->b, p3->b);
}

inline float Image::cubic_spline_fit(float dx, int pt0, int pt1, int pt2, int pt3)
{
    return (float) ((( ( -pt0 + 3 * pt1 - 3 * pt2 + pt3 ) *   dx + ( 2 * pt0 - 5 * pt1 + 4 * pt2 - pt3 ) ) * dx + ( -pt0 + pt2 ) ) * dx + (pt1 + pt1) ) / 2.0;
}

void Image::bicubic(int sx, int sy, float xfrac, float yfrac, pixel *point, int newWidth, int newHeight)
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
}*/

bool Image::resize(int newWidth, int newHeight, const int interpolation)
{
    float xstep = (float)width / newWidth;
    float ystep = (float)height / newHeight;
    float xval = 0;
    float yval = 0;//0.5 * ystep - 0.5;
    float xfrac, yfrac;
    int x,y,sx,sy;

    /*pixel *temp = new pixel[newHeight*newWidth];

    if(interpolation == BILINEAR) {
        for(y=0; y < newHeight; y++) {
            sy = (int) yval;
            yfrac = yval - sy;

            xval = 0.5 * xstep - 0.5;
            for(x=0; x < newWidth; x++) {
                sx = (int) xval;
                xfrac = xval - sx;

                if(interpolation == BILINEAR) {
                    bilinear(sx, sy, xfrac, yfrac, &temp[y*newWidth + x]);
                } else if (interpolation == BICUBIC) {
                    bicubic(sx, sy, xfrac, yfrac, &temp[y*newWidth + x], newWidth, newHeight);
                }

                xval += ystep;
            }

            yval += ystep;
        }
    }
    else if(interpolation == BILINEAR_SSE) {
        const int kFractionBits = 16;
        const int kFractionMax = 1 << kFractionBits;
        const int kFractionMask = ((1 << kFractionBits) - 1);

        const int yscale_fixed = (height  << kFractionBits) / newHeight;
        const int xscale_fixed = (width << kFractionBits) / newWidth;
        for(y = 0; y < newHeight; ++y) {
            int source_y_subpixel = (y * yscale_fixed);
            int source_y = source_y_subpixel >> kFractionBits;
            int source_y_fraction = (source_y_subpixel & kFractionMask) >> 8;

            for(x=0; x < newWidth; ++x) {
                int source_x_subpixel = (x * xscale_fixed);
                int source_x = source_x_subpixel >> kFractionBits;
                int source_x_fraction = (source_x_subpixel & kFractionMask) >> 8;

                bilinear_sse(source_x, source_y, source_x_fraction, source_y_fraction, &temp[y*newWidth + x]);
            }
        }
    }
    else if(interpolation == BILINEAR_SSE_FLOAT) {

        for(y = 0; y < newHeight; ++y) {
            sy = (int) yval;
            yfrac = yval - sy;

            xval = 0.5 * xstep - 0.5;
            for(x=0; x < newWidth; ++x) {
                sx = (int) xval;
                xfrac = xval - sx;

                bilinear_sse_float(sx, sy, xfrac, yfrac, &temp[y*newWidth + x]);
                xval += xstep;
            }
            yval += ystep;
        }
    }
    else if(interpolation == BOX_FILTER) {
        //#pragma omp parallel for
        for(int i=0; i < newHeight; ++i) {
            float yval = i * ystep;
            int nexty = yval + ystep + 0.5;
            int y = yval + 0.5;
            int ycount = nexty - y;

            box_average_h2(&temp[newWidth*i], y*width, ycount, xstep);
            yval += ystep;
        }
    }*/
    if(interpolation == BOX_FILTER2) {
        struct timeval *Tps, *Tpf;
        Tps = (struct timeval*) malloc(sizeof(struct timeval));
        Tpf = (struct timeval*) malloc(sizeof(struct timeval));

        uint16_t **asd = new uint16_t*[newWidth]; // TODO: Hardcoded colorspace 
        #pragma omp parallel for
        for(int i=0; i < newWidth; ++i) {
            asd[i] = reinterpret_cast<uint16_t*>(memalign(16, sizeof(uint16_t) * (height + 16)*3));  // Append six pixels so we have 16bit to go on in case of SSE overflows
        }

        gettimeofday (Tps, NULL);
        for(int i=0; i < newWidth; ++i) {
            float xval = i * xstep;
            int nextx = xval + xstep + 0.5;
            int x = xval + 0.5;
            int xcount = nextx - x;

            box2_horizontal(asd[i], x, xcount);
            xval += xstep;
        }
        gettimeofday (Tpf, NULL);
        std::cout << "Vertical Filter Time (usec):" << (Tpf->tv_usec - Tps->tv_usec) / 1000 << "ms" << std::endl;

        for(int i=0; i < width; ++i) {
            free(bitmap[i]);
        }
        delete[] bitmap;
        bitmap = NULL;

        gettimeofday (Tps, NULL);

        uint16_t **bsd = new uint16_t*[height];
        for(int i=0; i < height; ++i) {
            bsd[i] = reinterpret_cast<uint16_t*>(memalign(16, sizeof(uint16_t) * (newWidth + 16)*3));  // Append six pixels so we have 16bit to go on in case of SSE overflows
        }
        #pragma omp parallel for
        for(int x=0; x < newWidth; ++x) {
            for(int y=0; y < height; ++y) {
                bsd[height - y - 1][x*3] = asd[x][y*3];
                bsd[height - y - 1][(x*3)+1] = asd[x][(y*3)+1];
                bsd[height - y - 1][(x*3)+2] = asd[x][(y*3)+2];
            }
        }
        for(int i=0; i < newWidth; ++i) {
            free(asd[i]);
        }
        delete[] asd;
        asd = NULL;
        gettimeofday (Tpf, NULL);
        std::cout << "Total Pivot Time (usec):" << (Tpf->tv_usec - Tps->tv_usec) / 1000 << "ms" << std::endl;

        bitmap = new pixel*[newHeight];
        for(int i=0; i < newHeight; ++i) {
            bitmap[i] = reinterpret_cast<pixel*>(memalign(16, sizeof(pixel) * newWidth+8));  // Append eight pixels for SSE overflow
        }

        gettimeofday (Tps, NULL);
        #pragma omp parallel for
        for(int i=0; i < newHeight; ++i) {
            float yval = i * ystep;
            int nexty = yval + ystep + 0.5;
            int y = yval + 0.5;
            int ycount = nexty - y;

            box2_vertical(bsd, i, y, ycount, xstep, newWidth);
            yval += ystep;
        }
        gettimeofday (Tpf, NULL);
        std::cout << "Horizontal Filter Time (usec):" << (Tpf->tv_usec - Tps->tv_usec) / 1000 << "ms" << std::endl;

        for(int i=0; i < height; ++i) {
            free(bsd[i]);
        }
        delete[] bsd;
        bsd = NULL;
    }

    width = newWidth;
    height = newHeight;

    return true;
}

void Image::box2_vertical(uint16_t** temp, int y, int ystart, int ycount, int xcount, int newWidth) {
    __m128i hi, lo, tempHi, tempLo;
    const __m128i zero = _mm_setzero_si128();
    const __m128i div = _mm_set1_epi16(1.0 / (xcount * ycount) * 256);
    uint8_t **bit = reinterpret_cast<uint8_t**>(bitmap);
    

    const __m128i* main_ptr = reinterpret_cast<const __m128i*>(&temp[ystart][0]);
    for(int x=0; x < newWidth*3; x+=16) { //TODO: Color component 
        lo = _mm_load_si128(main_ptr++);
        hi = _mm_load_si128(main_ptr++);

        for(int i=1; i < ycount; ++i) {
            // TODO: Try storing memory references
            const __m128i* ptr = reinterpret_cast<const __m128i*>(&temp[ystart+i][x]);
            tempLo = _mm_load_si128(ptr++);
            tempHi = _mm_load_si128(ptr);
            hi = _mm_add_epi16(hi, tempHi);
            lo = _mm_add_epi16(lo, tempLo);
        }

        lo = _mm_mullo_epi16(div, lo);
        lo = _mm_srli_epi16(lo, 8);
        hi = _mm_mullo_epi16(div, hi);
        hi = _mm_srli_epi16(hi, 8);

        lo = _mm_packus_epi16(lo, hi);

        __m128i* dest128 = reinterpret_cast<__m128i*>(&bit[y][x]);
        _mm_store_si128(dest128, lo);
    }
}

void Image::box2_horizontal(uint16_t* temp, int xstart, int xcount)
{
    __m128i hi, lo, tempHi, tempLo;
    const __m128i zero = _mm_setzero_si128();
    uint8_t **bit = reinterpret_cast<uint8_t**>(bitmap);

    for(int y=0; y < height*3; y+=16) {
        hi = _mm_setzero_si128();
        lo = _mm_setzero_si128();

        for(int i=1; i < xcount; ++i) {
            const __m128i* ptr = reinterpret_cast<const __m128i*>(&bit[xstart+i][y]);
            tempLo = _mm_load_si128(ptr);
            tempHi = _mm_unpackhi_epi8(tempLo, zero);
            tempLo = _mm_unpacklo_epi8(tempLo, zero);
            hi = _mm_add_epi16(hi, tempHi);
            lo = _mm_add_epi16(lo, tempLo);
        }

        __m128i* dest128 = reinterpret_cast<__m128i*>(&temp[y]);
        _mm_store_si128(dest128, lo);
        _mm_store_si128(++dest128, hi);
    }
}


/*void Image::box_average_h2(pixel* temp, int ystart, int ycount, float xstep)
{
    __m128i hi, lo, tempHi, tempLo, remHi, remLo;
    const __m128i zero = _mm_setzero_si128();

    for(float xpos=0.0; xpos < width; xpos+=xstep) {
        int x = xpos+0.5;
        int nextx = xpos + xstep + 0.5;
        int xcount = nextx - x;
        hi = _mm_setzero_si128();
        lo = _mm_setzero_si128();
        remHi = _mm_setzero_si128();
        remLo = _mm_setzero_si128();

        int offset = ystart + x;
        if(xcount > 4) {
            for(int i=0; i < xcount-4; i+=4) {
                for(int j=0; j < ycount; ++j) {
                    //TODO: Duff's device?
                    const __m128i* source_ptr = reinterpret_cast<const __m128i*>(&bitmap[offset + width*j]);
                    tempLo = _mm_loadu_si128(source_ptr);
                    tempHi = _mm_unpackhi_epi8(tempLo, zero);
                    tempLo = _mm_unpacklo_epi8(tempLo, zero);
                    hi = _mm_add_epi16(hi, tempHi);
                    lo = _mm_add_epi16(lo, tempLo);
                }
                offset += 4;
            }
        }

        for(int j=0; j < ycount; ++j) {
            //TODO: Duff's device?
            const __m128i* source_ptr = reinterpret_cast<const __m128i*>(&bitmap[offset + width*j]);
            tempLo = _mm_loadu_si128(source_ptr);
            tempHi = _mm_unpackhi_epi8(tempLo, zero);
            tempLo = _mm_unpacklo_epi8(tempLo, zero);
            remHi = _mm_add_epi16(remHi, tempHi);
            remLo = _mm_add_epi16(remLo, tempLo);
        }

        switch(xcount % 4) {
            case 0:
                hi = _mm_add_epi16(hi, remHi);
                lo = _mm_add_epi16(lo, remLo);
                break;
            case 2:
                lo = _mm_add_epi16(lo, remLo);
                break;
            case 3:
                lo = _mm_add_epi16(lo, remLo);
                remHi = _mm_slli_si128(remHi, 8);
                hi = _mm_add_epi16(hi, remHi);
                break;
            case 1:
                remLo = _mm_slli_si128(remLo, 8);
                lo = _mm_add_epi16(lo, remLo);
                break;
        }

        lo = _mm_add_epi16(lo, hi);
        tempLo = _mm_srli_si128(lo, 8);
        lo = _mm_add_epi16(lo, tempLo);

        __m128i div = _mm_set1_epi16(1.0 / (xcount * ycount) * 256);

        lo = _mm_mullo_epi16(div, lo);
        lo = _mm_srli_epi16(lo, 8);
        lo = _mm_packus_epi16(lo, zero);

        __m128i* dest128 = reinterpret_cast<__m128i*>(temp++);
        _mm_storeu_si128(dest128, lo);
    }
}*/

unsigned char Image::clamp(double num)
{
    return num > 255 ? 255 : num < 0 ? 0 : num;
}

unsigned char Image::clamp(float num)
{
    return num > 255 ? 255 : num < 0 ? 0 : num;
}

/*std::string Image::scanBarcode()
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

    delete[] data;

    return barcode;
}*/
