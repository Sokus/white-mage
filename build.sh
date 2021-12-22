#!/bin/bash

# Figure out the script location (to use absolute paths)
script=$(readlink -f "$0")
location=$(dirname "$script")


# Common flags
warnings="-Wall -Wextra -Wshadow -Wconversion -Wdouble-promotion -Wno-unused-function"
common="-O0 -g -D NISK_DEBUG=1 -lm"

# Source files to compile
platform_src="$location/code/linux_platform.c"
glad_src="$location/external/src/glad/glad.c"
sources="$platform_src $glad_src"

# External headers/libraries
inc_dir="$location/external/include"
lib_dir="$location/external/lib/linux"
sdl_lib="$lib_dir/SDL2"
sdl_flags="-D _REENTRANT -lSDL2 -Wl,-rpath,$ORIGIN$sdl_lib"
external_flags="-I$inc_dir $glad_flags $sdl_flags"

mkdir -p build
cd build

gcc $sources -o card-game.out $common $warnings -lGL -ldl $external_flags 