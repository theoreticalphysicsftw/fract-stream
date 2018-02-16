#ifndef FRAME_BUFFER_HPP_INCLUDED
#define FRAME_BUFFER_HPP_INCLUDED

#include <cstdint>
#include "nvenc_encoder.hpp"
#include "../cuda/cuda_context.hpp"

// TODO : implement the frame buffer (add auto registration, destruction, mapping etc..>)
namespace fractal
{

    
    struct frame_buffer_t
    {
        typedef nvenc_encoder_t::output_ptr_t output_ptr_t;
        typedef nvenc_encoder_t::input_ptr_t input_ptr_t;
        
        CUdeviceptr nv12_frame;
        size_t nv12_frame_stride;
        
        size_t height;
        size_t width;
        
        void* registered_resource;
        input_ptr_t input_buffer;
        output_ptr_t output_buffer;
        
        uint8_t pad[128];
        
        frame_buffer_t(uint32_t height, uint32_t width);
        ~frame_buffer_t();
        
        bool map();
        bool unmap();
        bool encode();
        bool encode_eos();
        
        bool lock_output(void*& ptr, uint32_t& size);
        bool unlock_output();
    };
}

#endif

