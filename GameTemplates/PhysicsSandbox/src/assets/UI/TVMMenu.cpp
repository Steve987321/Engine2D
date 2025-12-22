#if defined(TOAD_EDITOR) || !defined(NDEBUG)
#include "framework/Framework.h"

#include "engine/Engine.h"
#include "engine/PlaySession.h"

#include "Toot/TVM/TVM.h"
#include "Toot/Compiler/Compiler.h"
#include "Toot/Compiler/Parser.h"


// from parser.cpp 
static std::string InstructionToStr(VM::Instruction instr)
{
	std::string s = op_code_names[instr.op] + ' ';
	instr.args.emplace_back(instr.reserved);
	for (const VMRegister& arg : instr.args)
	{
		switch (arg.type)
		{
		case VMRegisterType::FLOAT:
			s += std::to_string(arg.value.flt);
			break;
		case VMRegisterType::INT:
			s += std::to_string(arg.value.num);
			break;
		case VMRegisterType::STRING:
			s += arg.value.str;
			break;
		case VMRegisterType::REGISTER:
			s += 'r' + std::to_string(arg.value.num);
			break;
		default:
			break;
		}
		s += ' ';
	}

	return s;
	// std::cout << s << std::endl;
}

namespace UI
{

void TVMMenu(VM& vm, bool& run_vm, char* source)
{
    ImGui::Begin("[Sim] command menu");

	//bool ImGui::InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)

	static std::vector<VM::Instruction> bytecodes;

	ImVec2 command_menu_window_size = ImGui::GetWindowSize();
	ImGui::BeginChild("Text Edit", {command_menu_window_size.x / 2.f - 10.f, command_menu_window_size.y});
	ImGui::InputTextMultiline("source", source, 1024, {command_menu_window_size.x, command_menu_window_size.y - 20.f});
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("VM options", {command_menu_window_size.x / 2.f - 10.f, command_menu_window_size.y});

	ImGui::BeginDisabled(!Toad::IsBeginPlay());
	if (ImGui::Button("Compile & Run"))
	{
		bytecodes.clear();
		run_vm = false;
		if (Compiler::CompileString(source, bytecodes, &vm) != Compiler::CompileResult::ERR)
		{
			vm.instructions = bytecodes;
			vm.Init();
			run_vm = true;
			vm.Run();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		run_vm = false;
	}
	ImGui::EndDisabled();
	if (ImGui::TreeNode("Compiled Bytecodes View1"))
	{
		int i = 0;
		for (const VM::Instruction& ins : bytecodes)
		{
			ImGui::TextColored({1, 1, 1, 0.5f}, "[%d]", i);
			ImGui::SameLine();
			ImGui::Text(" %s", InstructionToStr(ins).c_str());
			i++;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Compiled Bytecodes View2"))
	{
		for (const VM::Instruction& ins : bytecodes)
		{
			ImGui::Text("%d", (int)ins.op);
			for (const VMRegister& arg : ins.args)
			{
				ImGui::SameLine();

				switch (arg.type)
				{
					case VMRegisterType::FLOAT:
						ImGui::Text("%.2f", arg.value.flt);
						break;
					case VMRegisterType::INT:
					case VMRegisterType::REGISTER:
						ImGui::Text("%d", arg.value.num);
						break;
					default: 
						ImGui::Text("?");
						break;
				}
			}
			if (ins.reserved.type != VMRegisterType::INVALID)
			{
				ImGui::SameLine();

				switch (ins.reserved.type)
				{
					case VMRegisterType::FLOAT:
						ImGui::Text("%.2f", ins.reserved.value.flt);
						break;
					case VMRegisterType::INT:
					case VMRegisterType::REGISTER:
						ImGui::Text("%d", ins.reserved.value.num);
						break;
					default: 
						ImGui::Text("?");
						break;
				}
			}
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Errors", "Errors (%lu)", Compiler::error_msgs.size()))
	{
		for (const std::string& err : Compiler::error_msgs)
		{
			ImGui::TextColored({0.5f, 0, 0, 1}, "%s", err.c_str());
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("VM Registers", "VM Registers (%lu)", vm.registers.size()))
	{
		for (size_t i = 0; i < vm.registers.size(); i++)
		{
			const VMRegister& reg = vm.registers[i];

			if (reg.type == VMRegisterType::INVALID)
				break;

			switch(reg.type)
			{
				case VMRegisterType::REGISTER:
					ImGui::TextColored({1, 1, 1, 0.6f}, "Reg [%lu]", i);
					ImGui::SameLine();
					
					ImGui::TextColored({1,1,0,1}, "[Register]");
					ImGui::SameLine();
					ImGui::Text("%d", reg.value.num);
					break;
				case VMRegisterType::INT:
					ImGui::TextColored({1, 1, 1, 0.6f}, "Reg [%lu]", i);
					ImGui::SameLine();
					
					ImGui::TextColored({0.2f,0.2f,1,1}, "[Int]");
					ImGui::SameLine();
					ImGui::Text("%d", reg.value.num);
					break;
				case VMRegisterType::FLOAT:
					ImGui::TextColored({1, 1, 1, 0.6f}, "Reg [%lu]", i);
					ImGui::SameLine();
					
					ImGui::TextColored({0.2f,1,0.2f,1}, "[Float]");
					ImGui::SameLine();
					ImGui::Text("%f", reg.value.flt);
				break;

				default:
					break;
			}
		}
		ImGui::TreePop();

	}

	ImGui::EndChild();

	ImGui::End();
}

}
#endif 