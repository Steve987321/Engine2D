-- premake5.lua

workspace "Onion"
    architecture "x64"
    configurations { 
        "Debug", 
        "Release", 
        "Dev"
    }

    startproject "ToadRunner"
    
output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs {
        "%{prj.name}/src",
        "Game/src",
        "vendor",
        "vendor/imgui",
        "vendor/sfml-imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include"
    }

    libdirs {
        "vendor/SFML-2.6.0/lib"
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
            "opengl32"
        }

    pchheader "pch.h"
    pchsource "%{prj.name}/src/pch.cpp"

    filter "files:Engine/src/imgui_impl/**.cpp"
        flags {"NoPCH"}

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
        }
        filter "system:macosx"
            links {
                "sfml-system",
                "sfml-window",
                "sfml-graphics",
                "sfml-audio",
            }
        filter "system:windows"
            links {
                "sfml-system-d",
                "sfml-window-d",
                "sfml-graphics-d",
                "sfml-audio-d",
            }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT"
        }
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
        runtime "Release"
        optimize "On"
    
    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }
        filter "system:macosx"
            links {
                "sfml-system",
                "sfml-window",
                "sfml-graphics",
                "sfml-audio",
            }
        filter "system:windows"
            links {
                "sfml-system-d",
                "sfml-window-d",
                "sfml-graphics-d",
                "sfml-audio-d",
            }
        runtime "Debug"
        symbols "On"
    
project "Game"
    location "Game"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs {
        "Engine/src",
        "vendor",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }

    libdirs {
        "vendor/SFML-2.6.0/lib"
    }

    links {
        "Engine",
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
            "opengl32"
        }

    pchheader "pch.h"
    pchsource "%{prj.name}/src/pch.cpp"

    filter "system:windows"
        staticruntime "off"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_WINDOWS",
            "_USRDLL"
        }
       filter "system:macosx"
           links {
               "sfml-system",
               "sfml-window",
               "sfml-graphics",
               "sfml-audio",
           }
       filter "system:windows"
           links {
               "sfml-system-d",
               "sfml-window-d",
               "sfml-graphics-d",
               "sfml-audio-d",
           }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT"
        }
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
        runtime "Release"
        optimize "On"
    
    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
            "_USRDLL"
        }
        filter "system:macosx"
            links {
                "sfml-system",
                "sfml-window",
                "sfml-graphics",
                "sfml-audio",
            }
        filter "system:windows"
            links {
                "sfml-system-d",
                "sfml-window-d",
                "sfml-graphics-d",
                "sfml-audio-d",
            }
        runtime "Debug"
        symbols "On"

project "ToadRunner"
    location "ToadRunner"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }

    includedirs {
        "%{prj.name}/src",
        "%{prj.name}/src/scripts",
        "Engine/src",
        "vendor",
        "vendor/imgui",
        "vendor/sfml-imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
    }

    libdirs {
        "vendor/SFML-2.6.0/lib"
    }

    links {
        "Engine",
        "Game",
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
            "opengl32"
        }

    pchheader "pch.h"
    pchsource "%{prj.name}/src/pch.cpp"

    filter "files:ToadRunner/src/imgui_impl/**.cpp"
        flags {"NoPCH"}

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_CONSOLE",
        }
        filter "system:macosx"
            links {
                "sfml-system",
                "sfml-window",
                "sfml-graphics",
                "sfml-audio",
            }
        filter "system:windows"
            links {
                "sfml-system-d",
                "sfml-window-d",
                "sfml-graphics-d",
                "sfml-audio-d",
            }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_CONSOLE",
        }

        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
        optimize "On"

    filter "configurations:Dev" 
        defines {
            "_DEBUG",
            "_CONSOLE",
            "TOAD_EDITOR"
        }
       filter "system:macosx"
           links {
               "sfml-system",
               "sfml-window",
               "sfml-graphics",
               "sfml-audio",
           }
       filter "system:windows"
           links {
               "sfml-system-d",
               "sfml-window-d",
               "sfml-graphics-d",
               "sfml-audio-d",
           }
        runtime "Debug"
        symbols "On"
