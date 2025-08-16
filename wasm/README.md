# WebAssembly Build and Demo

This directory contains a minimal WebAssembly build of MozJPEG using
[Emscripten](https://emscripten.org/). The build exposes two functions:

- `wasm_compress()` – compress a source image to JPEG
- `wasm_get_progress()` – placeholder progress callback

## Build

1. Install the Emscripten SDK and set the `EMS_SDK_DIR` environment variable
   to point to it if it is not located at `/home/ubuntu/3rdParty/emsdk`.
2. Run the build script:

   ```sh
   ./build.sh
   ```

   The compiled artifacts `mozjpeg.js` and `mozjpeg.wasm` will be placed in
   `wasm/build/`.

## Browser Demo

The `demo/` subdirectory contains a very simple browser demo that calls
`wasm_compress()` from JavaScript.

To deploy the demo on a static web server:

1. Copy the following files into the same directory on your server:
   - `wasm/build/mozjpeg.js`
   - `wasm/build/mozjpeg.wasm`
   - `wasm/demo/index.html`
   - `wasm/demo/demo.js`
2. Open `index.html` in a browser and select an input image (PNG, BMP, etc.).
3. Choose a JPEG quality and click **Compress** to download the resulting
   JPEG and view a preview.

The demo uses a flat file layout so it can be served by any simple static
file server.
