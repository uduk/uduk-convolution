#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sndfile.h>

#define CHUNKSIZE 8

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
neuromodulation (long originalLen)
{
  double *nSignal = (double *) calloc (originalLen, sizeof (double));
  const int fs = 44100;

  srand(time(NULL));

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long x = 0; x < originalLen / fs; x++) {

    int step = x * fs;
    int q = 11025;

    unsigned int freq = 100 + rand() % 1000;
    double w = 2 * M_PI * freq / fs;

    for (long i = x * fs; i < step + q; i++) {
      nSignal[i] = 0.6 * sin(w * i * 0.6);
    }
   
    for (long i = step + q; i < step + q * 2; i++) {
      nSignal[i] = 0.0;
    }
   
    freq = 1000 + rand() % 10000;
    w = 2 * M_PI * freq / fs;

    for (long i = step + q * 2; i < step + q * 3; i++) {
      nSignal[i] = 0.6 * sin(w * i * 0.6);
    }

    for (long i = step + q * 3; i < step + q * 4; i++) {
      nSignal[i] = 0.0;
    }
  }

  return nSignal;
}

int 
main (int argc, char *argv[])
{

  if (argc != 3) {
    fprintf(stderr, "Usage: %s seconds output.wav\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  long len = (atoi(argv[1]) * 44100);

  double *nSignal = neuromodulation(len);

  writeWav(argv[2], nSignal, len);

  free(nSignal);

  exit(EXIT_SUCCESS);;
}
