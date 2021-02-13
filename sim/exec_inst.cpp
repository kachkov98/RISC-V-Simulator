#include "exec_inst.h"
#include "common.h"
#include "sim.h"
#include "syscall.h"
#include <cstdint>
#include <cstdio>

#define URS1 (state->getReg(cur_inst->getRs1()))
#define RS1 ((int32_t)state->getReg(cur_inst->getRs1()))
#define URS2 (state->getReg(cur_inst->getRs2()))
#define RS2 ((int32_t)state->getReg(cur_inst->getRs2()))
#define UIMM (cur_inst->getImm())
#define IMM ((int32_t)cur_inst->getImm())
#define GET_PC() (state->getPC())
#define SET_PC(x) (state->setPC(x))
#define RD(x) (state->setReg(cur_inst->getRd(), (x)))
#define OFFSET ((cur_inst - fst_inst) * 4)
#define NEXT_INST()                                                                                \
  {                                                                                                \
    if (options::execution_log)                                                                    \
      cur_inst->dump(options::log);                                                                \
    return (cur_inst + 1)->exec(fst_inst, state);                                                  \
  }
#define END_TRACE()                                                                                \
  {                                                                                                \
    if (options::execution_log)                                                                    \
      cur_inst->dump(options::log);                                                                \
  }

void ExecDummy(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  fprintf(options::log, "This instruction is not implemented yet\n");
  NEXT_INST();
}

void ExecECALL([[maybe_unused]] const ir::Inst *fst_inst, [[maybe_unused]] const ir::Inst *cur_inst,
               sim::State *state) {
  syscall::ExecSysCall(state, static_cast<syscall::SysCall>(state->getReg(isa::Regs::a7)));
  NEXT_INST();
}

void ExecFENCE(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  sim::State::flush();
  SET_PC(GET_PC() + OFFSET + 4);
  END_TRACE();
}

void ExecLB(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(state->read<int8_t>(RS1 + IMM));
  NEXT_INST();
}

void ExecLH(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(state->read<int16_t>(RS1 + IMM));
  NEXT_INST();
}

void ExecLW(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(state->read<int32_t>(RS1 + IMM));
  NEXT_INST();
}

void ExecLBU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(state->read<uint8_t>(RS1 + IMM));
  NEXT_INST();
}

void ExecLHU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(state->read<uint16_t>(RS1 + IMM));
  NEXT_INST();
}

void ExecLWU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(state->read<uint32_t>(RS1 + IMM));
  NEXT_INST();
}

void ExecSB(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  state->write<uint8_t>(RS1 + IMM, RS2);
  NEXT_INST();
}

void ExecSH(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  state->write<uint16_t>(RS1 + IMM, RS2);
  NEXT_INST();
}

void ExecSW(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  state->write<uint32_t>(RS1 + IMM, RS2);
  NEXT_INST();
}

void ExecLUI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(UIMM);
  NEXT_INST();
}

void ExecAUIPC(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(GET_PC() + OFFSET + UIMM);
  NEXT_INST();
}

void ExecJAL(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t cur_pc = GET_PC() + OFFSET;
  RD(cur_pc + 4);
  SET_PC(cur_pc + IMM * 2);
  END_TRACE();
}

void ExecJALR(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t cur_pc = GET_PC() + OFFSET;
  RD(cur_pc + 4);
  uint32_t offset = RS1 + IMM;
  SET_PC(offset & ~1u);
  END_TRACE();
}

void ExecBEQ(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t cur_pc = GET_PC() + OFFSET;
  SET_PC(RS1 == RS2 ? cur_pc + IMM * 2 : cur_pc + 4);
  END_TRACE();
}

void ExecBNE(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t cur_pc = GET_PC() + OFFSET;
  SET_PC(RS1 != RS2 ? cur_pc + IMM * 2 : cur_pc + 4);
  END_TRACE();
}

void ExecBLT(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t cur_pc = GET_PC() + OFFSET;
  SET_PC(RS1 < RS2 ? cur_pc + IMM * 2 : cur_pc + 4);
  END_TRACE();
}

void ExecBGE(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t cur_pc = GET_PC() + OFFSET;
  SET_PC(RS1 >= RS2 ? cur_pc + IMM * 2 : cur_pc + 4);
  END_TRACE();
}

void ExecBLTU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t cur_pc = GET_PC() + OFFSET;
  SET_PC(URS1 < URS2 ? cur_pc + IMM * 2 : cur_pc + 4);
  END_TRACE();
}

void ExecBGEU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t cur_pc = GET_PC() + OFFSET;
  SET_PC(URS1 >= URS2 ? cur_pc + IMM * 2 : cur_pc + 4);
  END_TRACE();
}

void ExecADDI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 + IMM);
  NEXT_INST();
}

void ExecSLTI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 < IMM ? 1 : 0);
  NEXT_INST();
}

void ExecSLTIU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS1 < UIMM ? 1 : 0);
  NEXT_INST();
}

void ExecXORI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 ^ IMM);
  NEXT_INST();
}

void ExecORI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 | IMM);
  NEXT_INST();
}

void ExecANDI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 & IMM);
  NEXT_INST();
}

void ExecSLLI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS1 << UIMM);
  NEXT_INST();
}

void ExecSRLI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS1 >> UIMM);
  NEXT_INST();
}

void ExecSRAI(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint32_t mask = (RS1 < 0 && UIMM > 0) ? ~(~0u >> UIMM) : 0;
  RD((URS1 >> UIMM) | mask);
  NEXT_INST();
}

void ExecADD(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 + RS2);
  NEXT_INST();
}

void ExecSUB(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 - RS2);
  NEXT_INST();
}

void ExecSLL(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS1 << (URS2 & 0b11111));
  NEXT_INST();
}

void ExecSLT(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 < RS2 ? 1 : 0);
  NEXT_INST();
}

void ExecSLTU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS1 < URS2 ? 1 : 0);
  NEXT_INST();
}

void ExecXOR(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 ^ RS2);
  NEXT_INST();
}

void ExecSRL(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS1 >> (URS2 & 0b11111));
  NEXT_INST();
}

void ExecSRA(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint8_t offset = URS2 & 0b11111;
  uint32_t mask = (RS1 < 0 && offset > 0) ? ~(~0u >> offset) : 0;
  RD((URS1 >> offset) | mask);
  NEXT_INST();
}

void ExecOR(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 | RS2);
  NEXT_INST();
}

void ExecAND(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(RS1 & RS2);
  NEXT_INST();
}

void ExecCSRRW(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  if (IMM != 0x180)
    throw SimException("Not satp sysreg is not supported");
  sim::MMU::satp |= RS1 & 0x80000000ul;
  NEXT_INST();
}

// RV32M standard extension
void ExecMUL(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS1 * URS2);
  NEXT_INST();
}

void ExecMULH(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint64_t result = (int64_t)RS1 * (int64_t)RS2;
  RD(result >> 32);
  NEXT_INST();
}

void ExecMULHSU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint64_t result = (int64_t)RS1 * (uint64_t)URS2;
  RD(result >> 32);
  NEXT_INST();
}

void ExecMULHU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  uint64_t result = (uint64_t)URS1 * (uint64_t)URS2;
  RD(result >> 32);
  NEXT_INST();
}

void ExecDIV(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  if (RS2 == 0)
    RD(-1);
  else if (RS2 == -1 && RS1 == INT32_MIN)
    RD(INT32_MIN);
  else
    RD(RS1 / RS2);
  NEXT_INST();
}

void ExecDIVU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS2 ? URS1 / URS2 : UINT32_MAX);
  NEXT_INST();
}

void ExecREM(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  if (RS2 == 0)
    RD(RS1);
  else if (RS2 == -1 && RS1 == INT32_MIN)
    RD(0);
  else
    RD(RS1 % RS2);
  NEXT_INST();
}

void ExecREMU(const ir::Inst *fst_inst, const ir::Inst *cur_inst, sim::State *state) {
  RD(URS2 ? URS1 % URS2 : URS1);
  NEXT_INST();
}
