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

bool VM::writeMem(uint32_t addr, uint32_t value) {
  if (addr >= S_MEM) {
    return false;
  }
  this->mem[addr] = (value >> 24) & 0xFF;
  this->mem[addr + 1] = (value >> 16) & 0xFF;
  this->mem[addr + 2] = (value >> 8) & 0xFF;
  this->mem[addr + 3] = value & 0xFF;
  return true;
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
  int32_t offset = (int16_t)(i_imm & 0xFFFF); // Used for branch instructions, allowing negative values
  uint32_t target_addr26 = (instr & 0x03FFFFFF) << 2;
  uint32_t u_rd = (instr >> 22) & 0xF;

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
    case ADDI:  // Type I
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

    // BRANCH INSTRUCTIONS (type I)
    case BEQ:
      if (this->regs[i_rs] == regs[i_rt]) {
        this->regs[PC] += (offset * 4);
      }
      break;

    case BNE:
      if (this->regs[i_rs] != this->regs[i_rt]) {
        this->regs[PC] += (offset * 4);
      }
      break;

    case BLT:
      if (this->regs[i_rs] < this->regs[i_rt]) {
        this->regs[PC] += (offset * 4);
      }
      break;

    case BGT:
      if (this->regs[i_rs] > this->regs[i_rt]) {
        this->regs[PC] += (offset * 4);
      }
      break;

    case BLE:
      if (this->regs[i_rs] <= this->regs[i_rt]) {
        this->regs[PC] += (offset * 4);
      }
      break;

    case BGE:
      if (this->regs[i_rs] >= this->regs[i_rt]) {
        this->regs[PC] += (offset * 4);
      }
      break;

    // JUMP INSTRUCTIONS (type J)
    case JMP:
      if (!isInsideMem(target_addr26)) {
        printf("Invalid jump address: 0x%X\n", target_addr26);
        exit(1);
      }
      this->regs[PC] = target_addr26;
      break;

    case CALL:
      if (!isInsideMem(target_addr26)) {
        printf("Invalid call address: 0x%X\n", target_addr26);
        exit(1);
      }
      if (this->regs[SP] <= 0x0FFEFFF) {
        printf("Stack Overflow\n");
        exit(1);
      }
      this->regs[SP] -= 4;
      writeMem(this->regs[SP], this->regs[PC]);
      this->regs[PC] = target_addr26;
      break;

    // UNARY AND STACK INSTRUCTIONS (type U)
    case PUSH:
      if (this->regs[SP] <= 0x0FFEFFF) {
        printf("Stack Overflow\n");
        exit(1);
      }
      this->regs[SP] -= 4;
      writeMem(this->regs[SP], this->regs[u_rd]);
      break;

    case POP:
      if (this->regs[SP] >= 0x00FFFFFF) {
        printf("Stack Underflow\n");
        exit(1);
      }
      this->regs[u_rd] = readMem(this->regs[SP]);
      this->regs[SP] += 4;
      break;
    case INC:
      this->regs[u_rd]++;
      break;
    case DEC:
      this->regs[u_rd]--;
      break;
    case NOT:
      this->regs[u_rd] = ~this->regs[u_rd];
      break;
    case RET:
      if (this->regs[SP] >= 0x00FFFFFF) {
        printf("Stack Underflow\n");
        exit(1);
      }
      this->regs[PC] = readMem(this->regs[SP]);
      this->regs[SP] += 4;
      break;
    default:
      printf("Unknown opcode: 0x%X\n", opcode);
      exit(1);
  }
}
