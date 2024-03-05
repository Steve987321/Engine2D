#include "pch.h"
#include "Wrappers.h"

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
	
#endif
	}

	DllHandle DLibOpen(std::string_view name)
	{
#ifdef _WIN32
		return LoadLibraryA(name.data());
#else
		return dlopen(name.data());
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
		return dlclose(handle, name.data());
#endif
	}

}