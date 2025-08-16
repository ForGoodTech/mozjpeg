#!/usr/bin/env bash
set -euo pipefail

# Build mozjpeg for WebAssembly using Emscripten.
# The resulting module exposes wasm_compress() and wasm_get_progress().

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Locate the Emscripten SDK and source its environment if needed
EMS_SDK_DIR=${EMS_SDK_DIR:-/home/ubuntu/3rdParty/emsdk}
if [ -f "$EMS_SDK_DIR/emsdk_env.sh" ]; then
  # shellcheck disable=SC1091
  source "$EMS_SDK_DIR/emsdk_env.sh" >/dev/null
else
  echo "Emscripten SDK not found in $EMS_SDK_DIR" >&2
  exit 1
fi

mkdir -p "$BUILD_DIR"
pushd "$BUILD_DIR" >/dev/null

# Configure and build static libraries (only configure if not already)
if [ ! -f Makefile ]; then
  emcmake cmake "$ROOT_DIR" -DENABLE_SHARED=OFF -DENABLE_STATIC=ON -DWITH_TURBOJPEG=ON
fi
emmake make -j"$(nproc)"

# Compile the WebAssembly module with the wrapper
emcc "$SCRIPT_DIR/mozjpeg_wasm.c" "$ROOT_DIR/tjutil.c" libjpeg.a turbojpeg.a \
  -I"$ROOT_DIR" -O3 \
  -s EXPORTED_FUNCTIONS='["_wasm_compress","_wasm_get_progress"]' \
  -s MODULARIZE=1 -s EXPORT_NAME="MozJPEG" \
  -o mozjpeg.js

popd >/dev/null

echo "Build artifacts are located in $BUILD_DIR"
