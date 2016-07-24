#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[]) 
{

  long is_len = 4;
  long ir_len = 5;
  long r_len = (4 + 5) - 1;

  double is[4] = {2.0, 4.0, 3.0, 6.0};
  double ir[5] = {1.0, 5.0, 2.0, 3.0, 4.0};
  double r[8];

  double **iSignal = (double **) calloc (r_len * is_len, sizeof (double *));
  double *convSignal = (double *) calloc (r_len, sizeof (double));

  long i, j, z, z_inc = 0;

  // Multiply with padding
  for (i = 0; i < is_len; i++) {

    iSignal[i] = (double *) calloc (r_len, sizeof (double));

    for (j = 0; j < ir_len; j++) {
      iSignal[i][j + z_inc] = is[i] * ir[j];
    }

    z_inc++;
  }

  // Add
  for (i = 0; i < r_len; i++) {
    double z = 0.0;
    for (j = 0; j < is_len; j++) {
      z += iSignal[j][i];
    }
    convSignal[i] = z;
  }

  for (j = 0; j < r_len; j++) {
    printf("%.2f\n", convSignal[j]);
  }

  for (i = 0; i < is_len; i++) {
    free(iSignal[i]);
  }

  free(iSignal);
  free(convSignal);

  return 0;
}
