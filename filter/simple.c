#include <stdio.h>

int
main (int argc, char *argv[]) 
{
  int N = 10;
  int PrevSample[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int Buffer[10];
  int a0 = 1;
  int a1 = 1;
  int a2 = 1;
  int b0 = 1;
  int b1 = 2;
  int b2 = 3;

  for (int I = 0; I < 10; I++) {
    Buffer[I] = ( b0 / a0 * PrevSample[0]) + (b1 / a0 * PrevSample[1]) + (b2 / a0 * PrevSample[2]) - (a1 / a0 * Buffer[I-1]) - (a2 / a0 * Buffer[I-2]); 
    printf("%d\n", Buffer[I]);
  }

  return 0;
}

