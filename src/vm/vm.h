

// void VM_init(VM *vm);
// void VM_load(VM *vm, char *arqBin);
// void VM_runInstr(VM *vm);

#pragma once
#include <stdint.h>

#define S_MEM 1024
#define SP 14
#define PC 15


class VM {
private:
  int32_t regs[16];
  uint8_t *mem;
  uint32_t readMem(uint32_t addr);
  bool writeMem(uint32_t addr, uint32_t value);

public:
  VM();
  ~VM();
  void load(char *arqBin);
  void runInstr();
};
