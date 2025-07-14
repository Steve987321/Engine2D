#pragma once

#include <string_view>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#include "CPPBinding.h"

class VM;
struct VMRegister;

namespace IO
{
	// Writes to stdout
	VMRegister WriteOut(VM& vm, const std::vector<VMRegister>& args);

	VMRegister Nothing(VM& vm, const std::vector<VMRegister>& args);

	CPPLib GetIOLib();
}
