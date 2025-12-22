#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <stack>
#include <set>

#include "Lib/CPPBinding.h"

enum OP_CODE
{
	OP_DEFINE_LABEL,
	OP_MOVE,
    OP_MOVE_ONCE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NOT,
	OP_CALL,
    OP_CALL_MOVE, // first arg is destination rest is function args
	OP_JUMP,
	OP_JUMP_IF_NOT_EQUAL,
	OP_JUMP_IF_EQUAL,
    OP_JUMP_IF_LESS,
    OP_JUMP_IF_GREATER,
    OP_JUMP_ONCE,
	//OP_SETRELREGINDEX, // for the vm to handle
	OP_RETURN,
	BYTE_CODE_COUNT,
};

enum class VMRegisterType
{
	INVALID, // invalid
	REGISTER, // for args
	FUNCTION, // for parser
	STRING,
	FLOAT,
	INT,
};

//struct VMString
//{
//    char* start;
//    size_t size;
//};

union VMRegisterValue
{
	int32_t num;
	float flt;
	const char* str;
};

// Register value info 
struct VMRegister
{
	VMRegisterValue value;
	VMRegisterType type; // zero initialized to INVALID
};

class VM
{
public:
	std::array<VMRegister, 1024> registers;

	uint64_t relative_register_index = 0;
	std::stack<uint64_t> rel_reg_stack;
    std::stack<uint64_t> ip_stack;

	uint64_t instruction_pointer = 0;
	uint64_t pre_label_ip = 0;

	struct Instruction
	{
		OP_CODE op;
		std::vector<VMRegister> args;
		VMRegister reserved;
	};

	std::vector<Instruction> instructions;

	// id - ip 
	std::unordered_map<uint64_t, uint64_t> labels;
	//// ... means vector of registers
	//// . means 1 register 
	//// int, string, explicit type requires that type 
	/// #todo cpp binder 
	std::unordered_map<std::string_view, CPPFunction> functions;
    std::vector<CPPLib> used_cpp_libs;
    std::set<uint64_t> skip_instructions;
	void Init();

	void Run();
    
    // returns non register type 
    const VMRegister* GetValueReg(const VMRegister& reg);
    
	void OpMove(const VMRegister& dst, const VMRegister& src);
    void OpMoveOnce(const VMRegister& dst, const VMRegister& src);
	void OpAdd(const VMRegister& dst, const VMRegister& a, const VMRegister& b);
	void OpSubtract(const VMRegister& dst, const VMRegister& a, const VMRegister& b);
	void OpMultiply(const VMRegister& dst, const VMRegister& a, const VMRegister& b);
	void OpDivide(const VMRegister& dst, const VMRegister& a, const VMRegister& b);
	void OpLabel(const VMRegister& a, size_t ip); // this is diff
    void OpJump(const VMRegister& jump);
    void OpJumpOnce(const VMRegister& jump);
    void OpReturn();
    
	// jump to ip or label depending on type
	void OpJumpIfNotEqual(const VMRegister& jump, const VMRegister& a, const VMRegister& b);
    void OpJumpIfEqual(const VMRegister& jump, const VMRegister& a, const VMRegister& b);
    void OpJumpIfLess(const VMRegister& jump, const VMRegister& a, const VMRegister& b);
    void OpJumpIfGreater(const VMRegister& jump, const VMRegister& a, const VMRegister& b);
    
    void OpCall(const std::vector<VMRegister>& args);
    void OpCallMove(const VMRegister& dst, const std::vector<VMRegister>& args);
    VMRegister& GetReg(uint64_t index);

	// returns anything other then type REGISTER
	VMRegister& GetValueAtReg();
    
	// helper for registering cpp libraries to vm 
	void RegisterLib(const CPPLib& lib);

private:
	// arg is usually ip 
	void BeginRelativeRegIndex(uint64_t rel);
	void EndRelativeRegIndex();
};
