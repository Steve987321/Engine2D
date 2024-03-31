import os 
import sys 
import shutil 

script_dir = os.path.dirname(os.path.abspath(__file__))
proj_dir = os.path.dirname(script_dir)

if sys.platform == "win32":
	dl_file_ext = ".dll"
	dl_file_prefix = ""
	sfml_bin_dir = os.path.join(proj_dir, "vendor", "SFML-2.6.0", "bin")
elif sys.platform == "darwin": 
	dl_file_ext = ".dylib"
	dl_file_prefix = "lib"
	sfml_bin_dir = os.path.join(proj_dir, "vendor", "SFML-2.6.0", "lib")

# output 
bin_release_dir = os.path.join(proj_dir, "bin", "Release-windows-x86_64")
bin_debug_dir = os.path.join(proj_dir, "bin", "Debug-windows-x86_64")

sfml_bin_release_files = [
	dl_file_prefix + "sfml-audio-2" + dl_file_ext,
	dl_file_prefix + "sfml-graphics-2" + dl_file_ext,
	dl_file_prefix + "sfml-system-2" + dl_file_ext,
	dl_file_prefix + "sfml-network-2" + dl_file_ext,
	dl_file_prefix + "sfml-window-2" + dl_file_ext,
]
sfml_bin_debug_files = [
	dl_file_prefix + "sfml-audio-d-2" + dl_file_ext,
	dl_file_prefix + "sfml-graphics-d-2" + dl_file_ext,
	dl_file_prefix + "sfml-system-d-2" + dl_file_ext,
	dl_file_prefix + "sfml-network-d-2" + dl_file_ext,
	dl_file_prefix + "sfml-window-d-2" + dl_file_ext,
]

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

if sys.platform == "win32": 
	for file in sfml_bin_debug_files: 
		print(f"Copying {file} to {bin_debug_dir}")
		shutil.copy2(file, bin_debug_dir)
