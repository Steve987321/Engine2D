project "RayCaster"
    kind "None" -- SharedLib, but None to skip build
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. output_dir .. "/")
    objdir ("bin-intermediate/" .. output_dir .. "/")

    files {
        "src/**.cpp",
        "src/**.h",
        "../../vendor/imgui/**.cpp",
        "../../vendor/sfml-imgui/imgui-SFML.cpp",
    }

    removefiles {
        "../../vendor/imgui/examples/**",
        "../../vendor/imgui/misc/**",
        "../../vendor/imgui/backends/**",
    }

    if _ACTION == "xcode4" then
    externalincludedirs {
        "../../Engine/src",
        "../../vendor",
        "../../vendor/imgui",
        "../../vendor/SFML-2.6.0/include",
        "../../vendor/json/include", 
        "src",
        "src/assets"
    }
    else 
    includedirs {
        "../../Engine/src",
        "../../vendor",
        "../../vendor/imgui",
        "../../vendor/SFML-2.6.0/include",
        "../../vendor/json/include", 
        "src",
        "src/assets"
    }
    end 
    libdirs {
        "../../vendor/SFML-2.6.0/lib"
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
            "opengl32",
            "ws2_32",
        }

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

    -- configurations

    filter "configurations:Debug" 
        defines {
            "_DEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
            "_USRDLL"
        }

        runtime "Debug"
        symbols "On"

    filter "configurations:Release" 
        defines {
            "NDEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
            "_USRDLL"
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Distro" 
        defines {
            "NDEBUG",
            "_WINDOWS",
            "GAME_IS_EXPORT",
            "TOAD_EDITOR",
            "TOAD_DISTRO",
            "_USRDLL"
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:ReleaseNoEditor"
        defines {
            "NDEBUG",
            "_WINDOWS",
            "_USRDLL",
            "GAME_IS_EXPORT"
        }

        runtime "Release"
        symbols "Off"
        optimize "On"

    filter "configurations:DebugNoEditor"
        defines {
                "_DEBUG",
                "_WINDOWS",
                "GAME_IS_EXPORT",
                "_USRDLL"
        }

        runtime "Debug"
        symbols "On"

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
