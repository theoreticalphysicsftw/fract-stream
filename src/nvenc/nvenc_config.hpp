#ifndef NVENC_CONFIG_HPP_INCLUDED
#define NVENC_CONFIG_HPP_INCLUDED

#include <cstdint>

namespace fractal
{
    struct nvenc_encoder_config_t
    {
        uint32_t height;
        uint32_t width;
        uint32_t fps;
        uint32_t gop_length;
        uint32_t number_of_b_frames;
        uint32_t initial_qp;
        uint32_t avg_bitrate;
        // Used in VBR mode only
        uint32_t max_bitrate;
        bool vbr;
    };
    
    struct nvenc_encoder_limits_t
    {
        uint32_t max_height;
        uint32_t max_width;
        uint32_t max_b_frames;
    };
}

#endif