#include "cuda_context.hpp"

namespace fractal
{
    cuda_context_t::cuda_context_t()
    {
        int device_count;
        cuInit(0);
        cuDeviceGetCount(&device_count);
        cuDeviceGet(&device, 0);
        cuCtxCreate(&context, CU_CTX_SCHED_BLOCKING_SYNC, device);
    }
    
    cuda_context_t::~cuda_context_t()
    {
        cuCtxDestroy(context);
    }
    
    void cuda_context_t::use_context_for_this_thread()
    {
        cuCtxPushCurrent(context);
    }
    
    
}