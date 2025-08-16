// Simple browser demo for MozJPEG WebAssembly build.
// Requires mozjpeg.js and mozjpeg.wasm built via build.sh in the same directory.

const modulePromise = MozJPEG();

const input = document.getElementById('input');
const quality = document.getElementById('quality');
const qval = document.getElementById('qval');
const download = document.getElementById('download');
const preview = document.getElementById('preview');

quality.addEventListener('input', () => {
  qval.textContent = quality.value;
});

document.getElementById('compress').addEventListener('click', async () => {
  if (!input.files.length) {
    alert('Select an image first.');
    return;
  }

  const file = input.files[0];
  const arrayBuffer = await file.arrayBuffer();
  const module = await modulePromise;

  const srcName = file.name;
  const dstName = 'out.jpg';
  try {
    console.log('Writing input file', srcName, arrayBuffer.byteLength, 'bytes');
    module.FS.writeFile(srcName, new Uint8Array(arrayBuffer));
    console.log('Input file written');

    console.log('Calling wasm_compress');
    const res = module.ccall(
      'wasm_compress',
      'number',
      ['string', 'string', 'number', 'number'],
      [srcName, dstName, parseInt(quality.value, 10), 0]
    );
    console.log('wasm_compress returned', res);
    if (!res) throw new Error('wasm_compress returned 0');

    console.log('Reading output file');
    const out = module.FS.readFile(dstName);
    console.log('Output file size', out.length, 'bytes');
    const blob = new Blob([out], { type: 'image/jpeg' });
    const url = URL.createObjectURL(blob);
    download.style.display = 'inline';
    download.href = url;
    download.download = dstName;
    download.textContent = `Download JPEG (${Math.round(blob.size / 1024)} kB)`;
    preview.src = url;
  } catch (err) {
    console.error('Compression failed:', err);
    alert('Compression failed. See console for details.');
  }
});
