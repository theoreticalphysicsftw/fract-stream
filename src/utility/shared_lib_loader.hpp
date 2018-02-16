#ifndef SHARED_LIB_LOADER_HPP_INCLUDED
#define SHARED_LIB_LOADER_HPP_INCLUDED

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

namespace utility
{
	
#if defined(_WIN32)
	typedef HMODULE dll_handle_t;
#elif defined(__linux__)
	typedef void* dll_handle_t;
#endif

	dll_handle_t load_shared_lib(const char*);
	void* load_symbol(dll_handle_t, const char*);
    void unload_lib(dll_handle_t);
}

#endif

