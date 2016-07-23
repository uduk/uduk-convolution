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
#define RATIO     0.666

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

#ifdef USE_SLOW_TURTLE_CONVOLUTION

double *
conv (double *a, long a_len, double *b, long b_len)
{
  double *convSignal = (double *) calloc ((a_len + b_len) - 1, sizeof (double));

  for (long i = 0; i < (a_len + b_len) - 1; i++) {
    double z = 0.0;
    for (long j = 0; j <= i; ++j) {
      z += ((j < a_len) && (i-j < b_len)) ? a[j] * b[i-j] : 0.0;
    }
    convSignal[i] = z;
    g_print("%c[2J (turtle) %ld of %ld\n", 27, i, (a_len + b_len) - 1);
  }

  return convSignal;
}

#elif USE_UDUK_INLINE_CONVOLUTION

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

double *
conv (double *a, long a_len, double *b, long b_len)
{
  double *convSignal = (double *) calloc ((a_len + b_len) - 1, sizeof (double));

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < (a_len + b_len) - 1; i++) {
    double z, t;
    long iz; 
    
    __asm__ ("mov %2, %1\n\t"
             "movd %1, %0\n\t"
           : "=x" (z), "=r" (t) : "E" (0.0f));

    __asm__ ("mov %1, %0\n\t"
           : "=r" (iz)
           : "r" (i) 
            );  
    
    for (long j = 0; j < b_len; j++) {
      if (iz >= 0 && iz < a_len) {
        double m = inline_double_multiply(a[iz], b[j]);
        z = inline_double_add(z, m);
      }
      iz--;
      convSignal[i] = z;
    }
    g_print("%c[2J (uduk) %ld of %ld\n", 27, i, (a_len + b_len) - 1);
  }

  return convSignal;
}

#else

double *
conv (double *a, long a_len, double *b, long b_len)
{
  double *convSignal = (double *) calloc ((a_len + b_len) - 1, sizeof (double));
  g_print("Under development, please recompile using \"-DUSE_UDUK_INLINE_CONVOLUTION\"\n");
  return convSignal;
}

#endif

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

  double *convSignal = conv(originalSignal, originalLen, impulseSignal, impulseLen);
  
  double maximum = convSignal[0];
  for (long c = 1; c < (originalLen + impulseLen) - 1; c++) {
    if (convSignal[c] > maximum) {
      maximum  = convSignal[c];
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
