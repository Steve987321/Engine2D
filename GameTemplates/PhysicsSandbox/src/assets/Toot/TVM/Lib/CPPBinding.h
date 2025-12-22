#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

#define REGISTER_LIBFUNC(lib, function, args)                                \
lib.functions.emplace_back(function, #function, args, #function " " args)    \

class VM;
struct VMRegister;
enum class VMRegisterType;

class CPPFunction
{
public:
	using TFunction = std::function<VMRegister(VM& vm, const std::vector<VMRegister>&)>;

	TFunction func = nullptr;
	const char* function_name;
	const char* accepted_args;
	const char* function_sig; // function_name + accepted_args
};

struct CPPLib
{
	std::string name;
	std::unordered_map<std::string_view, VMRegister> vars;
	std::vector<CPPFunction> functions;
};

