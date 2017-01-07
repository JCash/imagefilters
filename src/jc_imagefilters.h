/*
*/

#pragma once

#include <stdint.h>
#include <math.h>


int jc_if_apply_curve(uint8_t* image, int w, int h, int nchannels,
                        uint8_t* curve_red, uint32_t curve_red_npoints,
                        uint8_t* curve_green, uint32_t curve_green_npoints,
                        uint8_t* curve_blue, uint32_t curve_blue_npoints,
                        float blend );

int jc_if_vignette(uint8_t* image, int w, int h, int nchannels, float falloff, float blend);

#if defined(JC_IMAGEFILTERS_IMPLEMENTATION)

float jc_if_map_to_01(float value, float min, float max);
float jc_if_mix(float a, float b, float t);
float jc_if_apply_curve_value(uint8_t* pts, uint32_t npoints, float value);


float jc_if_map_to_01(float value, float min, float max)
{
    return (value - min) / (max - min);
}

float jc_if_mix(float a, float b, float t)
{
    return a * (1.0f - t) + b * t;
}

float jc_if_apply_curve_value(uint8_t* pts, uint32_t npoints, float value)
{
    float unit = value / 255.0f;
    uint32_t index1 = (uint32_t)( (npoints - 1) * unit );
    uint32_t index2 = npoints>1 ? index1+1 : index1;

    float v1 = pts[index1];
    float v2 = pts[index2];
    float step = 1.0f / (npoints - 1);
    float t = jc_if_map_to_01(unit, index1*step, index2*step);
    float v = jc_if_mix(v1, v2, t);

/*
printf("unit  %f\n", unit);
printf("indices: %d, %d\n", index1, index2);
printf("v1, v2: %f, %f\n", v1, v2);
printf("step: %f\n", step);
printf("r1,r2: %f, %f\n", index1*step, index2*step);
printf("t: %f\n", t);
printf("v: %f\n", v);
*/

    return v;
}

int jc_if_apply_curve(uint8_t* image, int w, int h, int nchannels,
                        uint8_t* curve_red, uint32_t curve_red_npoints,
                        uint8_t* curve_green, uint32_t curve_green_npoints,
                        uint8_t* curve_blue, uint32_t curve_blue_npoints,
                        float blend )
{
    uint8_t* curves[3] = { curve_red, curve_green, curve_blue };
    uint32_t num[3] = { curve_red_npoints, curve_green_npoints, curve_blue_npoints };

    for( int i = 0; i < w * h; ++i )
    {
        for( int c = 0; c < nchannels; ++c )
        {
            float value = image[i*nchannels + c];
            float mapped = jc_if_apply_curve_value(curves[c], num[c], value);
            image[i*nchannels + c] = (uint8_t)(jc_if_mix(value, mapped, blend));
        }
    }
    return 0;
}

int jc_if_vignette(uint8_t* image, int w, int h, int nchannels, float falloff, float blend)
{
    int mx = w / 2;
    int my = h / 2;
    int radius = mx < my ? mx : my;

    for( int y = 0; y < h; ++y )
    {
        for( int x = 0; x < w; ++x )
        {
            int i = y * w * nchannels + x * nchannels;
            for( int c = 0; c < nchannels; ++c )
            {
                float value = image[i + c];
                int dx = x - mx;
                int dy = y - my;
                float dist = sqrtf(dx*dx + dy*dy);
                if(dist > radius)
                    dist = radius;
                float s = 1.0f - dist / radius;
                s = powf(s, falloff);
                float mapped = s * value;
                image[i + c] = (uint8_t)(jc_if_mix(value, mapped, blend));
            }
        }
    }
    return 0;    
}


#endif // JC_IMAGEFILTERS_IMPLEMENTATION
