project "ToadRunner"
    -- filter "configurations:ReleaseNoEditor"
        -- kind "WindowedApp"
    -- filter "configurations:DebugNoEditor or configurations:Debug or configurations:Release or configurations:Distro"
    filter "system:windows"
        kind "WindowedApp"
    filter "system:macosx"
        kind "ConsoleApp"
    filter {}
    
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
        "../vendor/sfml-imgui/imgui-SFML.cpp",
        "../vendor/pugixml/**.cpp",
    }

    removefiles {
        "../vendor/imgui/examples/**",
        "../vendor/imgui/misc/**",
        "../vendor/imgui/backends/**",
    }

    include_dirs = 
    {
        "src",
        "src/scripts",
        "../Engine/src",
        "../vendor",
        "../vendor/imgui",
        "../vendor/sfml-imgui",
        "../vendor/SFML-3.0.0/include",
        "../vendor/json/include",
        "../vendor/magic_enum/include"
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

    links {
        "Engine",
        -- "Game",
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
        links{
            "opengl32",
            "ws2_32",
        }

        pchheader "pch.h"
        pchsource "src/pch.cpp"

    filter "files:../vendor/**.cpp"
        flags {"NoPCH"}
    
    filter "files:src/**.mm"
        flags {"NoPCH"}
        
    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations 

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "TOAD_EDITOR"
        }

        runtime "Debug"
        symbols "On"

    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_CONSOLE",
            "TOAD_EDITOR",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Distro" 
        defines {
            "NDEBUG",
            "_CONSOLE",
            "TOAD_EDITOR",
            "TOAD_DISTRO",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:ReleaseNoEditor"
        defines {
            "NDEBUG",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:DebugNoEditor"
        defines {
            "_DEBUG",
            "_CONSOLE",
        }

        runtime "Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:TestNoEditor"
        defines {
            "_DEBUG",
        }

        runtime "Release"
        symbols "On"
        optimize "On"

    filter "configurations:DebugNoEditor or configurations:ReleaseNoEditor or configurations:TestNoEditor"
        removefiles{
            "src/ui/**.cpp",
            "src/ui/**.h",
            "src/project/**.cpp",
            "src/project/**.h",
            "vendor/imgui/**.cpp",
            "vendor/imgui/**.h",
        }

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
    filter {"system:windows", "configurations:ReleaseNoEditor or configurations:Release or configurations:Distro"}
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
            "sfml-network",
        }