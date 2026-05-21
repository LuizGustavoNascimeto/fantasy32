#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void hotReload() {
  while (1) {
    sleep(999);
  }
}
int main() {
  VM vm;

  printf("testando o hot reload\n");

  hotReload();
  return 0;
}
