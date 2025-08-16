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
  module.FS_writeFile(srcName, new Uint8Array(arrayBuffer));

  const res = module._wasm_compress(srcName, dstName, parseInt(quality.value, 10), 0);
  if (!res) {
    alert('Compression failed.');
    return;
  }

  const out = module.FS_readFile(dstName);
  const blob = new Blob([out], { type: 'image/jpeg' });
  const url = URL.createObjectURL(blob);
  download.style.display = 'inline';
  download.href = url;
  download.download = dstName;
  download.textContent = `Download JPEG (${Math.round(blob.size / 1024)} kB)`;
  preview.src = url;
});
