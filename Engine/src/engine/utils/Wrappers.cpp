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
}