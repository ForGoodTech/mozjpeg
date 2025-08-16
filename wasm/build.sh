#!/usr/bin/env bash
set -euo pipefail

# Build mozjpeg for WebAssembly using Emscripten.
# The resulting module exposes wasm_compress() and wasm_get_progress().

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
pushd "$BUILD_DIR" >/dev/null

# Configure and build static libraries
emcmake cmake "$ROOT_DIR" -DENABLE_SHARED=OFF -DENABLE_STATIC=ON -DWITH_TURBOJPEG=ON
emmake make -j"$(nproc)"

# Compile the WebAssembly module with the wrapper
emcc "$SCRIPT_DIR/mozjpeg_wasm.c" "$ROOT_DIR/tjutil.c" libjpeg.a turbojpeg.a \
  -I"$ROOT_DIR" -O3 \
  -s EXPORTED_FUNCTIONS='["_wasm_compress","_wasm_get_progress"]' \
  -s MODULARIZE=1 -s EXPORT_NAME="MozJPEG" \
  -o mozjpeg.js

popd >/dev/null

echo "Build artifacts are located in $BUILD_DIR"
