/*
Resources:
https://www.quora.com/How-does-Instagram-develop-their-filters
http://dimitroff.bg/image-filtering-your-own-instagram/
http://mashable.com/2013/10/20/photoshop-instagram-filters/#L68uReJH5Pqo
*/


#define JC_INI_IMPLEMENTATION
#include "jc_ini.h"

#define JC_IMAGEFILTERS_IMPLEMENTATION
#include "jc_imagefilters.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"

#include <stdbool.h>
#include <string.h>

#include <sys/time.h>

static uint64_t get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000U + (uint64_t)tv.tv_usec;
}


void Usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\timgfilter -i <image file> -p <preset_file> -o <output> \n");
}

static void file_write_fn(void* ctx, void* data, int size)
{
    fwrite(data, 1, size, (FILE*)ctx);
}

int main(int argc, const char** argv)
{
    const char* inputfile = 0;
    const char* outputfile = 0;

    bool flip = 0;
    for( int i = 1; i < argc; ++i )
    {
        if(strcmp(argv[i], "-i") == 0)
        {
            if( i+1 < argc )
                inputfile = argv[i+1];
            else
            {
                Usage();
                return 1;
            }
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            if( i+1 < argc )
                outputfile = argv[i+1];
            else
            {
                Usage();
                return 1;
            }
        }
    }

    FILE* input = 0;
    if( inputfile )
    {
        input = fopen(inputfile, "rb");
    }
    else
    {
        input = stdin;
    }

    uint64_t loadstart = get_time();

    int width = 0;
    int height = 0;
    int nchannels = 0;
    uint8_t* image = stbi_load_from_file(input, &width, &height, &nchannels, 3);

    if(inputfile)
    {
        fclose(input);
    }

    if(!image)
    {
        fprintf(stderr, "Failed to open image from %s\n", inputfile ? inputfile : "<stdin>");
        return 1;
    }

    uint64_t loadend = get_time();
    printf("Loaded in %f ms\n", (loadend - loadstart) / 1000.0f);

    int result = 0;

    // FILTERS
    uint64_t filterstart = get_time();
    
    // uint8_t ptsred[]   = { 40, 50, 80, 90, 95, 120, 160 };
    // uint8_t ptsgreen[] = { 80, 120, 140, 160, 220 };
    // uint8_t ptsblue[]  = { 0, 50, 75, 120, 230 };

    // uint8_t ptsred[]   = { 41, 48, 57, 65, 72, 79, 86, 91, 96, 104, 109, 115, 122, 132, 143, 154 };
    // uint8_t ptsgreen[] = { 4, 10, 20, 34, 52, 73, 94, 117, 139, 157, 177, 192, 204, 214, 223, 229 };
    // uint8_t ptsblue[]  = { 23, 41, 63, 87, 115, 139, 163, 185, 204, 219, 230, 238, 244, 247, 250, 252 };

    // "1977"
    uint8_t ptsred[]   = { 91, 255 };
    uint8_t ptsgreen[] = { 0, 255 };
    uint8_t ptsblue[]  = { 44, 255 };


    result = jc_if_apply_curve(image, width, height, nchannels,
                                    ptsred, sizeof(ptsred)/sizeof(ptsred[0]),
                                    ptsgreen, sizeof(ptsgreen)/sizeof(ptsgreen[0]),
                                    ptsblue, sizeof(ptsblue)/sizeof(ptsblue[0]), 1.0f);


    result = jc_if_gamma(image, width, height, nchannels, 2.00f, 1);

    result = jc_if_contrast(image, width, height, nchannels, 0.15f, 1);

    result = jc_if_vignette(image, width, height, nchannels, 0.75f, 0.5f);


    uint64_t filterend = get_time();
    printf("Filters took %f ms\n", (filterend - filterstart) / 1000.0f);

    if( flip )
    {
        int stride = width*3;
        uint8_t* row = (uint8_t*)malloc((size_t)stride);
        for( int y = 0; y < height/2; ++y )
        {
            memcpy(row, &image[y*stride], (size_t)stride);
            memcpy(&image[y*stride], &image[(height-1-y)*stride], (size_t)stride);
            memcpy(&image[(height-1-y)*stride], row, (size_t)stride);
        }
    }

    uint64_t savestart = get_time();

    FILE* output = 0;
    if( outputfile )
    {
        output = fopen(outputfile, "wb");
    }
    else
    {
        output = stdout;
    }

    //result = stbi_write_png_to_func(file_write_fn, output, width, height, nchannels, image, width * nchannels);
    result = stbi_write_tga_to_func(file_write_fn, output, width, height, nchannels, image);

    if( outputfile )
    {
        fclose(output);
        printf("wrote %s\n", outputfile);
    }

    uint64_t saveend = get_time();
    printf("Saved in %f ms\n", (saveend - savestart) / 1000.0f);

    free(image);

    return result ? 1 : 0;
}
