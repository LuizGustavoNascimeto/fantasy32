

// void VM_init(VM *vm);
// void VM_load(VM *vm, char *arqBin);
// void VM_runInstr(VM *vm);

#pragma once
#include <stdint.h>

#define S_MEM 1024
#define SP 14
#define PC 15

enum Opcode {
  ADD = 0x00,
  SUB = 0x01,
  MUL = 0x02,
  DIV = 0x03,
  MOD = 0x04,
  AND = 0x05,
  OR = 0x06,
  XOR = 0x07,
  SHL = 0x08,
  SHR = 0x09,
  ROL = 0x0A,
  ROR = 0x0B,
  ADDI = 0x0C,
  MOVL = 0x0D,
  MOVH = 0x0E,
  LOAD = 0x0F,
  STORE = 0x10,
  BEQ = 0x11,
  BNE = 0x12,
  BLT = 0x13,
  BGT = 0x14,
  BLE = 0x15,
  BGE = 0x16,
  JMP = 0x17,
  CALL = 0x18,
  PUSH = 0x19,
  POP = 0x1A,
  INC = 0x1B,
  DEC = 0x1C,
  NOT = 0x1D,
  RET = 0x1E,
  RECT = 0x1F,
  DSPRITE = 0x20,
  CLEAR = 0x21,
  GKEY = 0x22,
  PLAY = 0x23,
  SLEEP = 0x24,
  PSTR = 0x25,
  PINT = 0x26,
  SYSCALL = 0x27,
  SRAND = 0x28,
  RAND = 0x29,
  FRAMENUM = 0x2A,
  HALT = 0x2B,
};

class VM {
private:
  int32_t regs[16];
  uint8_t *mem;
  uint32_t readMem(uint32_t addr);

public:
  VM();
  ~VM();
  void load(char *arqBin);
  void runInstr();
};
