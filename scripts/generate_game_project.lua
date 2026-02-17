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

newoption{
    trigger = "ownlibs",
    description = "Set if project uses copied libs"
}

if not _OPTIONS["enginepath"] then
error("Error: The --enginepath argument is required.")
end

if _ACTION == "cmake" then
require "scripts/cmake"
end 

output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
engine_path = _OPTIONS["enginepath"]
game_project_name = _OPTIONS["projectname"] .. "_Game"

if _OPTIONS["usesrc"] then 
    if _OPTIONS["ownlibs"] then
        -- use paths from own project folder 
        include_dirs = 
        {
            engine_path .. "/engine/src",
            "vendor",
            "vendor/magic_enum/include",
            "vendor/imgui",
            "vendor/implot",
            "vendor/SFML-3.0.0/include",
            "vendor/json/include",
            "%{prj.name}/src",
            "%{prj.name}/src/assets"
        }
        lib_dirs = 
        {
            engine_path .. "/bin/Release-%{cfg.system}-x86_64",
            engine_path .. "/vendor/SFML-3.0.0/lib",
        }
        sources = 
        {
            "%{prj.name}/src/**.cpp",
            "%{prj.name}/src/**.h",
            "vendor/imgui/**.cpp",
            "vendor/implot/**.cpp",
            "vendor/sfml-imgui/imgui-SFML.cpp",
        }
        remove_sources = 
        {
            "vendor/imgui/examples/**",
            "vendor/imgui/misc/**",
            "vendor/imgui/backends/**",
            "vendor/implot/examples/**",
        }
    else 
        -- use paths from the engine 
        include_dirs = 
        {
            engine_path .. "/engine/src",
            engine_path .. "/vendor",
            engine_path .. "/vendor/magic_enum/include",
            engine_path .. "/vendor/imgui",
            engine_path .. "/vendor/implot",
            engine_path .. "/vendor/SFML-3.0.0/include",
            engine_path .. "/vendor/json/include",
            "%{prj.name}/src",
            "%{prj.name}/src/assets"
        }
        lib_dirs = 
        {
            engine_path .. "/bin/Release-%{cfg.system}-x86_64",
            "vendor/SFML-3.0.0/lib",
        }
        sources = 
        {
            "%{prj.name}/src/**.cpp",
            "%{prj.name}/src/**.h",
            engine_path .. "/vendor/imgui/**.cpp",
            engine_path .. "/vendor/sfml-imgui/imgui-SFML.cpp",
            engine_path .. "/vendor/implot/**.cpp",
        }
        remove_sources = 
        {
            engine_path .. "/vendor/imgui/examples/**",
            engine_path .. "/vendor/imgui/misc/**",
            engine_path .. "/vendor/imgui/backends/**",
            engine_path .. "/vendor/implot/examples/**",
        }
    end 
else -- distro 
    if _OPTIONS["ownlibs"] then
        include_dirs = 
        {
            engine_path .. "/script_api",
            engine_path .. "/game_templates/vendor/magic_enum/include",
            "vendor/imgui",
            "vendor/implot",
            "vendor/SFML-3.0.0/include",
            "vendor/json/include",
            "%{prj.name}/src",
            "%{prj.name}/src/assets",
        }
        lib_dirs = 
        {
            engine_path,
            "vendor/SFML-3.0.0/lib",
        }
        sources = 
        {
            "%{prj.name}/src/**.cpp",
            "%{prj.name}/src/**.h",
            "vendor/imgui/**.cpp",
            "vendor/sfml-imgui/imgui-SFML.cpp",
            "vendor/implot/**.cpp",
        }
        remove_sources = 
        {
            "vendor/imgui/examples/**",
            "vendor/imgui/misc/**",
            "vendor/imgui/backends/**",
            "vendor/implot/examples/**",
        }
    else 
        include_dirs = 
        {
            engine_path .. "/script_api",
            engine_path .. "/game_templates/vendor",
            engine_path .. "/game_templates/vendor/magic_enum/include",
            engine_path .. "/game_templates/vendor/imgui",
            engine_path .. "/game_templates/vendor/imploot",
            engine_path .. "/game_templates/vendor/SFML-3.0.0/include",
            engine_path .. "/game_templates/vendor/json/include",
            "%{prj.name}/src",
            "%{prj.name}/src/assets",
        }
        lib_dirs = 
        {
            engine_path,
            engine_path .. "/game_templates/vendor/SFML-3.0.0/lib",
        }
        sources = 
        {
            "%{prj.name}/src/**.cpp",
            "%{prj.name}/src/**.h",
            engine_path .. "/game_templates/vendor/imgui/**.cpp",
            engine_path .. "/game_templates/vendor/sfml-imgui/imgui-SFML.cpp",
            engine_path .. "/game_templates/vendor/implot/**.cpp",
        }
        remove_sources = 
        {
            engine_path .. "/game_templates/imgui/examples/**",
            engine_path .. "/game_templates/imgui/misc/**",
            engine_path .. "/game_templates/imgui/backends/**",
            engine_path .. "/game_templates/implot/examples/**",
        }
    end 
end

workspace(_OPTIONS["projectname"])
    architecture "x64"
    configurations{
        "Release",
        "Debug", 
        "Dev",          -- same as release but defines TOAD_EDITOR
        "DevDebug",     -- with debugging symbols and debugging runtime library
        "Test",         -- optimizations and debugging info, also enables UI
    }
    
    startproject (game_project_name)

project(game_project_name)
    location(game_project_name)
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"

    flags {"MultiProcessorCompile"}

    targetdir("bin/" ..output_dir .. "/")
    objdir("bin-intermediate/" ..output_dir .. "/")

    files {
        sources
    }

    removefiles {
        remove_sources
    }
    
    includedirs{
        include_dirs
    }
    libdirs{
        lib_dirs
    }

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

    filter "configurations:Test"
        defines {
            "_DEBUG",
            "_USRDLL",
            "GAME_IS_EXPORT"
        }

        runtime "Debug"
        symbols "On"
        optimize "On"

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
            "vorbis",
            "vorbisenc",
            "vorbisfile",
            "sfml-system-d",
            "sfml-window-d",
            "sfml-graphics-d",
            "sfml-audio-d",
        }
    filter {"system:windows", "configurations:Release or configurations:Dev or configurations:Test"}
        defines {
            "SFML_STATIC",
        }
        links {
            "winmm",
            "gdi32",
            "freetype",
            "flac",
            "ogg",
            "vorbis",
            "vorbisenc",
            "vorbisfile",
            "sfml-system",
            "sfml-window",
            "sfml-graphics",
            "sfml-audio",
        }
