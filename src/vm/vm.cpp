#include "vm.h"
#include "../util/util.cpp"
#include "opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VM::VM() {
  this->mem = new uint8_t[S_MEM];
  memset(this->mem, 0, S_MEM * sizeof(uint8_t));
  memset(this->regs, 0, 16 * sizeof(int32_t));
}

VM::~VM() { delete[] this->mem; }

uint32_t VM::readMem(uint32_t addr) {
  return (this->mem[addr] << 24) | (this->mem[addr + 1] << 16) |
         (this->mem[addr + 2] << 8) | this->mem[addr + 3];
}

void VM::load(char *arqBin) {
  FILE *bin = fopen(arqBin, "rb");
  fseek(bin, 0, SEEK_END);
  int binSize = ftell(bin) - sizeof(uint32_t);
  rewind(bin);

  uint32_t codeStart;
  fread(&codeStart, sizeof(uint32_t), 1, bin);
  codeStart = __builtin_bswap32(codeStart);
  fread(this->mem, 1, binSize, bin);
  this->regs[PC] = codeStart;

  fclose(bin);
}

void VM::runInstr() {
  int32_t pc = this->regs[PC];
  uint32_t instr = readMem(pc);
  uint32_t opcode = instr >> 26;

  uint32_t i_rs = (instr >> 22) & 0xF;
  uint32_t i_rt = (instr >> 18) & 0xF;
  uint32_t i_rd = (instr >> 14) & 0xF;
  uint32_t i_imm = instr & 0x3FFFF;

  this->regs[PC] += 4;

  switch (opcode) {
  // Arithmetic and logical instructions (type R, except for ADDI)
  case ADD:
    this->regs[i_rd] = this->regs[i_rs] + this->regs[i_rt];
    break;
  case SUB:
    this->regs[i_rd] = this->regs[i_rs] - this->regs[i_rt];
    break;
  case MUL:
    this->regs[i_rd] = this->regs[i_rs] * this->regs[i_rt];
    break;
  case DIV:
    this->regs[i_rd] = this->regs[i_rs] / this->regs[i_rt];
    break;
  case MOD:
    this->regs[i_rd] = this->regs[i_rs] % this->regs[i_rt];
    break;
  case AND:
    this->regs[i_rd] = this->regs[i_rs] & this->regs[i_rt];
    break;
  case OR:
    this->regs[i_rd] = this->regs[i_rs] | this->regs[i_rt];
    break;
  case XOR:
    this->regs[i_rd] = this->regs[i_rs] ^ this->regs[i_rt];
    break;
  case SHL:
    this->regs[i_rd] = this->regs[i_rs] << (this->regs[i_rt] & 0x1F);
    break;
  case SHR:
    this->regs[i_rd] = this->regs[i_rs] >> (this->regs[i_rt] & 0x1F);
    break;
  case ROL:
    this->regs[i_rd] = (this->regs[i_rs] << (this->regs[i_rt] & 0x1F)) |
                       (this->regs[i_rs] >> (32 - (this->regs[i_rt] & 0x1F)));
    break;
  case ROR:
    this->regs[i_rd] = (this->regs[i_rs] >> (this->regs[i_rt] & 0x1F)) |
                       (this->regs[i_rs] << (32 - (this->regs[i_rt] & 0x1F)));
    break;
  case ADDI: // Type I
    this->regs[i_rt] = this->regs[i_rs] + (i_imm & 0xFFFF);
    break;

  // Memory movement instructions (type I)
  case MOVL:
    this->regs[i_rt] = (i_imm & 0xFFFF);
    break;
  case MOVH:
    this->regs[i_rt] = i_rt | (i_imm << 16);
    break;
  case LOAD:
    this->regs[i_rt] = this->mem[i_rs + (i_imm * 4)];
    if (!isInsideMem) {
      printf("Memory access out of bounds: 0x%X\n", i_rs + (i_imm * 4));
      exit(1);
    }
    break;
  case STORE:
    this->mem[i_rs + (i_imm * 4)] = i_rt;
    if (!isInsideMem) {
      printf("Memory access out of bounds: 0x%X\n", i_rs + (i_imm * 4));
      exit(1);
    }
    break;
  default:
    printf("Unknown opcode: 0x%X\n", opcode);
    exit(1);
  }
}
