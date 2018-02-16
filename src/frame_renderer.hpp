#ifndef FRAME_RENDERER_HPP_INCLUDED
#define FRAME_RENDERER_HPP_INCLUDED

#include "nvenc/frame_buffer.hpp"
#include "fractal_params.hpp"

namespace fractal
{
    void render_frame(const fractal_params_t& fractal_params, frame_buffer_t& buffer,size_t frame_idx);
}

#endif