#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <NDL.h>
#include <BMP.h>

int main() {
  // printf("Test ends! Spinning...\n");
  NDL_Init(0);
  int w = 800, h = 600;
  void *bmp = BMP_Load("/share/pictures/projectn.bmp", &w, &h);
  printf("1\n");
  assert(bmp);
  NDL_OpenCanvas(&w, &h);
  printf("2\n");
  NDL_DrawRect(bmp, 0, 0, w, h);
  printf("3\n");
  free(bmp);
  NDL_Quit();
  printf("4\n");
  while (1);
  return 0;
}
