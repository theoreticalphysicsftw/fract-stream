#include "shared_lib_loader.hpp"
    
namespace utility
{
    dll_handle_t load_shared_lib(const char* libpath)
    {
#if  defined(_WIN32)
        return LoadLibrary(libpath);
#elif defined(__linux__)
        return dlopen(libpath,RTLD_LAZY);
#endif
    }
    
    void* load_symbol(dll_handle_t handle, const char* symbol)
    {
#if  defined(_WIN32)
        return GetProcAddress(handle,symbol);
#elif defined(__linux__)
        return dlsym(handle,symbol);
#endif
	}
    
    void unload_lib(dll_handle_t handle)
    {
#if  defined(_WIN32)
        FreeLibrary(handle);
#elif defined(__linux__)
        dlclose(handle);
#endif
    }
}
