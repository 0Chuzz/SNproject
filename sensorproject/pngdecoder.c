/*
 * pngdecoder.c
 *
 *  Created on: Jun 5, 2014
 *      Author: chuzz
 */

#include<png.h>
#include<stdlib.h>
#include<assert.h>

int decodePng(const char filename[], unsigned char buff[], int w, int h){
	FILE *fp;
	char header[8];
	int y;

	int width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;
	png_bytep * row_pointers;
	fp = fopen(filename, "r");
	fread(header, 1, 8, fp);

    png_ptr = png_create_read_struct
        (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
       return -1;


    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
            return -1;

    if (setjmp(png_jmpbuf(png_ptr)))
            return -1; //abort("[read_png_file] Error during init_io");

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);


    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
            return -1;//abort("[read_png_file] Error during read_image");

    /*
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (y=0; y<height; y++)
            row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    */

    char *rowptrs[240];
    row_pointers = (png_bytep *) rowptrs;
    for (y=0; y< 240; y++){
    	rowptrs[y] = (png_bytep) buff + (y*40);
    }
    assert(height == 240 && width == 320 && number_of_passes == 1 &&
    		color_type == PNG_COLOR_TYPE_GRAY && bit_depth == 1);
    png_read_image(png_ptr, row_pointers);
    fclose(fp);

    //printf("%d %d %d %d %d\n", width, height, color_type, bit_depth, png_get_rowbytes(png_ptr,info_ptr));

    return 0;
}

