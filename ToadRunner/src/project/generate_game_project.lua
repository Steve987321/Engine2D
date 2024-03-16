--premake5.lua

newoption{
    trigger = "enginepath",
    description = "Set the engine installation path"
}

newoption{
    trigger = "projectname",
    description = "The name of the game project"
}

newoption{
    trigger = "usesrc",
    description = "Set if using a version of the engine that has the source code"
}

if not _OPTIONS["enginepath"] then
error("Error: The --enginepath argument is required.")
end

if not _OPTIONS["projectname"] then
error("Error: The --projectname argument is required.")
end

output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
engine_path = _OPTIONS["enginepath"]
game_project_name = _OPTIONS["projectname"] .. "_Game"

workspace(_OPTIONS["projectname"])
    architecture "x64"
    configurations{
        "Release",
        "Dev", -- same as release but defines TOAD_EDITOR
        "DevDebug", -- with debugging symbols and debugging runtime library
    }
    
    startproject (game_project_name)

if _OPTIONS["usesrc"] then 
project "Engine"
    location "Engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir("bin/" ..output_dir .. "/")
    objdir("bin-intermediate/" ..output_dir .. "/")

    files {
        engine_path .. "/src/**.cpp",
        engine_path .. "/src/**.h",
        "vendor/imgui/**.cpp",
        "vendor/sfml-imgui/imgui-SFML.cpp",
    }

    removefiles {
        "vendor/imgui/examples/**",
        "vendor/imgui/misc/**",
        "vendor/imgui/backends/**",
    }

    includedirs{
        engine_path .. "/src",
        game_project_name .. "/src",
        "vendor",
        "vendor/imgui",
        "vendor/sfml-imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include"
    }

    libdirs{
        "vendor/SFML-2.6.0/lib"
    }

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"
        
    -- configurations 

    filter "configurations:Release"
        defines{
            "NDEBUG",
            "ENGINE_IS_EXPORT"
        }

        runtime "Release"
        optimize "On"
        symbols "Off"

    filter "configurations:Dev"
        defines{
            "NDEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }

        runtime "Release"
        optimize "On"
        symbols "Off"
       
    filter "configurations:DevDebug"
        defines{
            "_DEBUG",
            "_CONSOLE",
            "ENGINE_IS_EXPORT",
            "TOAD_EDITOR"
        }

        runtime "Debug"
        optimize "Off"
        symbols "On"
        
    -- platform 

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

        staticruntime "Off"
        systemversion "latest"

    -- sfml 
    filter "system:macosx"
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

    filter {"system:windows", "configurations:DevDebug"}
        defines {
            "SFML_STATIC",
        }
        links {
            "winmm",
            "gdi32",
            "freetype",
            "flac",
            "ogg",
            "openal32",
            "vorbis",
            "vorbisenc",
            "vorbisfile",
            "sfml-system-s-d",
            "sfml-window-s-d",
            "sfml-graphics-s-d",
            "sfml-audio-s-d",
        }
    filter {"system:windows", "configurations:Release or configurations:Dev"}
        defines {
            "SFML_STATIC",
        }
        links {
            "winmm",
            "gdi32",
            "freetype",
            "flac",
            "ogg",
            "openal32",
            "vorbis",
            "vorbisenc",
            "vorbisfile",
            "sfml-system-s",
            "sfml-window-s",
            "sfml-graphics-s",
            "sfml-audio-s",
        }
end 

project(game_project_name)
    location(game_project_name)
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir("bin/" ..output_dir .. "/")
    objdir("bin-intermediate/" ..output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h",
        "vendor/imgui/**.cpp",
        "vendor/sfml-imgui/imgui-SFML.cpp",
    }

    removefiles {
        "vendor/imgui/examples/**",
        "vendor/imgui/misc/**",
        "vendor/imgui/backends/**",
    }

if _OPTIONS["usesrc"] then 
    includedirs{
        engine_path .. "/src",
        "vendor",
        "vendor/imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }
    libdirs{
        "vendor/SFML-2.6.0/lib"
    }
else
    includedirs{
        engine_path .. "/script_api",
        "vendor",
        "vendor/imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
        "%{prj.name}/src",
        "%{prj.name}/src/scripts"
    }
    libdirs{
        engine_path .. "/libs",
        "vendor/SFML-2.6.0/lib"
    }
end 

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations

    filter "configurations:Release"
        defines{
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT"
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Dev"
        defines{
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
        }
        
        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:DevDebug"
        defines{
            "_DEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
        }

        runtime "Debug"
        symbols "On"
        optimize "Off"

    -- platform 

    filter "system:macosx"
        links {
            "Engine",
            "OpenGL.framework",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
        }

    filter "system:windows"
        links {
            "Engine",
            "opengl32"
        }

    -- sfml 
    filter "system:macosx"
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

    filter {"system:windows", "configurations:DevDebug"}
        defines {
            "SFML_STATIC",
        }
        links {
            "winmm",
            "gdi32",
            "freetype",
            "flac",
            "ogg",
            "openal32",
            "vorbis",
            "vorbisenc",
            "vorbisfile",
            "sfml-system-s-d",
            "sfml-window-s-d",
            "sfml-graphics-s-d",
            "sfml-audio-s-d",
        }
    filter {"system:windows", "configurations:Release or configurations:Dev"}
        defines {
            "SFML_STATIC",
        }
        links {
            "winmm",
            "gdi32",
            "freetype",
            "flac",
            "ogg",
            "openal32",
            "vorbis",
            "vorbisenc",
            "vorbisfile",
            "sfml-system-s",
            "sfml-window-s",
            "sfml-graphics-s",
            "sfml-audio-s",
        }

if _OPTIONS["usesrc"] then 
project "ToadRunner"
    location "ToadRunner"
    filter "configurations:Dev or configurations:DevDebug"
        kind "ConsoleApp"
    filter "configurations:Release"
        kind "WindowedApp"
    filter {}
    
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.mm",
        "vendor/imgui/**.cpp",
        "vendor/sfml-imgui/imgui-SFML.cpp",
        "vendor/pugixml/**.cpp",
    }

    removefiles {
        "vendor/imgui/examples/**",
        "vendor/imgui/misc/**",
        "vendor/imgui/backends/**",
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

    filter "files:vendor/**.cpp"
        flags {"NoPCH"}
    
    filter "files:ToadRunner/src/**.mm"
        flags {"NoPCH"}
        
    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations 

    filter "configurations:Release" 
        defines {
            "NDEBUG",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Dev" 
        defines {
            "NDEBUG",
            "_CONSOLE",
            "TOAD_EDITOR",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:DevDebug"
        defines {
            "_DEBUG",
            "_CONSOLE",
        }

        runtime "Debug"
        symbols "On"

    filter "system:macosx"
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
    
    filter {"system:windows", "configurations:DebugNoEditor or configurations:Debug"}
        defines {
            "SFML_STATIC",
        }
        links {
            "winmm",
            "gdi32",
            "freetype",
            "flac",
            "ogg",
            "openal32",
            "vorbis",
            "vorbisenc",
            "vorbisfile",
            "sfml-system-s-d",
            "sfml-window-s-d",
            "sfml-graphics-s-d",
            "sfml-audio-s-d",
        }
    filter {"system:windows", "configurations:ReleaseNoEditor or configurations:Release or configurations:Distro"}
        defines {
            "SFML_STATIC",
        }
        links {
            "winmm",
            "gdi32",
            "freetype",
            "flac",
            "ogg",
            "openal32",
            "vorbis",
            "vorbisenc",
            "vorbisfile",
            "sfml-system-s",
            "sfml-window-s",
            "sfml-graphics-s",
            "sfml-audio-s",
        }
end