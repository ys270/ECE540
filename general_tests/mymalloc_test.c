#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p)    ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p)    bf_free(p)
#endif


int main(int argc, char *argv[])
{
  const unsigned NUM_ITEMS = 10;
  int i;
  int size;
  int sum = 0;
  int expected_sum = 0;
  int *array[NUM_ITEMS];

  size = 4;
  expected_sum += size * size;
  array[0] = (int *)MALLOC(size * sizeof(int));
  printf("address of 0 is %p\n",array[0]);
  for (i=0; i < size; i++) {
    array[0][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[0][i];
  } //for i

  size = 16;
  expected_sum += size * size;
  array[1] = (int *)MALLOC(size * sizeof(int));
    printf("address of 1 is %p\n",array[1]);
  for (i=0; i < size; i++) {
    array[1][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[1][i];
  } //for i

  size = 8;
  expected_sum += size * size;
  array[2] = (int *)MALLOC(size * sizeof(int));
    printf("address of 2 is %p\n",array[2]);
  for (i=0; i < size; i++) {
    array[2][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[2][i];
  } //for i

  size = 32;
  expected_sum += size * size;
  array[3] = (int *)MALLOC(size * sizeof(int));
    printf("address of 3 is %p\n",array[3]);
  for (i=0; i < size; i++) {
    array[3][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[3][i];
  } //for i
  printf("free 0\n");
  FREE(array[0]);
  printf("free 2\n");
  FREE(array[2]);

  size = 7;
  expected_sum += size * size;
  array[4] = (int *)MALLOC(size * sizeof(int));
    printf("address of 4 is %p\n",array[4]);
  for (i=0; i < size; i++) {
    array[4][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[4][i];
  } //for i

  size = 256;
  expected_sum += size * size;
  array[5] = (int *)MALLOC(size * sizeof(int));
  printf("address of 5 is %p\n",array[5]);
  for (i=0; i < size; i++) {
    array[5][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[5][i];
  } //for i

  printf("free 5\n");
  FREE(array[5]);
  printf("free 1\n");
  FREE(array[1]);
  printf("free 3\n");
  FREE(array[3]);

  size = 23;
  expected_sum += size * size;
  array[6] = (int *)MALLOC(size * sizeof(int));
  printf("the address opf 6 is %p\n",array[6]);
  for (i=0; i < size; i++) {
    array[6][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[6][i];
  } //for i

  size = 4;
  expected_sum += size * size;
  array[7] = (int *)MALLOC(size * sizeof(int));
  printf("the address pf 7 is %p\n",array[7]);
  for (i=0; i < size; i++) {
    array[7][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[7][i];
  } //for i

  FREE(array[4]);

  size = 10;
  expected_sum += size * size;
  array[8] = (int *)MALLOC(size * sizeof(int));
 printf("the address pf 8 is %p\n",array[8]);
  for (i=0; i < size; i++) {
    array[8][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[8][i];
  } //for i

  size = 32;
  expected_sum += size * size;
  array[9] = (int *)MALLOC(size * sizeof(int));
   printf("the address pf 9 is %p\n",array[9]);
  for (i=0; i < size; i++) {
    array[9][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[9][i];
  } //for i
  printf("free 6\n");
  FREE(array[6]);
  printf("free 7\n");
  FREE(array[7]);
  printf("free 8\n");
  FREE(array[8]);
  printf("free 9\n");
  FREE(array[9]);

  if (sum == expected_sum) {
    printf("Calculated expected value of %d\n", sum);
    printf("Test passed\n");
  } else {
    printf("Expected sum=%d but calculated %d\n", expected_sum, sum);
    printf("Test failed\n");
  } //else

  return 0;
}
