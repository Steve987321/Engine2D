project "Engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"
    
    targetdir ("../bin/" .. output_dir .. "/")
    objdir ("../bin-intermediate/" .. output_dir .. "/")

    flags {"MultiProcessorCompile"}

    files {
        "src/**.cpp",
        "src/**.h",
        "src/**.mm",
        "../vendor/imgui/**.cpp",
        "../vendor/implot/**.cpp",
        "../vendor/sfml-imgui/imgui-SFML.cpp",
    }

    removefiles {
        "../vendor/imgui/examples/**",
        "../vendor/imgui/misc/**",
        "../vendor/imgui/backends/**",
    }

    include_dirs = 
    {
        "src",
        "../GameTemplates/Game/src",
        "../vendor",
        "../vendor/imgui",
        "../vendor/implot",
        "../vendor/sfml-imgui",
        "../vendor/SFML-3.0.0/include",
        "../vendor/json/include",
        "../vendor/magic_enum/include",
    }

if _ACTION == "xcode4" then
    externalincludedirs {
        include_dirs
    }
else 
    includedirs {
       include_dirs
    }
end 
    libdirs {
        "../vendor/SFML-3.0.0/lib"
    }

    filter "system:windows"   
        disablewarnings {
            "4275" -- C4275 can be ignored in Visual C++ if you are deriving from a type in the C++ Standard Library, from std::runtime_error 
        }

    filter "system:macosx"
        links {
            "OpenGL.framework",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
        }

    filter "system:windows"
        links {
            "opengl32",
            "ws2_32",
        }

        pchheader "pch.h"
        pchsource "src/pch.cpp"

    filter "files:../vendor/**.cpp"
        flags {"NoPCH"}

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations 

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }

        runtime "Debug"
        symbols "On"
        optimize "Off"
    
    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Distro"
        defines {
            "NDEBUG",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR",
            "TOAD_DISTRO",
        }

        runtime "Release"
        optimize "On"
        
    filter "configurations:ReleaseNoEditor"
        defines {
            "NDEBUG",
            "ENGINE_IS_EXPORT",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:DebugNoEditor"
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
        }

        runtime "Debug"
        optimize "Off"
        symbols "On"
    
    filter "configurations:TestNoEditor"
        defines {
            "_DEBUG",
            "ENGINE_IS_EXPORT",
        }

        runtime "Debug"
        symbols "On"
        optimize "On"

    filter "system:macosx"
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
            "sfml-network",
        }
    
    filter {"system:windows", "configurations:DebugNoEditor or configurations:Debug"}
        links {
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
            "sfml-network-d",
        }
    filter {"system:windows", "configurations:ReleaseNoEditor or configurations:Release or configurations:Distro or configurations:TestNoEditor"}
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
            "sfml-network",
        }
