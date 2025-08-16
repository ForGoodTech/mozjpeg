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
 * fraction of the original file size saved by compression is written to
 * *rate.
 */
int wasm_compress(const char *infilename, const char *outfilename,
                  int quality, double *rate) {
  tjhandle dhandle = NULL, chandle = NULL;
  unsigned char *inBuf = NULL, *rgbBuf = NULL, *jpegBuf = NULL;
  unsigned long jpegSize = 0;
  size_t inSize = 0;
  int width = 0, height = 0, subsamp = 0;
  FILE *infile = NULL, *outfile = NULL;
  int retval = 0;

  infile = fopen(infilename, "rb");
  if (!infile)
    goto bailout;
  if (fseek(infile, 0, SEEK_END) < 0)
    goto bailout;
  inSize = ftell(infile);
  rewind(infile);
  inBuf = (unsigned char *)tjAlloc(inSize);
  if (!inBuf)
    goto bailout;
  if (fread(inBuf, 1, inSize, infile) != inSize)
    goto bailout;
  fclose(infile);
  infile = NULL;

  dhandle = tjInitDecompress();
  if (!dhandle)
    goto bailout;
  if (tjDecompressHeader2(dhandle, inBuf, inSize, &width, &height, &subsamp) < 0)
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

  chandle = tjInitCompress();
  if (!chandle)
    goto bailout;
  if (tjCompress2(chandle, rgbBuf, width, 0, height, TJPF_RGB, &jpegBuf,
                  &jpegSize, TJSAMP_420, quality, TJFLAG_ACCURATEDCT) < 0)
    goto bailout;

  outfile = fopen(outfilename, "wb");
  if (!outfile)
    goto bailout;
  if (fwrite(jpegBuf, jpegSize, 1, outfile) != 1)
    goto bailout;
  if (rate)
    *rate = (jpegSize >= inSize) ? 0.0 :
            (double)(inSize - jpegSize) / (double)inSize;

  retval = 1; /* Success */

bailout:
  if (!retval) {
    const char *err = chandle ? tjGetErrorStr2(chandle)
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
