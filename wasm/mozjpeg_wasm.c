#include <stdio.h>
#include <stdlib.h>
#include "turbojpeg.h"

/*
 * Simple WebAssembly-friendly wrappers around the TurboJPEG API.
 * These functions expect that the input file contains an image format
 * supported by tjLoadImage() (for instance, PNG or BMP.)
 */

/* Placeholder progress function.  Real applications can wire this into
 * their own progress callbacks.  Returning 0.0 indicates unknown progress.
 */
double wasm_get_progress(void) {
  return 0.0;
}

/*
 * Compress an image file to JPEG.
 * Returns 1 on success and 0 on failure.  If rate is non-NULL then the
 * approximate bits-per-pixel of the compressed image is written to *rate.
 */
int wasm_compress(const char *infilename, const char *outfilename,
                  int quality, double *rate) {
  tjhandle handle = NULL;
  unsigned char *srcBuf = NULL, *jpegBuf = NULL;
  unsigned long jpegSize = 0;
  int width = 0, height = 0, pixelFormat = TJPF_RGB;
  FILE *outfile = NULL;
  int retval = 0;

  fprintf(stderr, "wasm_compress: '%s' -> '%s' quality=%d\n", infilename,
          outfilename, quality);
  fflush(stderr);

  fprintf(stderr, "Loading input file...\n");
  srcBuf = tjLoadImage(infilename, &width, 1, &height, &pixelFormat, 0);
  if (!srcBuf)
    goto bailout;
  fprintf(stderr, "Loaded image %dx%d\n", width, height);
  fflush(stderr);

  fprintf(stderr, "Initializing compressor...\n");
  handle = tjInitCompress();
  if (!handle)
    goto bailout;

  fprintf(stderr, "Compressing...\n");
  if (tjCompress2(handle, srcBuf, width, 0, height, TJPF_RGB, &jpegBuf,
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
    const char *err = handle ? tjGetErrorStr2(handle) : tjGetErrorStr();
    fprintf(stderr, "TurboJPEG error: %s\n", err);
  }
  if (outfile)
    fclose(outfile);
  if (jpegBuf)
    tjFree(jpegBuf);
  if (srcBuf)
    tjFree(srcBuf);
  if (handle)
    tjDestroy(handle);
  return retval;
}
