#ifndef FRAME_QUEUE_HPP_INCLUDED
#define FRAME_QUEUE_HPP_INCLUDED

#include <cstdint>
#include <vector>
#include <thread>
#include <condition_variable>
#include "nvenc/frame_buffer.hpp"
#include "fractal_params.hpp"

// TODO : Implement the queue logic

namespace fractal
{
    // Cyclic queue to be used by the rendering, encoding and serialization threads
    struct frame_queue_t
    {
        std::vector<frame_buffer_t> queue;
        
        short for_rendering_index;
        short for_encoding_count;
        short for_encoding_index;
        short for_serialization_count;
        short for_serialization_index;
        
        // 0 indicates that NvEncEncodePicture returned NV_ENC_ERR_NEED_MORE_INPUT
        short ready_for_serialization_count;
        
        std::mutex mutex;
        
        // Wait for buffer in which to render a frame;
        std::condition_variable wait_for_render_buffer;
        // Wait for buffer in which rendering has finished and is now ready for encoding.
        std::condition_variable wait_for_encode_buffer;
        // Wait for buffer in which encoding has finished and is now ready for serialization.
        std::condition_variable wait_for_serialization_buffer; 
        
        frame_queue_t(const fractal_params_t& params);
        
        frame_buffer_t& get_for_rendering();
        void end_rendering_frame();
        
        frame_buffer_t& get_for_encoding();
        void end_encoding_frame();
        
        frame_buffer_t& get_for_serialization();
        void end_serializing_frame();
        
        void signal_serialization();
        
    };
}

#endif