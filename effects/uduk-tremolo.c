/*
 *

 o   o o-o   o   o o  o 
 |   | |  \  |   | | /  
 |   | |   O |   | OO   
 |   | |  /  |   | | \  
  o-o  o-o    o-o  o  o  Uduk Tremolo v1.0

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

  if (sndInfo_r.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
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

double 
inline_double_multiply (double p1, double p2)
{
  double out;
  __asm__ __volatile__ ("fmul %2, %0"
      : "=&t" (out) : "%0" (p1), "u" (p2));
  return out;
}

double 
inline_double_add (double p1, double p2)
{
  double out;
  __asm__ __volatile__ ("fadd %2, %0"
      : "=&t" (out) : "%0" (p1), "u" (p2));
  return out;
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

  int Fs = 44100;
  float Fc = 5;
  double F = Fc / Fs;
  float alpha = 0.5;

  #pragma omp parallel for
  for (long i = 0; i < originalLen; i++) {
    double z = inline_double_multiply(3.14159265358979323846, F);
    double ans;
    __asm__ __volatile__ ("fsin" : "=t" (ans) : "0" (sin(2 * i * z)));
    double tremolo = inline_double_add(1,  inline_double_multiply(alpha,  ans));
    originalSignal[i] = inline_double_multiply(originalSignal[i], tremolo);
  }

  writeWav(argv[2], originalSignal, originalLen);

  free(originalSignal);

  exit(EXIT_SUCCESS);;
}
