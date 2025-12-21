#pragma once

namespace Toad
{

#ifdef _WIN32
	using DllHandle = HMODULE;
	using DllAddress = FARPROC;
#else
	using DllHandle = void*;
	using DllAddress = void*;
#endif 

	// Returns a dll handle by the given name 
	DllHandle DLibOpen(std::string_view name);

	// Closes a dll handle and sets to nullptr
	void DLibFree(DllHandle& handle);

	// Returns the addres of a symbol inside a dll handle 
	DllAddress DLibGetAddress(DllHandle handle, std::string_view name);

	// Returns a string status of last dll call 
	std::string DLGetError();

}
