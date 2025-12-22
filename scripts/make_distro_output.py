'''
Create a distribution of the engine. 
'''

import subprocess
import shutil
import os 
import sys 

if sys.platform == "darwin":
    sfml_bin_folder = "lib"
    sfml_version_suffix = ".3.0.0"
    platform_as_str = "macosx"
    exe_ext = ""
    dlib_prefix = "lib"
    dlib_ext = ".dylib"
elif sys.platform == "win32":
    sfml_bin_folder = "bin"
    sfml_version_suffix = "-3"
    platform_as_str = "windows"
    exe_ext = ".exe"
    dlib_prefix = ""
    dlib_ext = ".dll"
else:
    print(f"{sys.platform} is not supported")
    sys.exit(1)

script_dir = os.path.dirname(os.path.abspath(__file__))
dir_out = os.path.join(script_dir, "output") 
proj_dir = os.path.dirname(script_dir)
copy_count = 0

# what game templates to add to distro (all)
game_templates = [
    "Game",
    "PhysicsSandbox",
    "RayCaster",
]

# everything except script_api
files = [
    (("bin", f"Distro-{platform_as_str}-x86_64", f"{dlib_prefix}Engine{dlib_ext}"), (""), ""),
    (("bin", f"Distro-{platform_as_str}-x86_64", f"ToadRunner{exe_ext}"), (""), ""),
    (("bin", f"ReleaseNoEditor-{platform_as_str}-x86_64", f"ToadRunner{exe_ext}"), ("bin",), f"ToadRunnerNoEditor{exe_ext}"),
    (("bin", f"ReleaseNoEditor-{platform_as_str}-x86_64", f"{dlib_prefix}Engine{dlib_ext}"), ("bin",), ""),
    (("bin", f"DebugNoEditor-{platform_as_str}-x86_64", f"ToadRunner{exe_ext}"), ("bin", "debug"), f"ToadRunnerNoEditorDebug{exe_ext}"),
    (("bin", f"DebugNoEditor-{platform_as_str}-x86_64", f"{dlib_prefix}Engine{dlib_ext}"), ("bin", "debug"), f"{dlib_prefix}EngineDebug{dlib_ext}"),
    (("bin", f"TestNoEditor-{platform_as_str}-x86_64", f"ToadRunner{exe_ext}"), ("bin", "debug"), f"ToadRunnerNoEditorTest{exe_ext}"),
    (("bin", f"TestNoEditor-{platform_as_str}-x86_64", f"{dlib_prefix}Engine{dlib_ext}"), ("bin", "debug"), f"{dlib_prefix}EngineTest{dlib_ext}"),
    (("scripts", "generate_game_project.lua"), ("scripts",), ""),
    (("scripts", "cmake"), ("scripts", "cmake"), ""),
    (("", "imgui-main.ini"), ("",), ""),
    (("", "imgui-anim.ini"), ("",), ""),
    (("", "imgui.ini"), ("",), ""),
    (("vendor", "bin", f"premake5{exe_ext}"), ("bin",), ""),
    (("vendor", "bin", "LICENSE.txt"), ("bin",), ""),
    (("vendor", "json"), ("game_templates", "vendor", "json"), ""),
    (("vendor", "magic_enum"), ("game_templates", "vendor", "magic_enum"), ""),
    (("vendor", "implot"), ("game_templates", "vendor", "implot"), ""),
    (("vendor", "filewatch"), ("game_templates", "vendor", "filewatch"), ""),
    (("vendor", "SFML-3.0.0"), ("game_templates", "vendor", "SFML-3.0.0"), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-audio{sfml_version_suffix}{dlib_ext}"), (""), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-system{sfml_version_suffix}{dlib_ext}"), (""), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-graphics{sfml_version_suffix}{dlib_ext}"), (""), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-network{sfml_version_suffix}{dlib_ext}"), (""), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-window{sfml_version_suffix}{dlib_ext}"), (""), ""), 
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-audio-d{sfml_version_suffix}{dlib_ext}"), ("bin", "debug"), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-system-d{sfml_version_suffix}{dlib_ext}"), ("bin", "debug"), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-graphics-d{sfml_version_suffix}{dlib_ext}"), ("bin", "debug"), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-network-d{sfml_version_suffix}{dlib_ext}"), ("bin", "debug"), ""),
    (("vendor", "SFML-3.0.0", sfml_bin_folder, f"{dlib_prefix}sfml-window-d{sfml_version_suffix}{dlib_ext}"), ("bin", "debug"), ""),
    (("vendor", "sfml-imgui"), ("game_templates", "vendor", "sfml-imgui"), ""),
]

if sys.platform == "win32":
    files.append((("bin", f"Distro-{platform_as_str}-x86_64", f"Engine.lib"), (""), ""))

# folders to ignore 
ignore_folders = [
    ".github",
    "examples",
    "cmake",
    "docs",
    "doc",
    "misc",
    "vulkan",
    "sdlgpu3",
    "example",
]

# not needed but should be copied if possible
optional_files = [
    "imgui-main.ini",
    "imgui-anim.ini",
    "imgui.ini",
    "thumbnail.png",
]
print(sys.platform)
if sys.platform == "win32": 
    imgui_backend_files = [
        "imgui_impl_opengl2.cpp",
        "imgui_impl_opengl2.h",
        "imgui_impl_win32.cpp",
        "imgui_impl_win32.h",
    ]
elif sys.platform == "darwin":
    imgui_backend_files = [
        "imgui_impl_opengl2.cpp",
        "imgui_impl_opengl2.h",
        "imgui_impl_osx.cpp",
        "imgui_impl_osx.h",
    ]

# add imgui

os.makedirs(os.path.join(dir_out, "game_templates", "vendor", "imgui", "backends"), exist_ok=True)

for file in os.listdir(os.path.join(proj_dir, "vendor", "imgui", "backends")):
    if file in imgui_backend_files:
        relative_path = os.path.relpath(os.path.join(proj_dir, "vendor", "imgui", "backends", file), proj_dir)

        files.append((tuple(relative_path.split(os.sep)), ("game_templates", "vendor", "imgui", "backends"), ""))

for file in os.listdir(os.path.join(proj_dir, "vendor", "imgui")):
    file_path = os.path.join(proj_dir, "vendor", "imgui", file)

    if file.endswith(".cpp") or file.endswith(".h"):
        relative_path = os.path.relpath(file_path, proj_dir)
        files.append((tuple(relative_path.split(os.sep)), ("game_templates", "vendor", "imgui"), ""))

print(files)
# add game templates
for gt in game_templates:
    files.append((("GameTemplates", gt, "src"), ("game_templates", gt, "src"), ""))
    files.append((("GameTemplates", gt, "readme.txt"), ("game_templates", gt ), ""))
    files.append((("GameTemplates", gt, "premake5.lua"), ("game_templates", gt), ""))
    files.append((("GameTemplates", gt, "thumbnail.png"), ("game_templates", gt), ""))

def should_skip_macos(entry):
    file = os.path.basename(entry) 
    
    # no debug binaries of sfml on macos
    if "sfml" in file and "-d" in file:
        return True 

    if file.endswith(".lib"):
        return True

    # premake5 binary not given for macos system
    if file == "premake5":
        return True
    
    return False 

# sfml uses symlinks
def create_sfml_symlinks(dir):
    sfml_modules = [
        "audio",
        "graphics",
        "system",
        "window",
        "network"
    ]

    for m in sfml_modules: 
        real_name = os.path.join(dir, f"libsfml-{m}.3.0.0.dylib")

        symlinks = [
            os.path.join(dir, f"libsfml-{m}.3.0.dylib"),
            os.path.join(dir, f"libsfml-{m}.dylib")
        ]

        if not os.path.exists(real_name):
            print(f"Skipping missing SFML dylib: {real_name}")
            continue

        for sl in symlinks:
            if not os.path.exists(sl):
                os.symlink(real_name, sl)
            
    
is_valid = True 
for relative, _, _ in files: 
    path = os.path.join(proj_dir, *relative)
    if not os.path.exists(path):
        file = relative[-1]
        if file not in optional_files:
            if sys.platform == "darwin":
                if should_skip_macos(path):
                    continue

            print(f"Error: Path doesn't exist: {path}")
            is_valid = False

if not is_valid: 
    sys.exit(1)

os.makedirs(os.path.join(dir_out, "bin"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "bin/debug"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "game_templates"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "game_templates/vendor"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "script_api"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "scripts"), exist_ok=True)

for relative, relative_dest, rename in files: 

    path_src = os.path.join(proj_dir, *relative)
    path_dst = os.path.join(dir_out, *relative_dest)

    skip = False 

    if sys.platform == "darwin":
        if should_skip_macos(path_src):
            skip = True  

    # skip optional files if they don't exist
    if not os.path.exists(path_src):
        for optional_file in optional_files: 
            if optional_file in path_src: 
                print(f"Skipping optional file: {path_src}")
                skip = True 

    # skip unmodified files 
    if not skip: 
        if len(rename) > 0: 
            dst = os.path.join(path_dst, rename)
        else:
            dst = os.path.join(path_dst, relative[-1])
        if os.path.isdir(path_src):
            dst = path_dst

        if os.path.exists(dst): 
            last_modified_time_src = os.path.getmtime(path_src)
            last_modified_time_dst = os.path.getmtime(dst)
            if last_modified_time_src == last_modified_time_dst: 
                print(f"Skipping unmodified file: {path_src}")
                skip = True 

    if skip: 
        continue 

    print(f"Copy: {path_src} to {path_dst}")

    if os.path.isdir(path_src):       
         shutil.copytree(path_src, os.path.join(path_dst, rename), dirs_exist_ok=True, ignore=shutil.ignore_patterns(*ignore_folders))
    else: 
        shutil.copy2(path_src, os.path.join(path_dst, rename))

    copy_count += 1

# script_api
engine_proj_dir = os.path.join(proj_dir, "Engine", "src")
script_api_dir = os.path.join(dir_out, "script_api")

for root, _, files in os.walk(engine_proj_dir): 
    for file in files: 
        if file.endswith(".h"): 
            dest_dir = os.path.join(script_api_dir, os.path.relpath(root, engine_proj_dir))
            
            file_src = os.path.join(root, file)

            file_dst = os.path.join(dest_dir, file)
            if os.path.exists(file_dst):
                if os.path.getmtime(file_dst) == os.path.getmtime(file_src): 
                    print(f"Skipping unmodified file: {file_src}")
                    continue 

            os.makedirs(dest_dir, exist_ok=True)

            print(f"Copy: {file_src} to {dest_dir}")
            shutil.copy2(file_src, dest_dir)
            copy_count += 1

if sys.platform == "darwin":
    # fix libEngine.dylib not being found, use otool -L to show what toadrunner is being linked against + path
    os.system(f"cd {dir_out} && install_name_tool -change @rpath/libEngine.dylib @loader_path/libEngine.dylib ./ToadRunner")

    # for the sfml libs add symlinks for it to link correctly 
    create_sfml_symlinks(dir_out)

print(f"Performed {copy_count} copies")