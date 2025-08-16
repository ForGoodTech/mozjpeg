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
  int width = 0, height = 0, subsamp = 0;
  FILE *infile = NULL, *outfile = NULL;
  int retval = 0;

  /* Trace macro to emit the current source line for debugging */
#define TRACE_LINE()                                                      \
  do {                                                                    \
    fprintf(stderr, "TRACE %s:%d\n", __FILE__, __LINE__);               \
    fflush(stderr);                                                       \
  } while (0)

  TRACE_LINE();
  fprintf(stderr, "wasm_compress: '%s' -> '%s' quality=%d\n", infilename,
          outfilename, quality);
  fflush(stderr);

  TRACE_LINE();
  fprintf(stderr, "Loading input file...\n");
  infile = fopen(infilename, "rb");
  TRACE_LINE();
  if (!infile) {
    perror(infilename);
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  if (fseek(infile, 0, SEEK_END) < 0) {
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  inSize = ftell(infile);
  TRACE_LINE();
  rewind(infile);
  TRACE_LINE();
  inBuf = (unsigned char *)tjAlloc(inSize);
  TRACE_LINE();
  if (!inBuf) {
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  if (fread(inBuf, 1, inSize, infile) != inSize) {
    perror(infilename);
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  fclose(infile);
  TRACE_LINE();
  infile = NULL;

  TRACE_LINE();
  dhandle = tjInitDecompress();
  TRACE_LINE();
  if (!dhandle) {
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  if (tjDecompressHeader2(dhandle, inBuf, inSize, &width, &height, &subsamp) < 0) {
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  rgbBuf = (unsigned char *)tjAlloc(width * height * tjPixelSize[TJPF_RGB]);
  TRACE_LINE();
  if (!rgbBuf) {
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  if (tjDecompress2(dhandle, inBuf, inSize, rgbBuf, width, 0, height,
                    TJPF_RGB, 0) < 0) {
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  tjDestroy(dhandle);
  TRACE_LINE();
  dhandle = NULL;
  TRACE_LINE();
  tjFree(inBuf);
  TRACE_LINE();
  inBuf = NULL;
  TRACE_LINE();
  fprintf(stderr, "Loaded image %dx%d\n", width, height);
  fflush(stderr);

  TRACE_LINE();
  fprintf(stderr, "Initializing compressor...\n");
  TRACE_LINE();
  chandle = tjInitCompress();
  TRACE_LINE();
  if (!chandle) {
    TRACE_LINE();
    goto bailout;
  }

  TRACE_LINE();
  fprintf(stderr, "Compressing...\n");
  TRACE_LINE();
  if (tjCompress2(chandle, rgbBuf, width, 0, height, TJPF_RGB, &jpegBuf,
                  &jpegSize, TJSAMP_420, quality, TJFLAG_ACCURATEDCT) < 0) {
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  fprintf(stderr, "Compression produced %lu bytes\n", jpegSize);
  fflush(stderr);

  TRACE_LINE();
  fprintf(stderr, "Writing output file...\n");
  TRACE_LINE();
  outfile = fopen(outfilename, "wb");
  TRACE_LINE();
  if (!outfile) {
    perror(outfilename);
    TRACE_LINE();
    goto bailout;
  }
  TRACE_LINE();
  if (fwrite(jpegBuf, jpegSize, 1, outfile) != 1) {
    perror(outfilename);
    TRACE_LINE();
    goto bailout;
  }

  TRACE_LINE();
  if (rate) {
    *rate = (double)jpegSize /
            (double)(width * height * tjPixelSize[TJPF_RGB]) * 8.0;
    TRACE_LINE();
  }

  TRACE_LINE();
  retval = 1; /* Success */

bailout:
  TRACE_LINE();
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
  TRACE_LINE();
  return retval;
}

#undef TRACE_LINE
