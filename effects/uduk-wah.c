#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>

#define CHUNKSIZE 8

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
wah (double *originalSignal, long originalLen)
{
  double *wahSignal = (double *) calloc (originalLen, sizeof (double));

  float damp = 0.05;
  int minf = 500;
  int maxf = 3000;
  int Fw = 2000;
  int Fs = 44100;
  double delta = (double) Fw/Fs;

  double *Fc = (double *) calloc (originalLen * 2, sizeof(double));

  long index = 0;
  double i = (double) minf;

  do {
    Fc[index++] = i;
    i += delta;
  } while (i <= maxf);

  while (index < originalLen) {
    double ii = (double) maxf;
    do {
      Fc[index++] = ii;
      ii -= delta;
    } while (ii >= minf);

    double iii = (double) minf;
    do {
      Fc[index++] = iii;
      iii += delta;
    } while (iii <= maxf);
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    Fc[i] = Fc[i];
  }

  double F1 = 2 * sin((M_PI * Fc[1]) / Fs);
  double Q1 = 2 * damp;

  double *yh = (double *) calloc (originalLen, sizeof(double));
  double *yb = (double *) calloc (originalLen, sizeof(double));
  double *yl = (double *) calloc (originalLen, sizeof(double));

  yh[1] = originalSignal[1];
  yb[1] = F1 * yh[1];
  yl[1] = F1 * yb[1];

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 2; i < originalLen; i++) {
    yh[i] = originalSignal[i] - yl[i-1] - Q1 * yb[i-1];
    yb[i] = F1 * yh[i] + yb[i-1];
    yl[i] = F1 * yb[i] + yl[i-1];
    F1 = 2 * sin((M_PI * Fc[i]) / Fs);
  }

  double maxAbs = 0.0;
  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    double m = fabs(yb[i]); 
    if (m > maxAbs)
      maxAbs = m;
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    wahSignal[i] = yb[i] / maxAbs; 
  }

  free(yh);
  free(yb);
  free(yl);
  free(Fc);

  return wahSignal;
}

int 
main (int argc, char *argv[])
{

  if (argc != 3) {
    fprintf(stderr, "Usage: %s input.wav output.wav\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  long originalLen; 
  double *originalSignal = readWav(argv[1], 
      &originalLen);

  double *wahSignal = wah(originalSignal, originalLen);

  writeWav(argv[2], wahSignal, originalLen);

  free(wahSignal);
  free(originalSignal);

  exit(EXIT_SUCCESS);;
}
