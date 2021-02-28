#include "translate_inst.h"
#include "jit.h"
#include "sim.h"

#define EMIT(...) (tr.getAsm().emit(__VA_ARGS__))
#define EMIT_MOV(DST, SRC) (TranslateMove(tr, (DST), (SRC)))
#define LABEL(name) (tr.getAsm().bind(name))
#define TMP1 (tr.getTmp1())
#define TMP2 (tr.getTmp2())
#define RS1 (tr.getReg(inst->getRs1()))
#define RS2 (tr.getReg(inst->getRs2()))
#define RD (tr.getReg(inst->getRd()))
#define UIMM (inst->getImm())
#define IMM ((int32_t)inst->getImm())
#define PC (tr.getPc())
#define MEM (tr.getMem())
#define OFFSET (tr.getOffset())
#define END_TRACE()

using namespace asmjit;

// Helper functions for common translation templates

void TranslateMove(const jit::Translator &tr, const Operand &dst, const Operand &src) {
  if (dst.isReg() && src == Imm(0))
    EMIT(x86::Inst::kIdXor, dst, dst);
  else
    EMIT(x86::Inst::kIdMov, dst, src);
}

template <typename T>
void TranslateMMULoad(const jit::Translator &tr, const ir::Inst *inst) {
    tr.saveAllRegs();
    EMIT(x86::Inst::kIdMov, x86::edi, RS1);
    EMIT(x86::Inst::kIdAdd, x86::edi, Imm(IMM));
    EMIT(x86::Inst::kIdCall, reinterpret_cast<uint64_t>(&sim::State::read<T>));
    tr.restoreAllRegs();
}

template <typename T>
void TranslateMMUStore(const jit::Translator &tr, const ir::Inst *inst) {
    tr.saveAllRegs();
    EMIT(x86::Inst::kIdMov, x86::edi, RS1);
    EMIT(x86::Inst::kIdAdd, x86::edi, Imm(IMM));
    EMIT(x86::Inst::kIdMov, x86::esi, RS2);
    EMIT(x86::Inst::kIdCall, reinterpret_cast<uint64_t>(&sim::State::write<T>));
    tr.restoreAllRegs();
}

void TranslateCondBranch(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id cmp_inst) {
  Operand FST = RS1;
  if (RS1.isMem() || RS1.isImm()) {
    EMIT_MOV(TMP1, RS1);
    FST = TMP1;
  }
  EMIT(x86::Inst::kIdCmp, FST, RS2);
  auto taken_offset = OFFSET + IMM * 2;
  if (taken_offset) {
    Label taken = tr.getAsm().newLabel();
    Label exit = tr.getAsm().newLabel();
    EMIT(cmp_inst, taken);
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + 4));
    EMIT(x86::Inst::kIdJmp, exit);
    tr.getAsm().bind(taken);
    EMIT(x86::Inst::kIdAdd, PC, Imm(taken_offset));
    tr.getAsm().bind(exit);
  } else {
    tr.deallocateAllRegs();
    EMIT(cmp_inst, tr.getFunctionStart());
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + 4));
  }
  END_TRACE();
}

void TranslateImmType(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id opcode_inst) {
  if (!inst->getRd())
    return;
  if (RD != RS1) {
    if (RD.isReg() || RS1.isReg())
      EMIT_MOV(RD, RS1);
    else {
      EMIT_MOV(TMP1, RS1);
      EMIT_MOV(RD, TMP1);
    }
  }
  EMIT(opcode_inst, RD, Imm(IMM));
}

void TranslateRegType(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id opcode_inst) {
  if (!inst->getRd())
    return;
  if (RD == RS1) {
    if (RD.isReg() || RS2.isReg())
      EMIT(opcode_inst, RD, RS2);
    else {
      EMIT_MOV(TMP1, RS2);
      EMIT(opcode_inst, RD, TMP1);
    }
  }
  else if (RD != RS2 && (RD.isReg() || (!RS1.isMem()))) {
    EMIT_MOV(RD, RS1);
    EMIT(opcode_inst, RD, RS2);
  }
  else {
    EMIT_MOV(TMP1, RS1);
    EMIT(opcode_inst, TMP1, RS2);
    EMIT_MOV(RD, TMP1);
  }
}

void TranslateShift(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id shift_inst) {
  if (!inst->getRd())
    return;
  Operand DST = RD.isMem() ? TMP1 : RD;
  if (DST != RS1)
    EMIT_MOV(DST, RS1);
  if (RD.isMem())
    EMIT_MOV(RD, TMP1);
  if (!inst->getRs2())
    return;
  EMIT_MOV(TMP1, x86::ecx);
  EMIT_MOV(x86::ecx, RS2);
  EMIT(shift_inst, RD, x86::cl);
  EMIT_MOV(x86::ecx, TMP1);
}
// end of helper functions

void TranslateLB(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMULoad<int8_t>(tr, inst);
    EMIT(x86::Inst::kIdMovsx, RD, x86::al);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RD.isReg())
    EMIT(x86::Inst::kIdMovsx, RD, x86::ptr_8(TMP1.r64(), IMM));
  else {
    EMIT(x86::Inst::kIdMovsx, TMP2, x86::ptr_8(TMP1.r64(), IMM));
    EMIT_MOV(RD, TMP2);
  }
}

void TranslateLH(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMULoad<int16_t>(tr, inst);
    EMIT(x86::Inst::kIdMovsx, RD, x86::ax);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RD.isReg())
    EMIT(x86::Inst::kIdMovsx, RD, x86::ptr_16(TMP1.r64(), IMM));
  else {
    EMIT(x86::Inst::kIdMovsx, TMP2, x86::ptr_16(TMP1.r64(), IMM));
    EMIT_MOV(RD, TMP2);
  }
}

void TranslateLW(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMULoad<int32_t>(tr, inst);
    EMIT_MOV(RD, x86::eax);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RD.isReg())
    EMIT_MOV(RD, x86::ptr_32(TMP1.r64(), IMM));
  else {
    EMIT_MOV(TMP2, x86::ptr_32(TMP1.r64(), IMM));
    EMIT_MOV(RD, TMP2);
  }
}

void TranslateLBU(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMULoad<uint8_t>(tr, inst);
    EMIT_MOV(RD, x86::al);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RD.isReg())
    EMIT(x86::Inst::kIdMovzx, RD, x86::ptr_8(TMP1.r64(), IMM));
  else {
    EMIT(x86::Inst::kIdMovzx, TMP2, x86::ptr_8(TMP1.r64(), IMM));
    EMIT_MOV(RD, TMP2);
  }
}

void TranslateLHU(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMULoad<uint16_t>(tr, inst);
    EMIT_MOV(RD, x86::ax);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RD.isReg())
    EMIT(x86::Inst::kIdMovzx, RD, x86::ptr_16(TMP1.r64(), IMM));
  else {
    EMIT(x86::Inst::kIdMovzx, TMP2, x86::ptr_16(TMP1.r64(), IMM));
    EMIT_MOV(RD, TMP2);
  }
}

void TranslateLWU(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMULoad<uint32_t>(tr, inst);
    EMIT_MOV(RD, x86::eax);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RD.isReg())
    EMIT_MOV(RD, x86::ptr_32(TMP1.r64(), IMM));
  else {
    EMIT_MOV(TMP2, x86::ptr_32(TMP1.r64(), IMM));
    EMIT_MOV(RD, TMP2);
  }
}

void TranslateSB(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMUStore<uint8_t>(tr, inst);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RS2.isReg())
    EMIT_MOV(x86::ptr_8(TMP1.r64(), IMM), RS2.as<x86::Gp>().r8());
  else {
    EMIT_MOV(TMP2, RS2);
    EMIT_MOV(x86::ptr_8(TMP1.r64(), IMM), TMP2.r8());
  }
}

void TranslateSH(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMUStore<uint16_t>(tr, inst);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RS2.isReg())
    EMIT_MOV(x86::ptr_16(TMP1.r64(), IMM), RS2.as<x86::Gp>().r16());
  else {
    EMIT_MOV(TMP2, RS2);
    EMIT_MOV(x86::ptr_16(TMP1.r64(), IMM), TMP2.r16());
  }
}

void TranslateSW(const jit::Translator &tr, const ir::Inst *inst) {
  if (sim::MMU::isVirtualAddressing()) {
    TranslateMMUStore<uint32_t>(tr, inst);
    return;
  }
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1.r64(), MEM);
  if (RS2.isReg())
    EMIT_MOV(x86::ptr_32(TMP1.r64(), IMM), RS2);
  else {
    EMIT_MOV(TMP2, RS2);
    EMIT_MOV(x86::ptr_32(TMP1.r64(), IMM), TMP2);
  }
}

void TranslateLUI(const jit::Translator &tr, const ir::Inst *inst) { EMIT_MOV(RD, Imm(UIMM)); }

void TranslateAUIPC(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT_MOV(TMP1, PC);
  EMIT(x86::Inst::kIdAdd, TMP1, Imm(OFFSET + UIMM));
  EMIT_MOV(RD, TMP1);
}

void TranslateJAL(const jit::Translator &tr, const ir::Inst *inst) {
  if (inst->getRd()) {
    EMIT_MOV(TMP1, PC);
    EMIT_MOV(RD, TMP1);
    EMIT(x86::Inst::kIdAdd, RD, Imm(OFFSET + 4));
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + IMM * 2));
  } else {
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + IMM * 2));
  }
  END_TRACE();
}

void TranslateJALR(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + 4));
  EMIT_MOV(TMP1, PC);
  EMIT_MOV(RD, TMP1);
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdAdd, TMP1, Imm(IMM));
  EMIT(x86::Inst::kIdAnd, TMP1, Imm(~1u));
  EMIT_MOV(PC, TMP1);
  END_TRACE();
}

void TranslateBEQ(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateCondBranch(tr, inst, x86::Inst::kIdJe);
}

void TranslateBNE(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateCondBranch(tr, inst, x86::Inst::kIdJne);
}

void TranslateBLT(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateCondBranch(tr, inst, x86::Inst::kIdJl);
}

void TranslateBGE(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateCondBranch(tr, inst, x86::Inst::kIdJge);
}

void TranslateBLTU(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateCondBranch(tr, inst, x86::Inst::kIdJb);
}

void TranslateBGEU(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateCondBranch(tr, inst, x86::Inst::kIdJae);
}

void TranslateADDI(const jit::Translator &tr, const ir::Inst *inst) {
  if (!inst->getRs1())
    EMIT_MOV(RD, Imm(IMM));
  else if (!IMM) {
    if (RD.isReg() || RS1.isReg())
      EMIT_MOV(RD, RS1);
    else {
      EMIT_MOV(TMP1, RS1);
      EMIT_MOV(RD, TMP1);
    }
  }
  else
  TranslateImmType(tr, inst, x86::Inst::kIdAdd);
}

void TranslateSLTI(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT(x86::Inst::kIdCmp, RS1, Imm(IMM));
  EMIT(x86::Inst::kIdMov, RD, Imm(0));
  EMIT(x86::Inst::kIdSetl, RD);
}

void TranslateSLTIU(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT(x86::Inst::kIdCmp, RS1, Imm(UIMM));
  EMIT(x86::Inst::kIdMov, RD, Imm(0));
  EMIT(x86::Inst::kIdSetb, RD);
}

void TranslateXORI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, x86::Inst::kIdXor);
}

void TranslateORI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, x86::Inst::kIdOr);
}

void TranslateANDI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, x86::Inst::kIdAnd);
}

void TranslateSLLI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, x86::Inst::kIdShl);
}

void TranslateSRLI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, x86::Inst::kIdShr);
}

void TranslateSRAI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, x86::Inst::kIdSar);
}

void TranslateADD(const jit::Translator &tr, const ir::Inst *inst) {
  if (!inst->getRs1() || !inst->getRs2()) {
    auto SRC = inst->getRs1() ? RS1 : RS2;
    if (RD.isReg() || SRC.isReg())
      EMIT_MOV(RD, SRC);
    else {
      EMIT_MOV(TMP1, SRC);
      EMIT_MOV(RD, TMP1);
    }
  }
  TranslateRegType(tr, inst, x86::Inst::kIdAdd);
}

void TranslateSUB(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, x86::Inst::kIdSub);
}

void TranslateSLL(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateShift(tr, inst, x86::Inst::kIdShl);
}

void TranslateSLT(const jit::Translator &tr, const ir::Inst *inst) {
  Operand FST = RS1;
  if (RS1.isMem() || RS1.isImm()) {
    EMIT_MOV(TMP1, RS1);
    FST = TMP1;
  }
  EMIT(x86::Inst::kIdCmp, FST, RS2);
  EMIT(x86::Inst::kIdMov, RD, Imm(0));
  EMIT(x86::Inst::kIdSetl, RD);
}

void TranslateSLTU(const jit::Translator &tr, const ir::Inst *inst) {
  Operand FST = RS1;
  if (RS1.isMem() || RS1.isImm()) {
    EMIT_MOV(TMP1, RS1);
    FST = TMP1;
  }
  EMIT(x86::Inst::kIdCmp, FST, RS2);
  EMIT(x86::Inst::kIdMov, RD, Imm(0));
  EMIT(x86::Inst::kIdSetb, RD);
}

void TranslateXOR(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, x86::Inst::kIdXor);
}

void TranslateSRL(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateShift(tr, inst, x86::Inst::kIdShr);
}

void TranslateSRA(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateShift(tr, inst, x86::Inst::kIdSar);
}

void TranslateOR(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, x86::Inst::kIdOr);
}

void TranslateAND(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, x86::Inst::kIdAnd);
}

// RV32M standard extension
void TranslateMUL(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdMul, RS2);
  EMIT_MOV(RD, TMP1);
}

void TranslateMULH(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT_MOV(TMP1, RS1);
  EMIT(x86::Inst::kIdMul, RS2);
  EMIT_MOV(RD, TMP2);
}
