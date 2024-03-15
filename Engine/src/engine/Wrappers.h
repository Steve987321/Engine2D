#pragma once

#ifdef _WIN32
using DllHandle = HMODULE;
using DllAddress = FARPROC;
#else
using DllHandle = void*;
using DllAddress = void*;
#endif 

namespace Toad
{
	void GetDesktopDimensions(int& width, int& height);

	DllHandle DLibOpen(std::string_view name);

	void DLibFree(DllHandle handle);
	
	DllAddress DLibGetAddress(DllHandle handle, std::string_view name);

	std::string DLGetError();
}