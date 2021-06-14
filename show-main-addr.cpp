#include <stdio.h>
int main(int argc, char* argv[]) {
  printf("main():%p\n", main);
  printf("argv:%p\n", &argv);
  printf("argc:%p\n", &argc);
  return 0;
}
