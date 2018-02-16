#ifndef VIDEO_RENREDER_HPP_INCLUDED
#define VIDEO_RENREDER_HPP_INCLUDED

#include <cstdint>
#include "fractal_params.hpp"
#include "frame_queue.hpp"
#include "nvenc/nvenc_encoder.hpp"
#include "cuda/cuda_context.hpp"
#include "utility/timer.hpp"


namespace fractal
{
    
    struct timings_t
    {
        typedef utility::timer_t::duration_t duration_t;
        
        duration_t total_render_time;
        duration_t avg_render_time_per_frame;
        duration_t max_render_time_per_frame;
        duration_t min_render_time_per_frame;
        
        // Avoid false sharing
        uint8_t pad0[64];
        
        duration_t total_encode_time;
        duration_t avg_encode_time_per_input;
        duration_t max_encode_time_per_input;
        duration_t min_encode_time_per_input;
        
        // Avoid false sharing
        uint8_t pad1[64];
        
        duration_t total_serialization_time;
        duration_t avg_serialization_time_per_output;
        duration_t max_serialization_time_per_output;
        duration_t min_serialization_time_per_output;
        
    };
    
    timings_t render_video(const fractal_params_t& fractal_params);
    
    extern nvenc_encoder_t nvenc;
    extern cuda_context_t cuda;
}

#endif