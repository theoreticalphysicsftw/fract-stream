#ifndef CUDA_CONTEXT_HPP_INCLUDED
#define CUDA_CONTEXT_HPP_INCLUDED

#include <cuda.h>

namespace fractal
{
    struct cuda_context_t
    {
        CUdevice device;
        CUcontext context;
        
        // We will need this in order to use the same context for multiple threads.
        void use_context_for_this_thread();
        cuda_context_t();
        ~cuda_context_t();
    };
}

#endif