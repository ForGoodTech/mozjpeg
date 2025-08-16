# WebAssembly Build and Demo

This directory contains a minimal WebAssembly build of MozJPEG using
[Emscripten](https://emscripten.org/). The build exposes two functions:

- `wasm_compress()` – recompress a JPEG image at the specified quality and
  return the fraction of the original file size saved (or -1 on failure)
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
`wasm_compress()` from JavaScript. It currently accepts JPEG input only.

To deploy the demo on a static web server:

1. Copy the following files into the same directory on your server:
   - `wasm/build/mozjpeg.js`
   - `wasm/build/mozjpeg.wasm`
   - `wasm/demo/index.html`
   - `wasm/demo/demo.js`
2. Open `index.html` in a browser and select a JPEG image to recompress.
3. Choose a JPEG quality and click **Compress** to download the resulting
   JPEG and view a preview.

The demo uses a flat file layout so it can be served by any simple static
file server.

### Troubleshooting

If the demo reports TurboJPEG errors such as `tj3LoadImage8(): Invalid argument`,
ensure that files are written and read via the Emscripten virtual filesystem:

- Use `module.FS.writeFile` and `module.FS.readFile` in JavaScript.
- Call `wasm_compress` through `module.ccall` so file names are passed as strings.

Using outdated helpers like `FS_writeFile` leaves the input file unwritten and
causes TurboJPEG to receive invalid data.
