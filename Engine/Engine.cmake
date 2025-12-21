add_library("Engine" SHARED
      "vendor/imgui/imgui.cpp"
      "vendor/imgui/imgui_demo.cpp"
      "vendor/imgui/imgui_draw.cpp"
      "vendor/imgui/imgui_tables.cpp"
      "vendor/imgui/imgui_widgets.cpp"
        "vendor/implot/.github/example_implot.cpp"
      "vendor/implot/implot.cpp"
      "vendor/implot/implot_demo.cpp"
      "vendor/implot/implot_items.cpp"
      "vendor/sfml-imgui/imgui-SFML.cpp"
    "Engine/src/EngineCore.h"
    "Engine/src/EngineMeta.h"
      "Engine/src/engine/AppWindow.cpp"
      "Engine/src/engine/AppWindow.h"
      "Engine/src/engine/DrawHelpers.cpp"
      "Engine/src/engine/DrawHelpers.h"
      "Engine/src/engine/DrawingCanvas.cpp"
      "Engine/src/engine/DrawingCanvas.h"
      "Engine/src/engine/Engine.cpp"
      "Engine/src/engine/Engine.h"
      "Engine/src/engine/FormatStr.h"
      "Engine/src/engine/Logger.cpp"
      "Engine/src/engine/Logger.h"
      "Engine/src/engine/PlaySession.cpp"
      "Engine/src/engine/PlaySession.h"
      "Engine/src/engine/PostFXShader.cpp"
      "Engine/src/engine/PostFXShader.h"
      "Engine/src/engine/Screen.cpp"
      "Engine/src/engine/Screen.h"
      "Engine/src/engine/Settings.h"
      "Engine/src/engine/Types.cpp"
      "Engine/src/engine/Types.h"
        "Engine/src/engine/default_objects/Audio.cpp"
        "Engine/src/engine/default_objects/Audio.h"
        "Engine/src/engine/default_objects/Camera.cpp"
        "Engine/src/engine/default_objects/Camera.h"
        "Engine/src/engine/default_objects/Circle.cpp"
        "Engine/src/engine/default_objects/Circle.h"
        "Engine/src/engine/default_objects/Object.cpp"
        "Engine/src/engine/default_objects/Object.h"
        "Engine/src/engine/default_objects/Sprite.cpp"
        "Engine/src/engine/default_objects/Sprite.h"
          "Engine/src/engine/default_objects/UI/Button.cpp"
          "Engine/src/engine/default_objects/UI/Button.h"
          "Engine/src/engine/default_objects/UI/Text.cpp"
          "Engine/src/engine/default_objects/UI/Text.h"
        "Engine/src/engine/default_scripts/AnimationController.cpp"
        "Engine/src/engine/default_scripts/AnimationController.h"
        "Engine/src/engine/default_scripts/Script.cpp"
        "Engine/src/engine/default_scripts/Script.h"
        "Engine/src/engine/systems/Animation.cpp"
        "Engine/src/engine/systems/Animation.h"
        "Engine/src/engine/systems/Flow.cpp"
        "Engine/src/engine/systems/Flow.h"
        "Engine/src/engine/systems/Input.cpp"
        "Engine/src/engine/systems/Input.h"
        "Engine/src/engine/systems/InputMac.cpp"
        "Engine/src/engine/systems/InputMac.h"
        "Engine/src/engine/systems/Reflection.cpp"
        "Engine/src/engine/systems/Reflection.h"
        "Engine/src/engine/systems/ResourceManager.cpp"
        "Engine/src/engine/systems/ResourceManager.h"
        "Engine/src/engine/systems/Scene.cpp"
        "Engine/src/engine/systems/Scene.h"
        "Engine/src/engine/systems/ScriptManager.cpp"
        "Engine/src/engine/systems/ScriptManager.h"
        "Engine/src/engine/systems/Tags.cpp"
        "Engine/src/engine/systems/Tags.h"
        "Engine/src/engine/systems/Time.cpp"
        "Engine/src/engine/systems/Time.h"
        "Engine/src/engine/systems/Timer.cpp"
        "Engine/src/engine/systems/Timer.h"
        "Engine/src/engine/utils/BezierCurve.cpp"
        "Engine/src/engine/utils/BezierCurve.h"
        "Engine/src/engine/utils/DLib.cpp"
        "Engine/src/engine/utils/DLib.h"
        "Engine/src/engine/utils/FSM.cpp"
        "Engine/src/engine/utils/FSM.h"
        "Engine/src/engine/utils/Helpers.cpp"
        "Engine/src/engine/utils/Helpers.h"
        "Engine/src/engine/utils/Wrappers.cpp"
        "Engine/src/engine/utils/Wrappers.h"
    "Engine/src/pch.cpp"
    "Engine/src/pch.h"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("Engine" PROPERTIES
    OUTPUT_NAME "Engine"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Debug-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Debug-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Debug-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("Engine" PRIVATE
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/src>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../GameTemplates/Game/src>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/imgui>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/implot>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/sfml-imgui>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/json/include>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/magic_enum/include>
)
target_compile_definitions("Engine" PRIVATE
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Debug>:_CONSOLE>
  $<$<CONFIG:Debug>:ENGINE_IS_EXPORT>
  $<$<CONFIG:Debug>:TOAD_EDITOR>
)
target_link_directories("Engine" PRIVATE
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("Engine"
  $<$<CONFIG:Debug>:${OpenGL_FRAMEWORK}>
  $<$<CONFIG:Debug>:${Cocoa_FRAMEWORK}>
  $<$<CONFIG:Debug>:${IOKit_FRAMEWORK}>
  $<$<CONFIG:Debug>:${CoreVideo_FRAMEWORK}>
  $<$<CONFIG:Debug>:sfml-system>
  $<$<CONFIG:Debug>:sfml-window>
  $<$<CONFIG:Debug>:sfml-graphics>
  $<$<CONFIG:Debug>:sfml-audio>
  $<$<CONFIG:Debug>:sfml-network>
)
target_compile_options("Engine" PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-fPIC>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-fPIC>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("Engine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("Engine" PROPERTIES
    OUTPUT_NAME "Engine"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Release-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Release-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Release-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("Engine" PRIVATE
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/src>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../GameTemplates/Game/src>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/imgui>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/implot>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/sfml-imgui>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/json/include>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/magic_enum/include>
)
target_compile_definitions("Engine" PRIVATE
  $<$<CONFIG:Release>:NDEBUG>
  $<$<CONFIG:Release>:ENGINE_IS_EXPORT>
  $<$<CONFIG:Release>:TOAD_EDITOR>
)
target_link_directories("Engine" PRIVATE
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("Engine"
  $<$<CONFIG:Release>:${OpenGL_FRAMEWORK}>
  $<$<CONFIG:Release>:${Cocoa_FRAMEWORK}>
  $<$<CONFIG:Release>:${IOKit_FRAMEWORK}>
  $<$<CONFIG:Release>:${CoreVideo_FRAMEWORK}>
  $<$<CONFIG:Release>:sfml-system>
  $<$<CONFIG:Release>:sfml-window>
  $<$<CONFIG:Release>:sfml-graphics>
  $<$<CONFIG:Release>:sfml-audio>
  $<$<CONFIG:Release>:sfml-network>
)
target_compile_options("Engine" PRIVATE
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-fPIC>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-fPIC>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("Engine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL ReleaseNoEditor)
  set_target_properties("Engine" PROPERTIES
    OUTPUT_NAME "Engine"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/ReleaseNoEditor-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/ReleaseNoEditor-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/ReleaseNoEditor-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("Engine" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/src>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../GameTemplates/Game/src>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/imgui>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/implot>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/sfml-imgui>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/json/include>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/magic_enum/include>
)
target_compile_definitions("Engine" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:NDEBUG>
  $<$<CONFIG:ReleaseNoEditor>:ENGINE_IS_EXPORT>
)
target_link_directories("Engine" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("Engine"
  $<$<CONFIG:ReleaseNoEditor>:${OpenGL_FRAMEWORK}>
  $<$<CONFIG:ReleaseNoEditor>:${Cocoa_FRAMEWORK}>
  $<$<CONFIG:ReleaseNoEditor>:${IOKit_FRAMEWORK}>
  $<$<CONFIG:ReleaseNoEditor>:${CoreVideo_FRAMEWORK}>
  $<$<CONFIG:ReleaseNoEditor>:sfml-system>
  $<$<CONFIG:ReleaseNoEditor>:sfml-window>
  $<$<CONFIG:ReleaseNoEditor>:sfml-graphics>
  $<$<CONFIG:ReleaseNoEditor>:sfml-audio>
  $<$<CONFIG:ReleaseNoEditor>:sfml-network>
)
target_compile_options("Engine" PRIVATE
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:C>>:-fPIC>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-fPIC>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL ReleaseNoEditor)
  set_target_properties("Engine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL DebugNoEditor)
  set_target_properties("Engine" PROPERTIES
    OUTPUT_NAME "Engine"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/DebugNoEditor-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/DebugNoEditor-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/DebugNoEditor-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("Engine" PRIVATE
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/src>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../GameTemplates/Game/src>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/imgui>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/implot>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/sfml-imgui>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/json/include>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/magic_enum/include>
)
target_compile_definitions("Engine" PRIVATE
  $<$<CONFIG:DebugNoEditor>:_DEBUG>
  $<$<CONFIG:DebugNoEditor>:_CONSOLE>
  $<$<CONFIG:DebugNoEditor>:ENGINE_IS_EXPORT>
)
target_link_directories("Engine" PRIVATE
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("Engine"
  $<$<CONFIG:DebugNoEditor>:${OpenGL_FRAMEWORK}>
  $<$<CONFIG:DebugNoEditor>:${Cocoa_FRAMEWORK}>
  $<$<CONFIG:DebugNoEditor>:${IOKit_FRAMEWORK}>
  $<$<CONFIG:DebugNoEditor>:${CoreVideo_FRAMEWORK}>
  $<$<CONFIG:DebugNoEditor>:sfml-system>
  $<$<CONFIG:DebugNoEditor>:sfml-window>
  $<$<CONFIG:DebugNoEditor>:sfml-graphics>
  $<$<CONFIG:DebugNoEditor>:sfml-audio>
  $<$<CONFIG:DebugNoEditor>:sfml-network>
)
target_compile_options("Engine" PRIVATE
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-fPIC>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-fPIC>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL DebugNoEditor)
  set_target_properties("Engine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Distro)
  set_target_properties("Engine" PROPERTIES
    OUTPUT_NAME "Engine"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Distro-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Distro-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Engine/../bin/Distro-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("Engine" PRIVATE
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/src>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../GameTemplates/Game/src>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/imgui>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/implot>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/sfml-imgui>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/json/include>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/magic_enum/include>
)
target_compile_definitions("Engine" PRIVATE
  $<$<CONFIG:Distro>:NDEBUG>
  $<$<CONFIG:Distro>:ENGINE_IS_EXPORT>
  $<$<CONFIG:Distro>:TOAD_EDITOR>
  $<$<CONFIG:Distro>:TOAD_DISTRO>
)
target_link_directories("Engine" PRIVATE
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/Engine/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("Engine"
  $<$<CONFIG:Distro>:${OpenGL_FRAMEWORK}>
  $<$<CONFIG:Distro>:${Cocoa_FRAMEWORK}>
  $<$<CONFIG:Distro>:${IOKit_FRAMEWORK}>
  $<$<CONFIG:Distro>:${CoreVideo_FRAMEWORK}>
  $<$<CONFIG:Distro>:sfml-system>
  $<$<CONFIG:Distro>:sfml-window>
  $<$<CONFIG:Distro>:sfml-graphics>
  $<$<CONFIG:Distro>:sfml-audio>
  $<$<CONFIG:Distro>:sfml-network>
)
target_compile_options("Engine" PRIVATE
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:C>>:-fPIC>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-fPIC>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Distro)
  set_target_properties("Engine" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE True
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()