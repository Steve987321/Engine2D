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
        "Debug", 
        "Dev", -- same as release but defines TOAD_EDITOR
        "DevDebug", -- with debugging symbols and debugging runtime library
    }
    
    startproject (game_project_name)

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
        engine_path .. "/Engine/src",
        "vendor",
        "vendor/imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
        "%{prj.name}/src",
        "%{prj.name}/src/assets"
    }
    libdirs{
        engine_path .. "/bin/Release-%{cfg.system}-x86_64",
        "vendor/SFML-2.6.0/lib",
    }
else
    includedirs{
        engine_path .. "/script_api",
        "vendor",
        "vendor/imgui",
        "vendor/SFML-2.6.0/include",
        "vendor/json/include",
        "%{prj.name}/src",
        "%{prj.name}/src/assets",
    }
    libdirs{
        engine_path .. "/libs",
        "vendor/SFML-2.6.0/lib",
    }
end 

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations

    filter "configurations:Release"
        defines{
            "NDEBUG",
            "_USRDLL",
            "GAME_IS_EXPORT",
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Dev"
        defines{
            "NDEBUG",
            "_USRDLL",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
        }
        
        runtime "Release"
        symbols "Off"
        optimize "On"

    -- ?? ??? ? DELET?? ?? ??
    filter "configurations:DevDebug"
        defines{
            "_DEBUG",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
        }

        runtime "Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Debug"
        defines{
            "_DEBUG",
            "GAME_IS_EXPORT",
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

        defines{
            "_WINDOWS",
        }

    -- sfml 
    filter "system:macosx"
        links {
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }

    filter {"system:windows", "configurations:DevDebug or configurations:Debug"}
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
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
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
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
