#include "Compiler.h"
#include "Lexer.h"
#include "Parser.h"

#include <iostream>
#include <vector>

namespace Compiler
{

CompileResult CompileString(std::string_view str, std::vector<VM::Instruction>& res, VM* vm)
{
	error_msgs.clear();
	for (void* p : strings)
		free(p);
	strings.clear();
	
	std::cout << str << std::endl;
	LexerInit(str);

	std::vector<Token> tokens;
	if (!LexerScan(tokens))
		return CompileResult::ERR;

	// to vm bytecodes
	if (!Parse(tokens, res, vm))
		return CompileResult::ERR;
	
	return CompileResult::NONE;
}

}
