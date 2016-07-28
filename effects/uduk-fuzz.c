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

  if (sndInfo_r.format != (SF_FORMAT_WAV)) {
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
signumInverse (double *originalSignal, long originalLen)
{
  double *signumSignal = (double *) calloc (originalLen, sizeof (double));

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {

    if (originalSignal[i] > 0) {
      signumSignal[i] = -1;
    }
    else if (originalSignal[i] < 0) {
      signumSignal[i] = 1;
    }
    else if (originalSignal[i] == 0) {
      signumSignal[i] = 0;
    }

  }

  return signumSignal;
}

double *
fuzz (double *originalSignal, long originalLen)
{
  double *fuzzSignal = (double *) calloc (originalLen, sizeof (double));
  double *q = (double *) calloc (originalLen, sizeof (double));
  double *z = (double *) calloc (originalLen, sizeof (double));
  double *expq = (double *) calloc (originalLen, sizeof (double));
  double *signum = signumInverse(originalSignal, originalLen);

  int gain = 21;
  float mix = 0.8;

  double maximumAbs = originalSignal[0];
  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long c = 1; c < originalLen; c++) {
    double m = fabs(originalSignal[c]);
    if (m > maximumAbs) {
      maximumAbs = m;
    }
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    q[i] = originalSignal[i] * gain / maximumAbs;
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    expq[i] = exp(signum[i] * q[i]);
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    z[i] = signum[i] * (1 - expq[i]); 
  }

  double maximumZ = z[0];
  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long c = 1; c < originalLen; c++) {
    double m = fabs(z[c]);
    if (m > maximumZ) {
      maximumZ = m;
    }
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    fuzzSignal[i] = mix * z[i] * maximumAbs / maximumZ + (1 - mix) * originalSignal[i];
  }

  double maximumY = fuzzSignal[0];
  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long c = 1; c < originalLen; c++) {
    double m = fabs(fuzzSignal[c]);
    if (m > maximumY) {
      maximumY = m;
    }
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    fuzzSignal[i] = fuzzSignal[i] * maximumAbs  / maximumY;
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    fuzzSignal[i] = fuzzSignal[i] / 1 + fabs(fuzzSignal[i]);
  }
  
  free(q);
  free(z);
  free(expq);
  free(signum);
  
  return fuzzSignal;
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

  double *fuzzSignal = fuzz(originalSignal, originalLen);

  writeWav(argv[2], fuzzSignal, originalLen);

  free(fuzzSignal);
  free(originalSignal);

  exit(EXIT_SUCCESS);;
}
