#!/bin/bash

cc=emcc
ld=emcc
bdir=build

obj=()

mkdir -p $bdir
mkdir -p web

for i in $(find src -type f -name "*.c"); do
    o=$bdir/$i.o
    mkdir -p $(dirname $o)
    $cc -c $i -o $o -ansi -Isrc -Wall -Wextra -Wpedantic -g
    obj+=($o)
done

find assets -type f -printf "--preload-file %p " | xargs \
    $ld ${obj[@]} -sUSE_SDL=1 -sUSE_SDL_IMAGE=1 -sLEGACY_GL_EMULATION \
    -sGL_FFP_ONLY -sMODULARIZE=1 -sALLOW_MEMORY_GROWTH \
    --use-preload-plugins -o web/index.js -Isrc -Iem

cp index.html web

cd assets
for i in $(find . -type f -name "*.png"); do
    mkdir -p $(dirname $i)
    cp $i ../web
done
cd ..
