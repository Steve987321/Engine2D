'''
Create a distribution of the engine. 
This now only works on Windows. 
'''

import subprocess
import shutil
import os 
import sys 

script_dir = os.path.dirname(os.path.abspath(__file__))
dir_out = os.path.join(script_dir, "output") 
proj_dir = os.path.dirname(script_dir)
copy_count = 0

# everything except script_api
files = [
    (("bin", "Distro-windows-x86_64", "Engine.dll"), (""), ""),
    (("bin", "Distro-windows-x86_64", "ToadRunner.exe"), (""), ""),
    (("bin", "Distro-windows-x86_64", "Engine.lib"), ("libs",), ""),
    (("bin", "ReleaseNoEditor-windows-x86_64", "Engine.dll"), ("bin",), ""),
    (("bin", "ReleaseNoEditor-windows-x86_64", "ToadRunner.exe"), ("bin",), "ToadRunnerNoEditor.exe"),
    (("bin", "DebugNoEditor-windows-x86_64", "ToadRunner.exe"), ("bin", "debug"), "ToadRunnerNoEditorDebug.exe"),
    (("bin", "DebugNoEditor-windows-x86_64", "Engine.dll"), ("bin", "debug"), ""),
    (("ToadRunner", "src", "project", "generate_game_project.lua"), ("scripts",), ""),
    (("ToadRunner", "imgui.ini"), ("",), ""),
    (("Game", "src", ), ("game_templates", "src"), ""),
    (("vendor", "bin", "premake5.exe"), ("bin",), ""),
    (("vendor", "bin", "LICENSE.txt"), ("bin",), ""),
    (("vendor", "imgui"), ("game_templates", "vendor", "imgui"), ""),
    (("vendor", "json"), ("game_templates", "vendor", "json"), ""),
    (("vendor", "SFML-2.6.0"), ("game_templates", "vendor", "SFML-2.6.0"), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-audio-2.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-system-2.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-graphics-2.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-network-2.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-window-2.dll"), (""), ""), 
    (("vendor", "SFML-2.6.0", "bin", "sfml-audio-d-2.dll"), ("bin", "debug"), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-system-d-2.dll"), ("bin", "debug"), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-graphics-d-2.dll"), ("bin", "debug"), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-network-d-2.dll"), ("bin", "debug"), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-window-d-2.dll"), ("bin", "debug"), ""),
    (("vendor", "sfml-imgui"), ("game_templates", "vendor", "sfml-imgui"), ""),
]

# files and folders to ignore 
ignore = [
    "examples",
    "cmake",
    "docs",
    "doc",
]

# not needed but should be copied if possible
optional_files = [
    "imgui.ini",
]

is_valid = True 
for relative, _, _ in files: 
    path = os.path.join(proj_dir, *relative)
    if not os.path.exists(path):
        for optional_file in optional_files: 
            if optional_file not in relative: 
                print(f"Error: Path doesn't exist: {path}")
                is_valid = False 

if not is_valid: 
    sys.exit(1)

os.makedirs(os.path.join(dir_out, "bin"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "bin/debug"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "game_templates"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "game_templates/src"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "game_templates/vendor"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "libs"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "script_api"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "scripts"), exist_ok=True)
os.path.join(proj_dir, "bin", "Release-windows-x86_64")

for relative, relative_dest, rename in files: 
    path_src = os.path.join(proj_dir, *relative)
    path_dst = os.path.join(dir_out, *relative_dest)

    skip = False 

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
        shutil.copytree(path_src, os.path.join(path_dst, rename), dirs_exist_ok=True, ignore=shutil.ignore_patterns(*ignore))
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

print(f"Performed {copy_count} copies")