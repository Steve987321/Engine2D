import subprocess
import shutil
import os 
import sys 

script_dir = os.path.dirname(os.path.abspath(__file__))
dir_out = os.path.join(script_dir, "output") 
proj_dir = os.path.dirname(script_dir)

# everything except script_api
files = [
    (("bin", "Distro-windows-x86_64", "Engine.dll"), (""), ""),
    (("bin", "Distro-windows-x86_64", "ToadRunner.exe"), (""), ""),
    (("bin", "Distro-windows-x86_64", "imgui.ini"), (""), ""),
    (("bin", "Release-windows-x86_64", "Engine.lib"), ("libs",), ""),
    (("bin", "Release-windows-x86_64", "ToadRunner.exe"), ("bin",), "ToadRunnerNoEditor.exe"),
    (("vendor", "SFML-2.6.0", "bin", "openal32.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-audio-2.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-graphics-2.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-network-2.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-system-2.dll"), (""), ""),
    (("vendor", "SFML-2.6.0", "bin", "sfml-window-2.dll"), (""), ""),
    (("ToadRunner", "src", "project", "generate_game_project.lua"), ("scripts",), ""),
    (("Game", "src", ), ("game_template", "src"), ""),
    (("vendor", "bin", "premake5.exe"), ("bin",), ""),
    (("vendor", "bin", "LICENSE.txt"), ("bin",), ""),
    (("vendor", "imgui"), ("game_template", "vendor"), ""),
    (("vendor", "json"), ("game_template", "vendor"), ""),
    (("vendor", "SFML-2.6.0"), ("game_template", "vendor"), ""),
    (("vendor", "sfml-imgui"), ("game_template", "vendor"), ""),
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
os.makedirs(os.path.join(dir_out, "game_template"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "game_template/src"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "game_template/vendor"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "libs"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "script_api"), exist_ok=True)
os.makedirs(os.path.join(dir_out, "scripts"), exist_ok=True)
os.path.join(proj_dir, "bin", "Release-windows-x86_64")

def Ignore(dir, files): 
    return [os.path.join(dir, ignore_file) for ignore_file in ignore]

for relative, relative_dest, rename in files: 
    path_src = os.path.join(proj_dir, *relative)
    path_dst = os.path.join(dir_out, *relative_dest)

    skip = False 
    if not os.path.exists(path_src):
        for optional_file in optional_files: 
            if optional_file in path_src: 
                print(f"Skipping optional file: {path_src}")
                skip = True 

    if skip: 
        continue 

    print(f"Copy: {path_src} to {path_dst}")

    if os.path.isdir(path_src): 
        shutil.copytree(path_src, os.path.join(path_dst, rename), dirs_exist_ok=True, ignore=Ignore)
    else: 
        shutil.copy2(path_src, os.path.join(path_dst, rename))

# script_api
engine_proj_dir = os.path.join(proj_dir, "Engine", "src")
script_api_dir = os.path.join(dir_out, "script_api")

for root, _, files in os.walk(engine_proj_dir): 
    for file in files: 
        if file.endswith(".h"): 
            dest_dir = os.path.join(script_api_dir, os.path.relpath(root, engine_proj_dir))
            os.makedirs(dest_dir, exist_ok=True)
            shutil.copy2(os.path.join(root, file), dest_dir)
