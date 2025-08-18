add_executable("ToadRunner"
      "vendor/imgui/imgui.cpp"
      "vendor/imgui/imgui_demo.cpp"
      "vendor/imgui/imgui_draw.cpp"
      "vendor/imgui/imgui_tables.cpp"
      "vendor/imgui/imgui_widgets.cpp"
      "vendor/pugixml/pugixml.cpp"
      "vendor/sfml-imgui/imgui-SFML.cpp"
    "ToadRunner/src/entry.cpp"
    "ToadRunner/src/pch.cpp"
    "ToadRunner/src/pch.h"
      "ToadRunner/src/project/Misc.cpp"
      "ToadRunner/src/project/Misc.h"
      "ToadRunner/src/project/NewScriptClass.h"
      "ToadRunner/src/project/ToadProject.cpp"
      "ToadRunner/src/project/ToadProject.h"
      "ToadRunner/src/ui/AnimationEditor.cpp"
      "ToadRunner/src/ui/AnimationEditor.h"
      "ToadRunner/src/ui/FSMGraphEditor.cpp"
      "ToadRunner/src/ui/FSMGraphEditor.h"
      "ToadRunner/src/ui/FileBrowser.cpp"
      "ToadRunner/src/ui/FileBrowser.h"
      "ToadRunner/src/ui/FlowEditor.cpp"
      "ToadRunner/src/ui/FlowEditor.h"
      "ToadRunner/src/ui/GameAssetsBrowser.cpp"
      "ToadRunner/src/ui/GameAssetsBrowser.h"
      "ToadRunner/src/ui/Inspector.cpp"
      "ToadRunner/src/ui/Inspector.h"
      "ToadRunner/src/ui/LogWindow.cpp"
      "ToadRunner/src/ui/LogWindow.h"
      "ToadRunner/src/ui/MessageQueue.cpp"
      "ToadRunner/src/ui/MessageQueue.h"
      "ToadRunner/src/ui/SceneHierarchy.cpp"
      "ToadRunner/src/ui/SceneHierarchy.h"
      "ToadRunner/src/ui/SceneHistory.cpp"
      "ToadRunner/src/ui/SceneHistory.h"
      "ToadRunner/src/ui/TextEditor.cpp"
      "ToadRunner/src/ui/TextEditor.h"
      "ToadRunner/src/ui/UI.cpp"
      "ToadRunner/src/ui/UI.h"
      "ToadRunner/src/ui/UIHelpers.cpp"
      "ToadRunner/src/ui/UIHelpers.h"
      "ToadRunner/src/ui/Viewport.cpp"
      "ToadRunner/src/ui/Viewport.h"
      "ToadRunner/src/utils/FileDialog.cpp"
      "ToadRunner/src/utils/FileDialog.h"
      "ToadRunner/src/utils/FileDialog.mm"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_dependencies("ToadRunner"
    "Engine"
  )
  set_target_properties("ToadRunner" PROPERTIES
    OUTPUT_NAME "ToadRunner"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Debug-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Debug-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Debug-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src/scripts>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../Engine/src>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/imgui>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/sfml-imgui>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Debug>:_CONSOLE>
  $<$<CONFIG:Debug>:TOAD_EDITOR>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:Debug>:Engine>
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
target_compile_options("ToadRunner" PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("ToadRunner" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_dependencies("ToadRunner"
    "Engine"
  )
  set_target_properties("ToadRunner" PROPERTIES
    OUTPUT_NAME "ToadRunner"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Release-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Release-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Release-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src/scripts>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../Engine/src>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/imgui>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/sfml-imgui>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:Release>:NDEBUG>
  $<$<CONFIG:Release>:_CONSOLE>
  $<$<CONFIG:Release>:TOAD_EDITOR>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:Release>:Engine>
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
target_compile_options("ToadRunner" PRIVATE
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("ToadRunner" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL ReleaseNoEditor)
  add_dependencies("ToadRunner"
    "Engine"
  )
  set_target_properties("ToadRunner" PROPERTIES
    OUTPUT_NAME "ToadRunner"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/ReleaseNoEditor-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/ReleaseNoEditor-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/ReleaseNoEditor-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src/scripts>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../Engine/src>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/imgui>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/sfml-imgui>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:NDEBUG>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:ReleaseNoEditor>:Engine>
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
target_compile_options("ToadRunner" PRIVATE
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL ReleaseNoEditor)
  set_target_properties("ToadRunner" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL DebugNoEditor)
  add_dependencies("ToadRunner"
    "Engine"
  )
  set_target_properties("ToadRunner" PROPERTIES
    OUTPUT_NAME "ToadRunner"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/DebugNoEditor-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/DebugNoEditor-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/DebugNoEditor-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src/scripts>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../Engine/src>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/imgui>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/sfml-imgui>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:DebugNoEditor>:_DEBUG>
  $<$<CONFIG:DebugNoEditor>:_CONSOLE>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:DebugNoEditor>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:DebugNoEditor>:Engine>
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
target_compile_options("ToadRunner" PRIVATE
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-O0>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-O0>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL DebugNoEditor)
  set_target_properties("ToadRunner" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Distro)
  add_dependencies("ToadRunner"
    "Engine"
  )
  set_target_properties("ToadRunner" PROPERTIES
    OUTPUT_NAME "ToadRunner"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Distro-macosx-x86_64
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Distro-macosx-x86_64
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../bin/Distro-macosx-x86_64
  )
endif()
if(APPLE)
find_library(OpenGL_FRAMEWORK OpenGL)
find_library(Cocoa_FRAMEWORK Cocoa)
find_library(IOKit_FRAMEWORK IOKit)
find_library(CoreVideo_FRAMEWORK CoreVideo)
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/src/scripts>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../Engine/src>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/imgui>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/sfml-imgui>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/include>
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:Distro>:NDEBUG>
  $<$<CONFIG:Distro>:_CONSOLE>
  $<$<CONFIG:Distro>:TOAD_EDITOR>
  $<$<CONFIG:Distro>:TOAD_DISTRO>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Distro>:${CMAKE_CURRENT_SOURCE_DIR}/ToadRunner/../vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:Distro>:Engine>
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
target_compile_options("ToadRunner" PRIVATE
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Distro)
  set_target_properties("ToadRunner" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()