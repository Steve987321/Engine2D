import os 
import sys 
import shutil 

script_dir = os.path.dirname(os.path.abspath(__file__))
proj_dir = os.path.dirname(script_dir)

if sys.platform == "win32":
	bin_release_dir = os.path.join(proj_dir, "bin", "Release-windows-x86_64")
	bin_debug_dir = os.path.join(proj_dir, "bin", "Debug-windows-x86_64")
	sfml_bin_dir = os.path.join(proj_dir, "vendor", "SFML-3.0.0", "bin")
	sfml_bin_release_files = [
		"sfml-audio-2.dll",
		"sfml-graphics-2.dll",
		"sfml-system-2.dll",
		"sfml-network-2.dll",
		"sfml-window-2.dll",
	]
	sfml_bin_debug_files = [
		"sfml-audio-d-2.dll",
		"sfml-graphics-d-2.dll",
		"sfml-system-d-2.dll",
		"sfml-network-d-2.dll",
		"sfml-window-d-2.dll",
	]

elif sys.platform == "darwin": 
	bin_release_dir = os.path.join(proj_dir, "bin", "Release-macosx-x86_64")
	bin_debug_dir = os.path.join(proj_dir, "bin", "Debug-macosx-x86_64")
	sfml_bin_dir = os.path.join(proj_dir, "vendor", "SFML-3.0.0", "lib")
	sfml_bin_release_files = [
		"libsfml-audio.dylib",
		"libsfml-graphics.dylib",
		"libsfml-system.dylib",
		"libsfml-network.dylib",
		"libsfml-window.dylib",
		"libsfml-audio.2.6.dylib",
		"libsfml-graphics.2.6.dylib",
		"libsfml-system.2.6.dylib",
		"libsfml-network.2.6.dylib",
		"libsfml-window.2.6.dylib",
		"libsfml-audio.2.6.dylib",
		"libsfml-graphics.2.6.0.dylib",
		"libsfml-system.2.6.0.dylib",
		"libsfml-network.2.6.0.dylib",
		"libsfml-window.2.6.0.dylib",
	]
	sfml_bin_debug_files = sfml_bin_release_files

is_valid = True 
if os.path.exists(sfml_bin_dir):
	for i, file in enumerate(sfml_bin_release_files): 
		file = os.path.join(sfml_bin_dir, file)
		if not os.path.exists(file):
			print(f"Error: Path doesn't exist: {file}")
			is_valid = False
		else:
			sfml_bin_release_files[i] = file

	for i, file in enumerate(sfml_bin_debug_files): 
		file = os.path.join(sfml_bin_dir, file)
		if not os.path.exists(file):
			print(f"Error: Path doesn't exist: {file}")
			is_valid = False
		else:
			sfml_bin_debug_files[i] = file
else: 
	sys.exit(1)

if not is_valid: 
	sys.exit(1)

print("Creating output directories")
os.makedirs(bin_release_dir, exist_ok=True)
os.makedirs(bin_debug_dir, exist_ok=True)

for file in sfml_bin_release_files: 
	print(f"Copying {file} to {bin_release_dir}")
	shutil.copy2(file, bin_release_dir)

for file in sfml_bin_debug_files: 
	print(f"Copying {file} to {bin_debug_dir}")
	shutil.copy2(file, bin_debug_dir)
