#include "bthread.h"
#include "schedulers.h"
#include "tbarrier.h"
#include <stdio.h>

double A[3][3] = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9} };
double B[3][3];
double C[3][3];

bthread_barrier_t barrier;

void multiply(double X[3][3], double Y[3][3], double Z[3][3],
                int row, int column) {
  int i;
  Z[row][column] = 0;
  for(i=0; i<3; i++) {
    Z[row][column] += X[row][i] * Y[i][column];
  }
}

void* thread_multiply (void* args)
{
  int row = (int) args / 3;
  int column = (int) args % 3;
  multiply(A, A, B, row, column);
  bthread_barrier_wait(&barrier);
  multiply(A, B, C, row, column);
  return NULL;
}

int main(int argc, char *argv[])
{
  bthread_t threads[9];
  int t, r, c;
  
  if(!bthread_barrier_init(&barrier, NULL, 9)) {
    for (t=0; t<9; t++) {
      bthread_create(&threads[t], NULL, &thread_multiply, (void*) t);
    }
    for (t=0; t<9; t++) {
      bthread_join(threads[t], NULL);
    }
    printf("The cube is:\n");
    for (r=0; r<3; r++) {
      for (c=0; c<3; c++) {
        printf(" %3.2f ", C[r][c]);
      }
      printf("\n");
    }
    bthread_barrier_destroy(&barrier);
  }
}
