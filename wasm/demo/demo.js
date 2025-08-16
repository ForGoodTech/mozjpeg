// Simple browser demo for MozJPEG WebAssembly build.
// Requires mozjpeg.js and mozjpeg.wasm built via build.sh in the same directory.

const modulePromise = MozJPEG();

const input = document.getElementById('input');
const quality = document.getElementById('quality');
const qval = document.getElementById('qval');
const download = document.getElementById('download');
const preview = document.getElementById('preview');
const rate = document.getElementById('rate');

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
    module.FS.writeFile(srcName, new Uint8Array(arrayBuffer));

    const savedFraction = module.ccall(
      'wasm_compress',
      'number',
      ['string', 'string', 'number'],
      [srcName, dstName, parseInt(quality.value, 10)]
    );
    if (savedFraction < 0)
      throw new Error('wasm_compress failed');

    const out = module.FS.readFile(dstName);
    const blob = new Blob([out], { type: 'image/jpeg' });
    const url = URL.createObjectURL(blob);
    download.style.display = 'inline';
    download.href = url;
    download.download = dstName;
    download.textContent = `Download JPEG (${Math.round(blob.size / 1024)} kB)`;
    const ratePercent = savedFraction * 100;
    rate.style.display = 'block';
    rate.textContent = `Space saved: ${ratePercent.toFixed(1)}%`;
    preview.src = url;
  } catch (err) {
    console.error('Compression failed:', err);
    alert('Compression failed. See console for details.');
  }
});
