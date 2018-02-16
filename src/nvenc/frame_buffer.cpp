#include "frame_buffer.hpp"
#include <cuda.h>
#include <cstdint>

extern fractal::nvenc_encoder_t nvenc;
extern fractal::cuda_context_t cuda;

namespace fractal
{
    
    frame_buffer_t::frame_buffer_t(uint32_t height, uint32_t width) : height(height), width(width)
    {
        cuMemAllocPitch(&nv12_frame,&nv12_frame_stride,width,height*3/2,16);
        nvenc.register_cuda_devptr((void*)(uintptr_t)nv12_frame,registered_resource,nv12_frame_stride);
        nvenc.create_bitstream_buffer(output_buffer);
    }
    
    
    frame_buffer_t::~frame_buffer_t()
    {
        nvenc.unregister_cuda_devptr(registered_resource);
        cuMemFree(nv12_frame);
        nvenc.destroy_bitstream_buffer(output_buffer);
    }
    
    
    bool frame_buffer_t::map()
    {
        return nvenc.map_input_resource(registered_resource,input_buffer);
    }
    
    
    bool frame_buffer_t::unmap()
    {
        return nvenc.unmap_input_resource(input_buffer);
    }
    
    
    bool frame_buffer_t::encode()
    {
        return nvenc.encode_frame(input_buffer,output_buffer,nv12_frame_stride,false);
    }
    
    
    bool frame_buffer_t::encode_eos()
    {
        return nvenc.encode_frame(0,0,0,true);
    }
    
    
    bool frame_buffer_t::lock_output(void*& ptr, uint32_t& size)
    {
        return nvenc.lock_bitstream_buffer(output_buffer,ptr,size);
    }
    
    
    bool frame_buffer_t::unlock_output()
    {
        return nvenc.unlock_bitstream_buffer(output_buffer);
    }
}