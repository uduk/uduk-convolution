/*
 *

 o   o o-o   o   o o  o 
 |   | |  \  |   | | /  
 |   | |   O |   | OO   
 |   | |  /  |   | | \  
  o-o  o-o    o-o  o  o  Uduk Delay v1.0

  written by: iherlambang@gmail.com

 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <math.h>

#define CHUNKSIZE 8

double *
readWav (char *filename, long *len) {

  SF_INFO sndInfo_r;
  double *buffer_r;
  long numFrames;

  SNDFILE *sndFile_r = sf_open(filename, SFM_READ, &sndInfo_r);
  if (sndFile_r == NULL) {
    fprintf(stderr, "Error reading source file '%s': %s\n", filename, sf_strerror(sndFile_r));
    exit(EXIT_FAILURE);
  }

  if (sndInfo_r.format != (SF_FORMAT_WAV)) {
    fprintf(stderr, "Input should be 16bit Wav\n");
    sf_close(sndFile_r);
    exit(EXIT_FAILURE);
  }

  buffer_r = malloc((sndInfo_r.frames * sndInfo_r.channels) * sizeof(double));
  if (buffer_r == NULL) {
    fprintf(stderr, "Could not allocate memory for file\n");
    sf_close(sndFile_r);
    exit(EXIT_FAILURE);
  }

  numFrames = sf_readf_double(sndFile_r, buffer_r, sndInfo_r.frames * sndInfo_r.channels);
  if (numFrames != sndInfo_r.frames) {
    fprintf(stderr, "Did not read enough frames for source\n");
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
    fprintf(stderr, "Error opening sound file '%s': %s\n", filename, sf_strerror(sndFile));
    exit(EXIT_FAILURE);
  }

  long writtenFrames = sf_writef_double(sndFile, (const double *) y, numFrames);

  if (writtenFrames != numFrames) {
    fprintf(stderr, "Did not write enough frames for source\n");
    sf_close(sndFile);
    exit(EXIT_FAILURE);
  }

  sf_write_sync(sndFile);
  sf_close(sndFile);
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

  double *outputSignal = (double *) calloc (originalLen, 
                                    sizeof (double));

  double delay = 3 * 44100; 
  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long a = 0; a < originalLen; a++) {
    if (a <= delay) {
      #pragma omp atomic write
      outputSignal[a] = 0;
    }
    if (a > delay) {
      #pragma omp atomic write
      outputSignal[a] = 0.75 * originalSignal[(int) a - (int) delay];
    }
  }

  double *delaySignal = (double *) calloc (originalLen, 
                                    sizeof (double));

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long a = 0; a < originalLen; a++) {
    delaySignal[a] = (originalSignal[a] + outputSignal[a]) * 0.8;
  }

  writeWav(argv[2], delaySignal, originalLen);

  free(delaySignal);
  free(outputSignal);
  free(originalSignal);

  return 0;
}
