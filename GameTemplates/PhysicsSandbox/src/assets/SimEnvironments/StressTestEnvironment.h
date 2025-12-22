#pragma once 

class Sim; 
namespace SimEnvironments
{
    // loads a pit for stress testing 
    // give count to spawn set initial object count 
    void StressTestEnvironmentLoad(size_t count);

    void StressTestImGui(Sim& sim_script);
}
