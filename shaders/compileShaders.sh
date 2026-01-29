#!/bin/bash

SRC_DIR="./sources"
DST_DIR="./compiled"

rm -rf "$DST_DIR"
mkdir -p "$DST_DIR"

for shader in "$SRC_DIR"/*; do
	filename=$(basename -- "$shader")
	out_file="$DST_DIR/$filename.spv"
	echo "Compiling shader $filename"
	glslangValidator -V "$shader" -o "$out_file"
done
