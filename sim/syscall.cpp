#include "syscall.h"
#include "sim.h"
#include <cstdio>

namespace syscall {
void ExecClose(sim::State *state) {
  printf("Close syscall!\n");
  state->setReg(isa::Regs::a0, 0);
}

void ExecRead(sim::State *state) {
  const uint32_t a0 = state->getReg(isa::Regs::a0);
  const uint32_t a1 = state->getReg(isa::Regs::a1);
  const uint32_t a2 = state->getReg(isa::Regs::a2);
  for (uint32_t i = 0; i < a2; ++i) {
    int res = fgetc(stdin);
    if (res != EOF)
      state->write<char>(a1 + i, res);
    else {
      state->setReg(isa::Regs::a0, i);
      return;
    }
  }
  state->setReg(isa::Regs::a0, a2);
}

void ExecWrite(sim::State *state) {
  const uint32_t a0 = state->getReg(isa::Regs::a0);
  const uint32_t a1 = state->getReg(isa::Regs::a1);
  const uint32_t a2 = state->getReg(isa::Regs::a2);
  for (uint32_t i = 0; i < a2; ++i)
    fputc(state->read<char>(a1 + i), stdout);
  state->setReg(isa::Regs::a0, a2);
}

void ExecFstat(sim::State *state) {
  printf("Fstat syscall!\n");
  state->setReg(isa::Regs::a0, 0);
}

void ExecExit([[maybe_unused]] sim::State *state) { throw SimException("Successfully finished!\n"); }

void ExecBrk([[maybe_unused]] sim::State *state) {
  printf("Brk syscall!\n");
  state->setReg(isa::Regs::a0, 0);
}

void ExecSysCall(sim::State *state, SysCall value) {
  switch (value) {
  case SysCall::close:
    ExecClose(state);
    break;
  case SysCall::read:
    ExecRead(state);
    break;
  case SysCall::write:
    ExecWrite(state);
    break;
  case SysCall::fstat:
    ExecFstat(state);
    break;
  case SysCall::exit:
    ExecExit(state);
    break;
  case SysCall::brk:
    ExecBrk(state);
    break;
  default:
    printf("Unssupported syscall %u\n", static_cast<uint32_t>(value));
  }
}
} // namespace syscall
