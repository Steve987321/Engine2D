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

group "GameTemplates"
    include "GameTemplates/Game"
group ""

include "Engine"
include "ToadRunner"
