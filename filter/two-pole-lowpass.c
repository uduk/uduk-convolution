#include <stdio.h>
#include <math.h>

double 
TwoPoleLowPassFilter (float cutoff, double x, int samplerate)
{
  double Q = 1.1f;
  double w0 = 2 * M_PI * cutoff / samplerate;
  double alpha = sin(w0) / (2.0 * Q);
  double b0 =  (1.0 - cos(w0))/2;
  double b1 =   1.0 - cos(w0);
  double b2 =  (1.0 - cos(w0))/2;
  double a0 =   1.0 + alpha;
  double a1 =  -2.0 * cos(w0);
  double a2 = 1.0 - alpha;
  double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

  double y = (b0/a0)*x + (b1/a0)*x1 + (b2/a0)*x2 - (a1/a0)*y1 - (a2/a0)*y2;                  
  x2 = x1;
  x1 = x;
  y2 = y1;
  y1 = y;

  return y;
}

int
main (int argc, char *argv[])
{

  double r = TwoPoleLowPassFilter(2.0, 0.2, 22050);
  printf("%.16f\n", r);

  return 0;
}
