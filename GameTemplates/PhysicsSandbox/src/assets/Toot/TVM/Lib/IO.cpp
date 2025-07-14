#include "IO.h"
#include "Toot/TVM/TVM.h"

#include <cassert>

namespace IO
{
	VMRegister WriteOut(VM& vm, const std::vector<VMRegister>& args)
	{
		// first arg is the function
		VMRegister reg = args[1];
		while (reg.type == VMRegisterType::REGISTER)
			reg = vm.GetReg(reg.value.num);

		switch (reg.type)
		{
		case VMRegisterType::FUNCTION:
			assert(false && "redart");
			break;
		case VMRegisterType::STRING:
            std::cout << reg.value.str;
			break;
		case VMRegisterType::FLOAT:
			std::cout << reg.value.flt;
			break;
		case VMRegisterType::INT:
			std::cout << reg.value.num;
			break;
		default:
			break;
		}
        std::cout << std::endl;
        
        return {};
	}

    VMRegister Nothing(VM &vm, const std::vector<VMRegister> &args)
	{
		VMRegister r;
		r.type = VMRegisterType::INT;
		r.value.num = 1234;
		return r;
	}

    CPPLib GetIOLib()
	{
		CPPLib l;
		l.name = "io";
		
		REGISTER_LIBFUNC(l, WriteOut, "...");
		REGISTER_LIBFUNC(l, Nothing, "");

		VMRegister pi{};
		pi.type = VMRegisterType::FLOAT;
		pi.value.flt = 3.141f;

		l.vars["PI"] = pi;

		return l;
	}
}
