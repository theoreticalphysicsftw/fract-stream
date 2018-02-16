#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>

#if defined(__linux__)
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include "cuda_module_loader.hpp"

namespace fractal
{
   
    cuda_module_loader_t::cuda_module_loader_t()
    {
        static constexpr const char* module_name = "render_fractal.cubin";
        static constexpr const char* kernel_name_float = "render_fractal_float";
        static constexpr const char* kernel_name_double = "render_fractal_double";
        static constexpr size_t buff_size = 256;
        
        std::string path;
        
        
        #if defined(__linux__)
        
        path.resize(buff_size);
        path.resize(readlink("/proc/self/exe",std::addressof(path[0]),buff_size));
        path = path.substr(0,path.find_last_of("/"));
        
        #elif defined(_WIN32)
        //TODO : implement for Windows using GetModuleFileNameEx();
        #endif
        
        (path+="/")+=module_name;
        
        
        if (cuModuleLoad(&module, path.c_str())!=CUDA_SUCCESS)
            {
                printf("Failed to load %s!\n",path.c_str()); 
                exit(EXIT_FAILURE);
            }
            
        if (cuModuleGetFunction(&kernel_float,module, kernel_name_float)!=CUDA_SUCCESS)
            {
                printf("Failed to load the kernel %s inside %s!\n",kernel_name_float,path.c_str()); 
                exit(EXIT_FAILURE);
            }
            
        if (cuModuleGetFunction(&kernel_double,module, kernel_name_double)!=CUDA_SUCCESS)
            {
                printf("Failed to load the kernel %s inside %s!\n",kernel_name_double,path.c_str()); 
                exit(EXIT_FAILURE);
            }
        
        cuStreamCreate(&stream,CU_STREAM_NON_BLOCKING);
    }
    
    cuda_module_loader_t::~cuda_module_loader_t()
    {
        cuModuleUnload(module);
        cuStreamDestroy(stream);
    }

}
