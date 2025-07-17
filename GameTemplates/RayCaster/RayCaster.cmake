add_executable("RayCaster"
      "vendor/imgui/imgui.cpp"
      "vendor/imgui/imgui_demo.cpp"
      "vendor/imgui/imgui_draw.cpp"
      "vendor/imgui/imgui_tables.cpp"
      "vendor/imgui/imgui_widgets.cpp"
      "vendor/sfml-imgui/imgui-SFML.cpp"
        "GameTemplates/RayCaster/src/assets/framework/Framework.cpp"
        "GameTemplates/RayCaster/src/assets/framework/Framework.h"
        "GameTemplates/RayCaster/src/assets/helpers/Helpers.cpp"
        "GameTemplates/RayCaster/src/assets/helpers/Helpers.h"
        "GameTemplates/RayCaster/src/assets/scripts/Map.cpp"
        "GameTemplates/RayCaster/src/assets/scripts/Map.h"
        "GameTemplates/RayCaster/src/assets/scripts/MapObject.cpp"
        "GameTemplates/RayCaster/src/assets/scripts/MapObject.h"
        "GameTemplates/RayCaster/src/assets/scripts/Player.cpp"
        "GameTemplates/RayCaster/src/assets/scripts/Player.h"
        "GameTemplates/RayCaster/src/assets/scripts/RayCaster.cpp"
        "GameTemplates/RayCaster/src/assets/scripts/RayCaster.h"
      "GameTemplates/RayCaster/src/game_core/Game.cpp"
      "GameTemplates/RayCaster/src/game_core/Game.h"
      "GameTemplates/RayCaster/src/game_core/ScriptRegister.cpp"
      "GameTemplates/RayCaster/src/game_core/ScriptRegister.h"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_dependencies("RayCaster"
    "Engine"
  )
  set_target_properties("RayCaster" PROPERTIES
    OUTPUT_NAME "RayCaster"
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Debug-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Debug-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Debug-macosx-x86_64"
  )
endif()
target_include_directories("RayCaster" PRIVATE
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/include>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src>
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src/assets>
)
target_compile_definitions("RayCaster" PRIVATE
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Debug>:_WINDOWS>
  $<$<CONFIG:Debug>:GAME_IS_EXPORT>
  $<$<CONFIG:Debug>:TOAD_EDITOR>
  $<$<CONFIG:Debug>:_USRDLL>
)
target_link_directories("RayCaster" PRIVATE
  $<$<CONFIG:Debug>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/lib>
)
target_link_libraries("RayCaster"
  $<$<CONFIG:Debug>:Engine>
  $<$<CONFIG:Debug>:OpenGL.framework>
  $<$<CONFIG:Debug>:Cocoa.framework>
  $<$<CONFIG:Debug>:IOKit.framework>
  $<$<CONFIG:Debug>:CoreVideo.framework>
  $<$<CONFIG:Debug>:sfml-system>
  $<$<CONFIG:Debug>:sfml-window>
  $<$<CONFIG:Debug>:sfml-graphics>
  $<$<CONFIG:Debug>:sfml-audio>
  $<$<CONFIG:Debug>:sfml-network>
)
target_compile_options("RayCaster" PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("RayCaster" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_dependencies("RayCaster"
    "Engine"
  )
  set_target_properties("RayCaster" PROPERTIES
    OUTPUT_NAME "RayCaster"
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Release-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Release-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Release-macosx-x86_64"
  )
endif()
target_include_directories("RayCaster" PRIVATE
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/include>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src>
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src/assets>
)
target_compile_definitions("RayCaster" PRIVATE
  $<$<CONFIG:Release>:NDEBUG>
  $<$<CONFIG:Release>:_WINDOWS>
  $<$<CONFIG:Release>:GAME_IS_EXPORT>
  $<$<CONFIG:Release>:TOAD_EDITOR>
  $<$<CONFIG:Release>:_USRDLL>
)
target_link_directories("RayCaster" PRIVATE
  $<$<CONFIG:Release>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/lib>
)
target_link_libraries("RayCaster"
  $<$<CONFIG:Release>:Engine>
  $<$<CONFIG:Release>:OpenGL.framework>
  $<$<CONFIG:Release>:Cocoa.framework>
  $<$<CONFIG:Release>:IOKit.framework>
  $<$<CONFIG:Release>:CoreVideo.framework>
  $<$<CONFIG:Release>:sfml-system>
  $<$<CONFIG:Release>:sfml-window>
  $<$<CONFIG:Release>:sfml-graphics>
  $<$<CONFIG:Release>:sfml-audio>
  $<$<CONFIG:Release>:sfml-network>
)
target_compile_options("RayCaster" PRIVATE
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("RayCaster" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL ReleaseNoEditor)
  add_dependencies("RayCaster"
    "Engine"
  )
  set_target_properties("RayCaster" PROPERTIES
    OUTPUT_NAME "RayCaster"
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/ReleaseNoEditor-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/ReleaseNoEditor-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/ReleaseNoEditor-macosx-x86_64"
  )
endif()
target_include_directories("RayCaster" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/include>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src>
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src/assets>
)
target_compile_definitions("RayCaster" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:NDEBUG>
  $<$<CONFIG:ReleaseNoEditor>:_WINDOWS>
  $<$<CONFIG:ReleaseNoEditor>:_USRDLL>
  $<$<CONFIG:ReleaseNoEditor>:GAME_IS_EXPORT>
)
target_link_directories("RayCaster" PRIVATE
  $<$<CONFIG:ReleaseNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/lib>
)
target_link_libraries("RayCaster"
  $<$<CONFIG:ReleaseNoEditor>:Engine>
  $<$<CONFIG:ReleaseNoEditor>:OpenGL.framework>
  $<$<CONFIG:ReleaseNoEditor>:Cocoa.framework>
  $<$<CONFIG:ReleaseNoEditor>:IOKit.framework>
  $<$<CONFIG:ReleaseNoEditor>:CoreVideo.framework>
  $<$<CONFIG:ReleaseNoEditor>:sfml-system>
  $<$<CONFIG:ReleaseNoEditor>:sfml-window>
  $<$<CONFIG:ReleaseNoEditor>:sfml-graphics>
  $<$<CONFIG:ReleaseNoEditor>:sfml-audio>
  $<$<CONFIG:ReleaseNoEditor>:sfml-network>
)
target_compile_options("RayCaster" PRIVATE
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:ReleaseNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL ReleaseNoEditor)
  set_target_properties("RayCaster" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL DebugNoEditor)
  add_dependencies("RayCaster"
    "Engine"
  )
  set_target_properties("RayCaster" PROPERTIES
    OUTPUT_NAME "RayCaster"
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/DebugNoEditor-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/DebugNoEditor-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/DebugNoEditor-macosx-x86_64"
  )
endif()
target_include_directories("RayCaster" PRIVATE
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/include>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src>
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src/assets>
)
target_compile_definitions("RayCaster" PRIVATE
  $<$<CONFIG:DebugNoEditor>:_DEBUG>
  $<$<CONFIG:DebugNoEditor>:_WINDOWS>
  $<$<CONFIG:DebugNoEditor>:GAME_IS_EXPORT>
  $<$<CONFIG:DebugNoEditor>:_USRDLL>
)
target_link_directories("RayCaster" PRIVATE
  $<$<CONFIG:DebugNoEditor>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/lib>
)
target_link_libraries("RayCaster"
  $<$<CONFIG:DebugNoEditor>:Engine>
  $<$<CONFIG:DebugNoEditor>:OpenGL.framework>
  $<$<CONFIG:DebugNoEditor>:Cocoa.framework>
  $<$<CONFIG:DebugNoEditor>:IOKit.framework>
  $<$<CONFIG:DebugNoEditor>:CoreVideo.framework>
  $<$<CONFIG:DebugNoEditor>:sfml-system>
  $<$<CONFIG:DebugNoEditor>:sfml-window>
  $<$<CONFIG:DebugNoEditor>:sfml-graphics>
  $<$<CONFIG:DebugNoEditor>:sfml-audio>
  $<$<CONFIG:DebugNoEditor>:sfml-network>
)
target_compile_options("RayCaster" PRIVATE
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:DebugNoEditor>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL DebugNoEditor)
  set_target_properties("RayCaster" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Distro)
  add_dependencies("RayCaster"
    "Engine"
  )
  set_target_properties("RayCaster" PROPERTIES
    OUTPUT_NAME "RayCaster"
    ARCHIVE_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Distro-macosx-x86_64"
    LIBRARY_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Distro-macosx-x86_64"
    RUNTIME_OUTPUT_DIRECTORY "/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/bin/Distro-macosx-x86_64"
  )
endif()
target_include_directories("RayCaster" PRIVATE
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/Engine/src>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/imgui>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/include>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/json/include>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src>
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/GameTemplates/RayCaster/src/assets>
)
target_compile_definitions("RayCaster" PRIVATE
  $<$<CONFIG:Distro>:NDEBUG>
  $<$<CONFIG:Distro>:_WINDOWS>
  $<$<CONFIG:Distro>:GAME_IS_EXPORT>
  $<$<CONFIG:Distro>:TOAD_EDITOR>
  $<$<CONFIG:Distro>:TOAD_DISTRO>
  $<$<CONFIG:Distro>:_USRDLL>
)
target_link_directories("RayCaster" PRIVATE
  $<$<CONFIG:Distro>:/Users/stefan/Developer/Steve987321/Engine2D/vendor/SFML-2.6.0/lib>
)
target_link_libraries("RayCaster"
  $<$<CONFIG:Distro>:Engine>
  $<$<CONFIG:Distro>:OpenGL.framework>
  $<$<CONFIG:Distro>:Cocoa.framework>
  $<$<CONFIG:Distro>:IOKit.framework>
  $<$<CONFIG:Distro>:CoreVideo.framework>
  $<$<CONFIG:Distro>:sfml-system>
  $<$<CONFIG:Distro>:sfml-window>
  $<$<CONFIG:Distro>:sfml-graphics>
  $<$<CONFIG:Distro>:sfml-audio>
  $<$<CONFIG:Distro>:sfml-network>
)
target_compile_options("RayCaster" PRIVATE
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Distro>,$<COMPILE_LANGUAGE:CXX>>:-std=c++20>
)
if(CMAKE_BUILD_TYPE STREQUAL Distro)
  set_target_properties("RayCaster" PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()