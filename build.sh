#!/bin/bash

cc=cc
ld=cc
bdir=build

obj=()

mkdir -p $bdir

for i in $(find src -type f -name "*.c"); do
    o=$bdir/$i.o
    mkdir -p $(dirname $o)
    $cc -c $i -o $o -ansi -Isrc -Wall -Wextra -Wpedantic -g
    obj+=($o)
done

$ld ${obj[@]} -o ld58 -lm -lSDL -lSDL_image -lGL -lGLU
