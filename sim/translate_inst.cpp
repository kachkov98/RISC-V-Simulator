#include "translate_inst.h"
#include "jit.h"

#define EMIT(...) (tr.getAsm().emit(__VA_ARGS__))
#define EMIT_MOV(DST, SRC) (TranslateMove(tr, (DST), (SRC)))
#define LABEL(name) (tr.getAsm().bind(name))
#define TMP (tr.getTmp())
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

void TranslateLoad(const jit::Translator &tr, const ir::Inst *inst, uint8_t num_bytes) {
#if 0
    tr.saveAllRegs();
    EMIT(x86::Inst::kIdMov, x86::esi, RS1);
    EMIT(x86::Inst::kIdAdd, x86::esi, Imm(IMM));
    EMIT(x86::Inst::kIdMov, x86::edx, Imm(num_bytes));
    EMIT(x86::Inst::kIdPush, x86::rdi);
    EMIT(x86::Inst::kIdLea, x86::rdi, tr.getMMU());
    EMIT(x86::Inst::kIdCall, tr.getLoadFunc());
    EMIT(x86::Inst::kIdPop, x86::rdi);
    tr.restoreAllRegs();
#else
  EMIT_MOV(TMP, RS1);
  EMIT(x86::Inst::kIdAdd, x86::rsi, MEM);
  if (num_bytes == 1)
    EMIT_MOV(x86::al, x86::ptr_8(x86::rsi, IMM));
  else if (num_bytes == 2)
    EMIT_MOV(x86::ax, x86::ptr_16(x86::rsi, IMM));
  else
    EMIT_MOV(x86::eax, x86::ptr_32(x86::rsi, IMM));
#endif
}

void TranslateStore(const jit::Translator &tr, const ir::Inst *inst, uint8_t num_bytes) {
#if 0
    tr.saveAllRegs();
    EMIT(x86::Inst::kIdMov, x86::esi, RS1);
    EMIT(x86::Inst::kIdAdd, x86::esi, Imm(IMM));
    EMIT(x86::Inst::kIdMov, x86::edx, Imm(num_bytes));
    EMIT(x86::Inst::kIdMov, x86::ecx, RS2);
    EMIT(x86::Inst::kIdPush, x86::rdi);
    EMIT(x86::Inst::kIdLea, x86::rdi, tr.getMMU());
    EMIT(x86::Inst::kIdCall, tr.getStoreFunc());
    EMIT(x86::Inst::kIdPop, x86::rdi);
    tr.restoreAllRegs();
#else
  EMIT_MOV(TMP, RS1);
  EMIT(x86::Inst::kIdAdd, x86::rsi, MEM);
  EMIT_MOV(x86::eax, RS2);
  if (num_bytes == 1)
    EMIT_MOV(x86::ptr_8(x86::rsi, IMM), x86::al);
  else if (num_bytes == 2)
    EMIT_MOV(x86::ptr_16(x86::rsi, IMM), x86::ax);
  else
    EMIT_MOV(x86::ptr_32(x86::rsi, IMM), x86::eax);
#endif
}

void TranslateCondBranch(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id cmp_inst) {
  Operand FST = RS1;
  if (RS1.isMem() || RS1.isImm()) {
    EMIT_MOV(TMP, RS1);
    FST = TMP;
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
  }
  else {
    tr.deallocateAllRegs();
    EMIT(cmp_inst, tr.getFunctionStart());
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + 4));
  }
  END_TRACE();
}

void TranslateImmType(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id opcode_inst) {
  if (!inst->getRd())
    return;
  Operand DST = RD.isMem() ? TMP : RD;
  if (DST != RS1)
    EMIT_MOV(DST, RS1);
  if (opcode_inst != x86::Inst::kIdMov)
    EMIT(opcode_inst, DST, Imm(IMM));
  if (RD.isMem())
    EMIT_MOV(RD, TMP);
}

void TranslateRegType(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id opcode_inst) {
  if (!inst->getRd())
    return;
#if 0
    Operand DST = RD.isMem() ? TMP : RD;
    if (DST != RS1)
        EMIT(x86::Inst::kIdMov, DST, RS1);
    if (opcode_inst != x86::Inst::kIdMov)
        EMIT(opcode_inst, DST, RS2);
    if (RD.isMem())
        EMIT(x86::Inst::kIdMov, RD, TMP);
#endif
  EMIT_MOV(TMP, RS1);
  if (opcode_inst != x86::Inst::kIdMov)
    EMIT(opcode_inst, TMP, RS2);
  EMIT_MOV(RD, TMP);
}

void TranslateShift(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id shift_inst) {
  if (!inst->getRd())
    return;
  Operand DST = RD.isMem() ? TMP : RD;
  if (DST != RS1)
    EMIT_MOV(DST, RS1);
  if (RD.isMem())
    EMIT_MOV(RD, TMP);
  if (!inst->getRs2())
    return;
  EMIT_MOV(TMP, x86::ecx);
  EMIT_MOV(x86::ecx, RS2);
  EMIT(shift_inst, RD, x86::cl);
  EMIT_MOV(x86::ecx, TMP);
}
// end of helper functions

void TranslateLB(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateLoad(tr, inst, 1);
  EMIT(x86::Inst::kIdMovsx, TMP, x86::al);
  EMIT_MOV(RD, TMP);
}

void TranslateLH(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateLoad(tr, inst, 2);
  EMIT(x86::Inst::kIdMovsx, TMP, x86::ax);
  EMIT_MOV(RD, TMP);
}

void TranslateLW(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateLoad(tr, inst, 4);
  EMIT_MOV(RD, x86::eax);
}

void TranslateLBU(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateLoad(tr, inst, 1);
  EMIT(x86::Inst::kIdMovzx, TMP, x86::al);
  EMIT_MOV(RD, TMP);
}

void TranslateLHU(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateLoad(tr, inst, 2);
  EMIT(x86::Inst::kIdMovzx, TMP, x86::ax);
  EMIT_MOV(RD, TMP);
}

void TranslateLWU(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateLoad(tr, inst, 4);
  EMIT_MOV(RD, x86::eax);
}

void TranslateSB(const jit::Translator &tr, const ir::Inst *inst) { TranslateStore(tr, inst, 1); }

void TranslateSH(const jit::Translator &tr, const ir::Inst *inst) { TranslateStore(tr, inst, 2); }

void TranslateSW(const jit::Translator &tr, const ir::Inst *inst) { TranslateStore(tr, inst, 4); }

void TranslateLUI(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT_MOV(RD, Imm(UIMM));
}

void TranslateAUIPC(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT_MOV(TMP, PC);
  EMIT(x86::Inst::kIdAdd, TMP, Imm(OFFSET + UIMM));
  EMIT_MOV(RD, TMP);
}

void TranslateJAL(const jit::Translator &tr, const ir::Inst *inst) {
  if (inst->getRd()) {
    EMIT_MOV(TMP, PC);
    EMIT_MOV(RD, TMP);
    EMIT(x86::Inst::kIdAdd, RD, Imm(OFFSET + 4));
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + IMM * 2));
  } else {
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + IMM * 2));
  }
  END_TRACE();
}

void TranslateJALR(const jit::Translator &tr, const ir::Inst *inst) {
  EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + 4));
  EMIT_MOV(TMP, PC);
  EMIT_MOV(RD, TMP);
  EMIT_MOV(TMP, RS1);
  EMIT(x86::Inst::kIdAdd, TMP, Imm(IMM));
  EMIT(x86::Inst::kIdAnd, TMP, Imm(~1u));
  EMIT_MOV(PC, TMP);
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
  TranslateImmType(tr, inst, IMM ? x86::Inst::kIdAdd : x86::Inst::kIdMov);
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
  TranslateImmType(tr, inst, IMM ? x86::Inst::kIdXor : x86::Inst::kIdMov);
}

void TranslateORI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, IMM ? x86::Inst::kIdOr : x86::Inst::kIdMov);
}

void TranslateANDI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, x86::Inst::kIdAnd);
}

void TranslateSLLI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, UIMM ? x86::Inst::kIdShl : x86::Inst::kIdMov);
}

void TranslateSRLI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, UIMM ? x86::Inst::kIdShr : x86::Inst::kIdMov);
}

void TranslateSRAI(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateImmType(tr, inst, UIMM ? x86::Inst::kIdSar : x86::Inst::kIdMov);
}

void TranslateADD(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, inst->getRd() ? x86::Inst::kIdAdd : x86::Inst::kIdMov);
}

void TranslateSUB(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, inst->getRd() ? x86::Inst::kIdSub : x86::Inst::kIdMov);
}

void TranslateSLL(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateShift(tr, inst, x86::Inst::kIdShl);
}

void TranslateSLT(const jit::Translator &tr, const ir::Inst *inst) {
  Operand FST = RS1;
  if (RS1.isMem() || RS1.isImm()) {
    EMIT_MOV(TMP, RS1);
    FST = TMP;
  }
  EMIT(x86::Inst::kIdCmp, FST, RS2);
  EMIT(x86::Inst::kIdMov, RD, Imm(0));
  EMIT(x86::Inst::kIdSetl, RD);
}

void TranslateSLTU(const jit::Translator &tr, const ir::Inst *inst) {
  Operand FST = RS1;
  if (RS1.isMem() || RS1.isImm()) {
    EMIT_MOV(TMP, RS1);
    FST = TMP;
  }
  EMIT(x86::Inst::kIdCmp, FST, RS2);
  EMIT(x86::Inst::kIdMov, RD, Imm(0));
  EMIT(x86::Inst::kIdSetb, RD);
}

void TranslateXOR(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, inst->getRd() ? x86::Inst::kIdXor : x86::Inst::kIdMov);
}

void TranslateSRL(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateShift(tr, inst, x86::Inst::kIdShr);
}

void TranslateSRA(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateShift(tr, inst, x86::Inst::kIdSar);
}

void TranslateOR(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, inst->getRd() ? x86::Inst::kIdOr : x86::Inst::kIdMov);
}

void TranslateAND(const jit::Translator &tr, const ir::Inst *inst) {
  TranslateRegType(tr, inst, x86::Inst::kIdAnd);
}
