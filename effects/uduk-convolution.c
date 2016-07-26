#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>

#define CHUNKSIZE 8
#define RATIO 0.7

double *
readWav (char *filename, long *len) {

  SF_INFO sndInfo_r;
  double *buffer_r;
  long numFrames;

  SNDFILE *sndFile_r = sf_open(filename, SFM_READ, &sndInfo_r);
  if (sndFile_r == NULL) {
    fprintf(stderr, "sf_open: '%s': %s\n", filename, sf_strerror(sndFile_r));
    exit(EXIT_FAILURE);
  }

  if (sndInfo_r.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
    fprintf(stderr, "Input should be 16bit Wav\n");
    sf_close(sndFile_r);
    exit(EXIT_FAILURE);
  }

  buffer_r = malloc((sndInfo_r.frames * sndInfo_r.channels) * sizeof(double));
  if (buffer_r == NULL) {
    fprintf(stderr, "malloc error\n");
    sf_close(sndFile_r);
    exit(EXIT_FAILURE);
  }

  numFrames = sf_readf_double(sndFile_r, buffer_r, sndInfo_r.frames * sndInfo_r.channels);
  if (numFrames != sndInfo_r.frames) {
    fprintf(stderr, "sf_readf_double problem\n");
    sf_close(sndFile_r);
    free(buffer_r);
    exit(EXIT_FAILURE);
  }

  if (sndInfo_r.channels == 2) {
    double *buffer_m = malloc((sndInfo_r.frames) * sizeof(double));

    for (int i = 0; i < sndInfo_r.frames; i++) {
      buffer_m[i] = 0;

      for(int j = 0; j < sndInfo_r.channels; j++) {
        buffer_m[i] += buffer_r[i * sndInfo_r.channels + j];
      }

      buffer_m[i] /= sndInfo_r.channels;
    }

    sf_close(sndFile_r);
    *len = numFrames;
    return buffer_m;
  }

  sf_close(sndFile_r);
  *len = numFrames;
  return buffer_r;
}

void
writeWav (char *filename, double *y, long numFrames) {

  SF_INFO info;
  info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
  info.channels = 1;
  info.samplerate = 44100;

  SNDFILE *sndFile = sf_open(filename, SFM_WRITE, &info);
  if (sndFile == NULL) {
    fprintf(stderr, "sfopen: '%s': %s\n", filename, sf_strerror(sndFile));
    exit(EXIT_FAILURE);
  }

  long writtenFrames = sf_writef_double(sndFile, (const double *) y, numFrames);

  if (writtenFrames != numFrames) {
    fprintf(stderr, "sf_writef_double problem\n");
    sf_close(sndFile);
    exit(EXIT_FAILURE);
  }

  sf_write_sync(sndFile);
  sf_close(sndFile);
}

double *
convolve (double *originalSignal, long originalLen, double *impulseSignal, long impulseLen)
{
  // fabs, print glib, conv, const
  long convLen = (originalLen + impulseLen) - 1;
  long iLen = impulseLen - 1;
  long oLen = originalLen - 1;

  double *convSignal = (double *) calloc (convLen, sizeof (double));
 
  for (long n = 0; n < convLen; n++) {
    long kmin, kmax, k;

    kmin = (n >= iLen) ? n - iLen : 0;
    kmax = (n < oLen) ? n : oLen;

    for (k = kmin; k <= kmax; k++) {
      convSignal[n] += originalSignal[k] * impulseSignal[n - k];
    }

  }

  return convSignal;
}

int 
main (int argc, char *argv[])
{

  if (argc != 4) {
    fprintf(stderr, "Usage: %s input.wav impulse.wav output.wav\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  long originalLen; 
  double *originalSignal = readWav(argv[1], 
                                      &originalLen);

  long impulseLen; 
  double *impulseSignal = readWav(argv[2], 
                                      &impulseLen);

  double *convSignal = convolve (originalSignal, originalLen, 
                                          impulseSignal, impulseLen);

  double maximum = convSignal[0];
  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long c = 1; c < (originalLen + impulseLen) - 1; c++) {
    double m = fabs(convSignal[c]);
    if (m > maximum) {
      maximum = m;
    }
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long c = 0; c < (originalLen + impulseLen) - 1; c++) {
    convSignal[c] = (convSignal[c] / maximum) * RATIO; 
  }

  writeWav(argv[3], convSignal, originalLen);

  free(convSignal);
  free(impulseSignal);
  free(originalSignal);

  exit(EXIT_SUCCESS);;
}
