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
blur (long originalLen)
{
  double *blurSignal = (double *) calloc (originalLen, sizeof (double));

  const static int q = 15;
  const static float c1 = (1 << q) - 1;
  const static float c2 = ((int)(c1 / 3)) + 1;
  const static float c3 = 1.f / c1;

  float random = 0.f;

  srand(time(NULL));

  double w = 2 * M_PI * 432 / 44100;

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (long i = 0; i < originalLen; i++) {
    random = ((float) rand() / (float) (RAND_MAX + 1));
    blurSignal[i] = 0.1 * ((2.f * ((random * c2) + (random * c2) + (random * c2)) - 3.f * (c2 - 1.f)) * c3); 
    blurSignal[i] += 0.6 * sin(w * i * 0.6);
  }
 
  return blurSignal;
}

int 
main (int argc, char *argv[])
{

  if (argc != 3) {
    fprintf(stderr, "Usage: %s seconds output.wav\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  long len = (atoi(argv[1]) * 44100);

  double *blurSignal = blur(len);

  writeWav(argv[2], blurSignal, len);

  free(blurSignal);

  exit(EXIT_SUCCESS);;
}
