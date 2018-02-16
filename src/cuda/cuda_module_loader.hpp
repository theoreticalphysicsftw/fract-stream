#ifndef CUDA_MODULE_LOADER_HPP_INCLUDED
#define CUDA_MODULE_LOADER_HPP_INCLUDED

#include <cuda.h>

namespace fractal
{
    class cuda_module_loader_t
    {
        CUmodule module;
        CUstream stream;
        CUfunction kernel_float;
        CUfunction kernel_double;
        
    public:

        cuda_module_loader_t();
        ~cuda_module_loader_t();
        
            
        bool execute_float(void** params,unsigned num_blocks_x,unsigned num_blocks_y, unsigned num_threads_x, unsigned num_threads_y)
        {
            auto status = cuLaunchKernel(kernel_float,num_blocks_x,num_blocks_y,1,num_threads_x,num_threads_y,1,0,stream,params,nullptr);
            
            cuStreamSynchronize(stream);
            
            return status == CUDA_SUCCESS;
        }
        bool execute_double(void** params,unsigned num_blocks_x,unsigned num_blocks_y, unsigned num_threads_x, unsigned num_threads_y)
        {
            auto status = cuLaunchKernel(kernel_double,num_blocks_x,num_blocks_y,1,num_threads_x,num_threads_y,1,0,stream,params,nullptr);
            
            cuStreamSynchronize(stream);
            
            return status  == CUDA_SUCCESS;
        }
        
    };
}

#endif