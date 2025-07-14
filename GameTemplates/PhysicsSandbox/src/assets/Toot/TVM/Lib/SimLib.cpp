#include "SimLib.h"
#include "Toot/TVM/TVM.h"

#include "framework/Framework.h"
#include "scripts/Sim.h"
#include "Fizzix/FZSim.h"

static const VMRegister* GetRegVal(VM& vm, const VMRegister& reg)
{
    if (reg.type == VMRegisterType::REGISTER)
    {
        const VMRegister* res = &vm.GetReg(reg.value.num);
        
        while (res->type == VMRegisterType::REGISTER)
            res = &vm.GetReg(reg.value.num);
        
        return res;
    }
    else
    {
        return &reg;
    }
}

VMRegister SimLib::DoSome(VM &vm, const std::vector<VMRegister> &args)
{
    fz::Sim& sim = Sim::GetSim();
    if (sim.polygons.empty())
        return {};
    
    Toad::Vec2f add;
    const VMRegister* x_reg = GetRegVal(vm, args[1]);
    const VMRegister* y_reg = GetRegVal(vm, args[2]);

    add.x = x_reg->value.flt;
    add.y = y_reg->value.flt;

    sim.polygons[0].rb.velocity += add;

    return {};
}

VMRegister SimLib::GetSome(VM& vm, const std::vector<VMRegister>& args)
{
    VMRegister res; 
    res.type = VMRegisterType::FLOAT;
    res.value.flt = 0.5f;
    return res;
}

VMRegister SimLib::GetY(VM& vm, const std::vector<VMRegister>& args)
{
    fz::Sim& sim = Sim::GetSim();
    VMRegister res;
    res.type = VMRegisterType::FLOAT;
    res.value.flt = sim.polygons[0].rb.center.y;
    return res; 
}

VMRegister SimLib::GetDY(VM& vm, const std::vector<VMRegister>& args)
{
    fz::Sim& sim = Sim::GetSim();
    VMRegister res;
    res.type = VMRegisterType::FLOAT;
    res.value.flt = Sim::d_y;
    return res; 
}

VMRegister SimLib::DrawCrossXY(VM &vm, const std::vector<VMRegister> &args)
{
    const VMRegister* x_reg = GetRegVal(vm, args[1]);
    const VMRegister* y_reg = GetRegVal(vm, args[2]);

    float cross_size = 10.f;
    Toad::Vec2f position = {x_reg->value.flt, y_reg->value.flt};
    position -= cross_size / 2.f;

    Toad::DrawingCanvas::DrawArrow(position, {cross_size, cross_size},  2.f);    
    position.y += cross_size / 2.f;
    Toad::DrawingCanvas::DrawArrow(position, {-cross_size, -cross_size},  2.f);    
   
    return {};
}

VMRegister SimLib::GetDT(VM &vm, const std::vector<VMRegister> &args)
{
    VMRegister res;
    res.type = VMRegisterType::FLOAT;
    res.value.flt = Toad::Time::GetFixedDeltaTime();
    return res; 
}

VMRegister SimLib::IsKeyDown(VM &vm, const std::vector<VMRegister> &args)
{
    const VMRegister* key = GetRegVal(vm, args[1]);
    VMRegister res; 
    res.type = VMRegisterType::INT;
    res.value.num = Toad::Input::IsKeyDown((Toad::Keyboard::Key)key->value.num);
    return res;
}

// CAR ENVIRONMENT 

static std::vector<float> spring_dist_state;

VMRegister SimLib::CESaveSpringStates(VM &vm, const std::vector<VMRegister> &args)
{
    fz::Sim& sim = Sim::GetSim(); 
    for (fz::Spring& spr : sim.springs)
    {
        spring_dist_state.emplace_back(spr.target_len);
    }
    return {};
}

VMRegister SimLib::CESetSpringDistanceFactor(VM &vm, const std::vector<VMRegister> &args)
{
    const VMRegister* stiffness_arg = GetRegVal(vm, args[1]);
    fz::Sim& sim = Sim::GetSim();  
    size_t i = 0;

    for (fz::Spring& spr : sim.springs)
    {
        if (i < spring_dist_state.size())
            spr.target_len = spring_dist_state[i] * stiffness_arg->value.flt;
        i++;
    }

    return {};
}

VMRegister SimLib::CEApplyGas(VM &vm, const std::vector<VMRegister> &args)
{
    const VMRegister* gas_arg = GetRegVal(vm, args[1]);
    Sim::env_car_gas = gas_arg->value.flt;
    return {};
}

CPPLib SimLib::GetSimLib()
{
    CPPLib l;
    l.name = "SimLib";
    
    REGISTER_LIBFUNC(l, DoSome, "registerregister");
    REGISTER_LIBFUNC(l, DrawCrossXY, "registerregister");
    REGISTER_LIBFUNC(l, GetY, "");
    REGISTER_LIBFUNC(l, GetDY, "");
    REGISTER_LIBFUNC(l, GetSome, "");
    REGISTER_LIBFUNC(l, GetDT, "");
    REGISTER_LIBFUNC(l, IsKeyDown, "register");

    REGISTER_LIBFUNC(l, CESaveSpringStates, "");
    REGISTER_LIBFUNC(l, CESetSpringDistanceFactor, "register");
    REGISTER_LIBFUNC(l, CEApplyGas, "register");
    
    return l;
}
