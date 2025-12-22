#pragma once

#include <vector>
#include <string>

#include "Toot/TVM/TVM.h"

namespace Compiler
{

    enum class CompileResult
    {
        NONE,
        ERR
    };

    inline std::vector<std::string> error_msgs;
    inline std::vector<void*> strings;
    
    // compile a string to vm bytecodes
    // optional vm arg to compile for that vm using its registered libraries 
    CompileResult CompileString(std::string_view str, std::vector<VM::Instruction>& res, VM* vm = nullptr);

}
