#include "CPPBinding.h"

class VM;
struct VMRegister;

namespace SimLib
{
	// tesing 
	VMRegister DoSome(VM& vm, const std::vector<VMRegister>& args);
	VMRegister GetSome(VM& vm, const std::vector<VMRegister>& args);
	VMRegister GetY(VM& vm, const std::vector<VMRegister>& args);
	VMRegister GetTargetY(VM& vm, const std::vector<VMRegister>& args);
	VMRegister DrawCrossXY(VM& vm, const std::vector<VMRegister>& args);
	VMRegister GetDT(VM& vm, const std::vector<VMRegister>& args);
	VMRegister IsKeyDown(VM& vm, const std::vector<VMRegister>& args);

	// for the car environment 
	VMRegister CESaveSpringStates(VM& vm, const std::vector<VMRegister>& args);
	VMRegister CESetSpringDistanceFactor(VM& vm, const std::vector<VMRegister>& args);
	VMRegister CEApplyGas(VM& vm, const std::vector<VMRegister>& args);

    
	
	CPPLib GetSimLib();
}
