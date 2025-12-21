#include "pch.h"
#include "DLib.h"
#include <EngineCore.h>

namespace Toad
{
	DllHandle DLibOpen(std::string_view name)
	{
#ifdef _WIN32
		return LoadLibraryA(name.data());
#else	
		return dlopen(name.data(), RTLD_NOW | RTLD_GLOBAL);
#endif
	}

	void DLibFree(DllHandle& handle)
	{
#ifdef _WIN32
		FreeLibrary(handle);
#else
		dlclose(handle);
#endif
        handle = nullptr;
	}

	DllAddress DLibGetAddress(DllHandle handle, std::string_view name)
	{
#ifdef _WIN32
		return GetProcAddress(handle, name.data());
#else
		return dlsym(handle, name.data());
#endif
	}

	std::string DLGetError()
	{
#ifdef _WIN32
		// #TODO: look at FormatMessage
		return std::to_string(GetLastError());
#else
        char* err= dlerror(); 
        if (!err)
            return "none";
        else 
		    return err;
#endif  
	}
}