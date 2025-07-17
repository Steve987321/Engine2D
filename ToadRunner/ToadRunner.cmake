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
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Debug-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Debug-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Debug-macosx-x86_64"
  )
endif()
if(APPLE)
  find_library(COCOA_FRAMEWORK Cocoa)
  find_library(IOKIT_FRAMEWORK IOKit)
  find_library(COREVIDEO_FRAMEWORK CoreVideo)
  find_library(OPENGL_FRAMEWORK OpenGL)
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src/scripts>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/sfml-imgui>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/include>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Debug>:_CONSOLE>
  $<$<CONFIG:Debug>:TOAD_EDITOR>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:Debug>:Engine>
  $<$<CONFIG:Debug>:${OPENGL_FRAMEWORK}>
  $<$<CONFIG:Debug>:${COCOA_FRAMEWORK}>
  $<$<CONFIG:Debug>:${IOKIT_FRAMEWORK}>
  $<$<CONFIG:Debug>:${COREVIDEO_FRAMEWORK}>
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
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Release-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Release-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Release-macosx-x86_64"
  )
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src/scripts>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/sfml-imgui>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/include>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:Release>:NDEBUG>
  $<$<CONFIG:Release>:_CONSOLE>
  $<$<CONFIG:Release>:TOAD_EDITOR>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:Release>:Engine>
  $<$<CONFIG:Release>:${OPENGL_FRAMEWORK}>
  $<$<CONFIG:Release>:${COCOA_FRAMEWORK}>
  $<$<CONFIG:Release>:${IOKIT_FRAMEWORK}>
  $<$<CONFIG:Release>:${COREVIDEO_FRAMEWORK}>
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
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/ReleaseNoEditor-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/ReleaseNoEditor-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/ReleaseNoEditor-macosx-x86_64"
  )
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src/scripts>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/sfml-imgui>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/include>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:NDEBUG>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:ReleaseNoEditor>:Engine>
  $<$<CONFIG:ReleaseNoEditor>:${OPENGL_FRAMEWORK}>
  $<$<CONFIG:ReleaseNoEditor>:${COCOA_FRAMEWORK}>
  $<$<CONFIG:ReleaseNoEditor>:${IOKIT_FRAMEWORK}>
  $<$<CONFIG:ReleaseNoEditor>:${COREVIDEO_FRAMEWORK}>
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
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/DebugNoEditor-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/DebugNoEditor-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/DebugNoEditor-macosx-x86_64"
  )
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src/scripts>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/sfml-imgui>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/include>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:DebugNoEditor>:_DEBUG>
  $<$<CONFIG:DebugNoEditor>:_CONSOLE>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:DebugNoEditor>:Engine>
  $<$<CONFIG:DebugNoEditor>:${OPENGL_FRAMEWORK}>
  $<$<CONFIG:DebugNoEditor>:${COCOA_FRAMEWORK}>
  $<$<CONFIG:DebugNoEditor>:${IOKIT_FRAMEWORK}>
  $<$<CONFIG:DebugNoEditor>:${COREVIDEO_FRAMEWORK}>
  $<$<CONFIG:DebugNoEditor>:sfml-system>
  $<$<CONFIG:DebugNoEditor>:sfml-window>
  $<$<CONFIG:DebugNoEditor>:sfml-graphics>
  $<$<CONFIG:DebugNoEditor>:sfml-audio>
  $<$<CONFIG:DebugNoEditor>:sfml-network>
)
target_compile_options("ToadRunner" PRIVATE
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-m64>
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
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Distro-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Distro-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/bin/Distro-macosx-x86_64"
  )
endif()
target_include_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/ToadRunner/src/scripts>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/sfml-imgui>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/include>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
)
target_compile_definitions("ToadRunner" PRIVATE
  $<$<CONFIG:Distro>:NDEBUG>
  $<$<CONFIG:Distro>:_CONSOLE>
  $<$<CONFIG:Distro>:TOAD_EDITOR>
  $<$<CONFIG:Distro>:TOAD_DISTRO>
)
target_link_directories("ToadRunner" PRIVATE
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-3.0.0/lib>
)
target_link_libraries("ToadRunner"
  $<$<CONFIG:Distro>:Engine>
  $<$<CONFIG:Distro>:${OPENGL_FRAMEWORK}>
  $<$<CONFIG:Distro>:${COCOA_FRAMEWORK}>
  $<$<CONFIG:Distro>:${IOKIT_FRAMEWORK}>
  $<$<CONFIG:Distro>:${COREVIDEO_FRAMEWORK}>
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