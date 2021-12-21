#!/bin/bash

# Figure out the script location (to use absolute paths)
script=$(readlink -f "$0")
location=$(dirname "$script")


# Common flags
warnings="-Wall -Wextra -Wshadow -Wconversion -Wdouble-promotion -Wno-unused-function"
common="-O0 -g -D NISK_DEBUG=1 -lm"

# Source files to compile
platform_src="$location/code/main.c"

# External headers/libraries
inc_dir="$location/external/include"
lib_dir="$location/external/lib/linux"
sdl_lib="$lib_dir/SDL2"
sdl_flags="-I$inc_dir/SDL2 -D _REENTRANT -L$sdl_lib -lSDL2"
external_flags="$sdl_flags -Wl,-rpath,$ORIGIN$sdl_lib"

mkdir -p build
cd build

gcc $platform_src -o game.out $common $warnings -lGL -ldl $external_flags