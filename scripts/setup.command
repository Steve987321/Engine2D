#! /bin/bash

base_dir=$(dirname $0)
setup_script="$base_dir/setup.py"
premake_script="$base_dir/../premake5.lua"

premake5 --file=$premake_script gmake2
python3 $setup_script