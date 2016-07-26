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
softclipping (double *originalSignal, long originalLen)
{
  double *softSignal = (double *) calloc (originalLen, sizeof (double));

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {

    double s = originalSignal[i];

    if (s >= 0 && s <= 1/3) {
      softSignal[i] = 2 * originalSignal[i];
    }

    else if (s >= 1/3 && s <= 2/3) {
      //softSignal[i] = (3 - pow (2 - (3 * i), 2)) / 3;  
      softSignal[i] = 2 * pow(s, 2) - 3 * pow(s, 4); // cardiif
    }

    else if (s >= 2/3 && s <= 1) {
      softSignal[i] = 1.0;
    }
  }

  return softSignal;
}

double *
overdrive2 (double *originalSignal, long originalLen, double *softSignal)
{
  double *driveSignal = (double *) calloc (originalLen, sizeof (double));
  double th = 1/3;

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    driveSignal[i] = softSignal[i];
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {

    if (fabs(originalSignal[i]) < th) {
      driveSignal[i] = 2 * originalSignal[i]; 
    }

    else if (fabs(originalSignal[i]) >= th) {

      if (originalSignal[i] > 0) {
        driveSignal[i] = (3 - pow(2 - originalSignal[i] * 3, 2)) / 3; 
      }
      else if (originalSignal[i] < 0) {
        driveSignal[i] = -(3 - pow(2 - fabs(originalSignal[i]) * 3, 2)) / 3;
      }

    }

    else if (fabs(originalSignal[i]) > 2 * th) {
      if (originalSignal[i] > 0)
        driveSignal[i] = 1;

      else if (originalSignal[i] < 0)
        driveSignal[i] = -1;
    }

  }

  return driveSignal;
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

  double *overSignal = softclipping (originalSignal, originalLen);

  writeWav(argv[2], overSignal, originalLen);

  free(overSignal);
  free(originalSignal);

  exit(EXIT_SUCCESS);;
}
