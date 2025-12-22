#pragma once

#include "Lexer.h"
#include <vector>
#include <set>
#include "Toot/TVM/TVM.h"

inline std::unordered_map<OP_CODE, std::string> op_code_names
{
	{OP_CODE::OP_DEFINE_LABEL, "OP_DEFINE_LABEL"},
    {OP_CODE::OP_MOVE, "OP_MOVE"},
    {OP_CODE::OP_MOVE_ONCE, "OP_MOVE_ONCE"},
	{OP_CODE::OP_ADD, "OP_ADD"},
	{OP_CODE::OP_SUBTRACT, "OP_SUBTRACT"},
	{OP_CODE::OP_MULTIPLY, "OP_MULTIPLY"},
	{OP_CODE::OP_DIVIDE, "OP_DIVIDE"},
	{OP_CODE::OP_NOT, "OP_NOT"},
    {OP_CODE::OP_CALL, "OP_CALL"},
    {OP_CODE::OP_CALL_MOVE, "OP_CALL_MOVE"},
	{OP_CODE::OP_JUMP, "OP_JUMP"},
	{OP_CODE::OP_JUMP_ONCE, "OP_JUMP_ONCE"},
    {OP_CODE::OP_JUMP_IF_NOT_EQUAL, "OP_JUMP_IF_NOT_EQUAL"},
    {OP_CODE::OP_JUMP_IF_EQUAL, "OP_JUMP_IF_EQUAL"},
    {OP_CODE::OP_JUMP_IF_LESS, "OP_JUMP_IF_LESS"},
    {OP_CODE::OP_JUMP_IF_GREATER, "OP_JUMP_IF_GREATER"},
	{OP_CODE::OP_RETURN, "OP_RETURN"},
	{OP_CODE::BYTE_CODE_COUNT, "BYTE_CODE_COUNT"},
};

namespace Compiler
{
	void AddLibToParserCtx(const CPPLib& lib);

	// returns true if there is no errors
    // use optional vm arg to add context
	bool Parse(const std::vector<Token>& tokens, std::vector<VM::Instruction>& op_codes_res, VM* vm = nullptr);
}
