#include <stdio.h>
#include <stdlib.h>
#include "turbojpeg.h"

/*
 * Simple WebAssembly-friendly wrappers around the TurboJPEG API.
 * The demo currently supports JPEG input only.
 */

/* Placeholder progress function.  Real applications can wire this into
 * their own progress callbacks.  Returning 0.0 indicates unknown progress.
 */
double wasm_get_progress(void) {
  return 0.0;
}

/*
 * Recompress a JPEG file with a new quality setting.
 * Returns 1 on success and 0 on failure.  If rate is non-NULL then the
 * approximate bits-per-pixel of the compressed image is written to *rate.
 */
int wasm_compress(const char *infilename, const char *outfilename,
                  int quality, double *rate) {
  tjhandle dhandle = NULL, chandle = NULL;
  unsigned char *inBuf = NULL, *rgbBuf = NULL, *jpegBuf = NULL;
  unsigned long jpegSize = 0;
  size_t inSize = 0;
  int width = 0, height = 0;
  FILE *infile = NULL, *outfile = NULL;
  int retval = 0;

  fprintf(stderr, "wasm_compress: '%s' -> '%s' quality=%d\n", infilename,
          outfilename, quality);
  fflush(stderr);

  fprintf(stderr, "Loading input file...\n");
  infile = fopen(infilename, "rb");
  if (!infile) {
    perror(infilename);
    goto bailout;
  }
  if (fseek(infile, 0, SEEK_END) < 0)
    goto bailout;
  inSize = ftell(infile);
  rewind(infile);
  inBuf = (unsigned char *)tjAlloc(inSize);
  if (!inBuf)
    goto bailout;
  if (fread(inBuf, 1, inSize, infile) != inSize) {
    perror(infilename);
    goto bailout;
  }
  fclose(infile);
  infile = NULL;

  dhandle = tjInitDecompress();
  if (!dhandle)
    goto bailout;
  if (tjDecompressHeader2(dhandle, inBuf, inSize, &width, &height, NULL) < 0)
    goto bailout;
  rgbBuf = (unsigned char *)tjAlloc(width * height * tjPixelSize[TJPF_RGB]);
  if (!rgbBuf)
    goto bailout;
  if (tjDecompress2(dhandle, inBuf, inSize, rgbBuf, width, 0, height,
                    TJPF_RGB, 0) < 0)
    goto bailout;
  tjDestroy(dhandle);
  dhandle = NULL;
  tjFree(inBuf);
  inBuf = NULL;
  fprintf(stderr, "Loaded image %dx%d\n", width, height);
  fflush(stderr);

  fprintf(stderr, "Initializing compressor...\n");
  chandle = tjInitCompress();
  if (!chandle)
    goto bailout;

  fprintf(stderr, "Compressing...\n");
  if (tjCompress2(chandle, rgbBuf, width, 0, height, TJPF_RGB, &jpegBuf,
                  &jpegSize, TJSAMP_420, quality, TJFLAG_ACCURATEDCT) < 0)
    goto bailout;
  fprintf(stderr, "Compression produced %lu bytes\n", jpegSize);
  fflush(stderr);

  fprintf(stderr, "Writing output file...\n");
  outfile = fopen(outfilename, "wb");
  if (!outfile) {
    perror(outfilename);
    goto bailout;
  }
  if (fwrite(jpegBuf, jpegSize, 1, outfile) != 1) {
    perror(outfilename);
    goto bailout;
  }

  if (rate)
    *rate = (double)jpegSize /
            (double)(width * height * tjPixelSize[TJPF_RGB]) * 8.0;

  retval = 1; /* Success */

bailout:
  if (!retval) {
    const char *err = chandle
                          ? tjGetErrorStr2(chandle)
                          : (dhandle ? tjGetErrorStr2(dhandle)
                                     : tjGetErrorStr());
    fprintf(stderr, "TurboJPEG error: %s\n", err);
  }
  if (outfile)
    fclose(outfile);
  if (jpegBuf)
    tjFree(jpegBuf);
  if (rgbBuf)
    tjFree(rgbBuf);
  if (inBuf)
    tjFree(inBuf);
  if (chandle)
    tjDestroy(chandle);
  if (dhandle)
    tjDestroy(dhandle);
  if (infile)
    fclose(infile);
  return retval;
}
