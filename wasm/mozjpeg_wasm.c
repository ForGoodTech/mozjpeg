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
  int width = 0, height = 0, inSubsamp = 0, inColorspace = 0;
  FILE *outfile = NULL;
  int retval = 0;

  /* Load the input image into an RGB buffer. */
  srcBuf = tjLoadImage(infilename, &width, 0, &height, &inColorspace, TJPF_RGB);
  if (!srcBuf)
    goto bailout;

  handle = tjInitCompress();
  if (!handle)
    goto bailout;

  if (tjCompress2(handle, srcBuf, width, 0, height, TJPF_RGB, &jpegBuf,
                  &jpegSize, TJSAMP_420, quality, TJFLAG_ACCURATEDCT) < 0)
    goto bailout;

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
    *rate = (double)jpegSize / (double)(width * height * tjPixelSize[TJPF_RGB]) * 8.0;

  retval = 1;  /* Success */

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
