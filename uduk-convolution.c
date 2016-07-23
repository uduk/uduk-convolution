/*
 *

 o   o o-o   o   o o  o 
 |   | |  \  |   | | /  
 |   | |   O |   | OO   
 |   | |  /  |   | | \  
  o-o  o-o    o-o  o  o  Uduk Convolution v1.0

  written by: iherlambang@gmail.com

 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <glib.h>

#define CHUNKSIZE 8

double *
readWav (char *filename, long *len) {

  SF_INFO sndInfo_r;
  SNDFILE *sndFile_r = sf_open(filename, SFM_READ, &sndInfo_r);
  if (sndFile_r == NULL) {
    fprintf(stderr, "Error reading source file '%s': %s\n", filename, sf_strerror(sndFile_r));
    exit(EXIT_FAILURE);
  }

  if (sndInfo_r.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
    fprintf(stderr, "Input should be 16bit Wav\n");
    sf_close(sndFile_r);
    exit(EXIT_FAILURE);
  }

  if (sndInfo_r.channels != 1) {
    fprintf(stderr, "Wrong number of channels\n");
    sf_close(sndFile_r);
    exit(EXIT_FAILURE);
  }

  double *buffer_r = malloc(sndInfo_r.frames * sizeof(double));
  if (buffer_r == NULL) {
    fprintf(stderr, "Could not allocate memory for file\n");
    sf_close(sndFile_r);
    exit(EXIT_FAILURE);
  }

  long numFrames = sf_readf_double(sndFile_r, buffer_r, sndInfo_r.frames);
  if (numFrames != sndInfo_r.frames) {
    fprintf(stderr, "Did not read enough frames for source\n");
    sf_close(sndFile_r);
    free(buffer_r);
    exit(EXIT_FAILURE);
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

double 
inline_double_multiply( double p1, double p2)
{
  double out;
  __asm__ __volatile__ ("fmul %2, %0"
      : "=&t" (out) : "%0" (p1), "u" (p2));
  return out;
}

double 
inline_double_add( double p1, double p2)
{
  double out;
  __asm__ __volatile__ ("fadd %2, %0"
      : "=&t" (out) : "%0" (p1), "u" (p2));
  return out;
}

double *
inline_conv (double *a, long a_len, double *b, long b_len)
{
  double *convSignal = (double *) calloc ((a_len + b_len) - 1, sizeof (double));

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < (a_len + b_len) - 1; i++) {
    double z = 0.0;
    int i1 = i;
    for (long j = 0; j < b_len; j++) {
      if (i1 >= 0 && i1 < a_len) {
        double m = inline_double_multiply(a[i1], b[j]);
        z = inline_double_add(z, m);
      }
      i1--;
      convSignal[i] = z;
    }
    g_print("%c[2J %ld of %ld\n", 27, i, (a_len + b_len) - 1);
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

  double *outputSignal = (double *) calloc (originalLen, 
                                    sizeof (double));

  double *convSignal = inline_conv(originalSignal, originalLen, impulseSignal, impulseLen);
  
  double maximum = convSignal[0];
  for (long c = 1; c < (originalLen + impulseLen) - 1; c++) {
    if (convSignal[c] > maximum) {
      maximum  = convSignal[c];
    }
  }

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long c = 0; c < (originalLen + impulseLen) - 1; c++) {
    convSignal[c] = (convSignal[c] / maximum); 
  }

  writeWav(argv[3], convSignal, originalLen);

  free(convSignal);
  free(outputSignal);
  free(impulseSignal);
  free(originalSignal);

  exit(EXIT_SUCCESS);;
}
