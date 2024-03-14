#include "pch.h"
#include "Wrappers.h"

#ifdef __APPLE__
#include <CoreGraphics/CGDisplayConfiguration.h>
#include <dlfcn.h>
#endif
namespace Toad
{

	void GetDesktopDimensions(int& width, int& height)
	{
#ifdef _WIN32
		HWND window = GetDesktopWindow();
		RECT r;
		GetWindowRect(window, &r);
		width = r.right - r.left;
		height = r.bottom - r.top;
#else
		CGDirectDisplayID id = CGMainDisplayID();
		width = CGDisplayPixelsWide(id);
		height = CGDisplayPixelsHigh(id);
#endif
	}

	DllHandle DLibOpen(std::string_view name)
	{
#ifdef _WIN32
		return LoadLibraryA(name.data());
#else	
		return dlopen(name.data(), RTLD_NOW | RTLD_GLOBAL);
#endif
	}

	void DLibFree(DllHandle handle)
	{
#ifdef _WIN32
		FreeLibrary(handle);
#else
		dlclose(handle);
#endif
	}

	DllAddress DLibGetAddress(DllHandle handle, std::string_view name)
	{
#ifdef _WIN32
		return GetProcAddress(handle, name.data());
#else
		return dlsym(handle, name.data());
#endif
	}

}