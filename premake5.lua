require "scripts/ecc"
require "scripts/cmake"

workspace "Onion"
    architecture "x64"
    configurations { 
        "Debug", 
        "Release", 
        "ReleaseNoEditor",
        "DebugNoEditor",
        "Distro",
    }

    startproject "ToadRunner"
    
output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

if _ACTION ~= "cmake" then 
group "GameTemplates"
    include "GameTemplates/Game"
    include "GameTemplates/RayCaster"
group ""
end 

include "Engine"
include "ToadRunner"
