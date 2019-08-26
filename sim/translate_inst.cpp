#include "translate_inst.h"
#include "jit.h"

#define EMIT(...) (tr.GetAsm().emit(__VA_ARGS__))
#define LABEL(name) (tr.GetAsm().bind(name))
#define TMP (tr.GetTmp())
#define RS1 (tr.GetReg(inst->GetRs1()))
#define RS2 (tr.GetReg(inst->GetRs2()))
#define RD (tr.GetReg(inst->GetRd()))
#define UIMM (inst->GetImm())
#define IMM ((int32_t)inst->GetImm())
#define PC (tr.GetPc())
#define OFFSET (tr.GetOffset())
#define END_TRACE()

using namespace asmjit;

// Helper functions for common translation templates
void TranslateLoad(const jit::Translator &tr, const ir::Inst *inst, uint8_t num_bytes)
{
    tr.SaveAllRegs();
    EMIT(x86::Inst::kIdMov, x86::esi, RS1);
    EMIT(x86::Inst::kIdAdd, x86::esi, Imm(IMM));
    EMIT(x86::Inst::kIdMov, x86::edx, Imm(num_bytes));
    EMIT(x86::Inst::kIdPush, x86::rdi);
    EMIT(x86::Inst::kIdLea, x86::rdi, tr.GetMMU());
    EMIT(x86::Inst::kIdCall, tr.GetLoadFunc());
    EMIT(x86::Inst::kIdPop, x86::rdi);
    tr.RestoreAllRegs();
}

void TranslateStore(const jit::Translator &tr, const ir::Inst *inst, uint8_t num_bytes)
{
    tr.SaveAllRegs();
    EMIT(x86::Inst::kIdMov, x86::esi, RS1);
    EMIT(x86::Inst::kIdAdd, x86::esi, Imm(IMM));
    EMIT(x86::Inst::kIdMov, x86::edx, Imm(num_bytes));
    EMIT(x86::Inst::kIdMov, x86::ecx, RS2);
    EMIT(x86::Inst::kIdPush, x86::rdi);
    EMIT(x86::Inst::kIdLea, x86::rdi, tr.GetMMU());
    EMIT(x86::Inst::kIdCall, tr.GetStoreFunc());
    EMIT(x86::Inst::kIdPop, x86::rdi);
    tr.RestoreAllRegs();
}

void TranslateCondBranch(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id cmp_inst)
{
    Label taken = tr.GetAsm().newLabel();
    Label exit = tr.GetAsm().newLabel();
    Operand FST = RS1;
    if (RS1.isMem())
    {
        EMIT(x86::Inst::kIdMov, TMP, RS1);
        FST = TMP;
    }
    EMIT(x86::Inst::kIdCmp, FST, RS2);
    EMIT(cmp_inst, taken);
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + 4));
    EMIT(x86::Inst::kIdJmp, exit);
    tr.GetAsm().bind(taken);
    EMIT(x86::Inst::kIdAdd, PC, Imm(OFFSET + IMM * 2));
    tr.GetAsm().bind(exit);
    END_TRACE();
}

void TranslateImmType(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id opcode_inst)
{
    if (!inst->GetRd())
        return;
    Operand DST = RD.isMem() ? TMP : RD;
    if (DST != RS1)
        EMIT(x86::Inst::kIdMov, DST, RS1);
    if (opcode_inst != x86::Inst::kIdMov)
        EMIT(opcode_inst, DST, Imm(IMM));
    if (RD.isMem())
        EMIT(x86::Inst::kIdMov, RD, TMP);
}

void TranslateRegType(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id opcode_inst)
{
    if (!inst->GetRd())
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
    EMIT(x86::Inst::kIdMov, TMP, RS1);
    if (opcode_inst != x86::Inst::kIdMov)
        EMIT(opcode_inst, TMP, RS2);
    EMIT(x86::Inst::kIdMov, RD, TMP);
}

void TranslateShift(const jit::Translator &tr, const ir::Inst *inst, x86::Inst::Id shift_inst)
{
    if (!inst->GetRd())
        return;
    Operand DST = RD.isMem() ? TMP : RD;
    if (DST != RS1)
        EMIT(x86::Inst::kIdMov, DST, RS1);
    if (RD.isMem())
        EMIT(x86::Inst::kIdMov, RD, TMP);
    if (!inst->GetRs2())
        return;
    EMIT(x86::Inst::kIdMov, TMP, x86::ecx);
    EMIT(x86::Inst::kIdMov, x86::ecx, RS2);
    EMIT(shift_inst, RD, x86::cl);
    EMIT(x86::Inst::kIdMov, x86::ecx, TMP);
}
// end of helper functions

void TranslateLB(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateLoad(tr, inst, 1);
    EMIT(x86::Inst::kIdMovsx, TMP, x86::al);
    EMIT(x86::Inst::kIdMov, RD, TMP);
}

void TranslateLH(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateLoad(tr, inst, 2);
    EMIT(x86::Inst::kIdMovsx, TMP, x86::ax);
    EMIT(x86::Inst::kIdMov, RD, TMP);
}

void TranslateLW(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateLoad(tr, inst, 4);
    EMIT(x86::Inst::kIdMov, RD, x86::eax);
}

void TranslateLBU(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateLoad(tr, inst, 1);
    EMIT(x86::Inst::kIdMov, RD, x86::eax);
}

void TranslateLHU(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateLoad(tr, inst, 2);
    EMIT(x86::Inst::kIdMov, RD, x86::eax);
}

void TranslateLWU(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateLoad(tr, inst, 4);
    EMIT(x86::Inst::kIdMov, RD, x86::eax);
}

void TranslateSB(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateStore(tr, inst, 1);
}

void TranslateSH(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateStore(tr, inst, 2);
}

void TranslateSW(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateStore(tr, inst, 4);
}

void TranslateLUI(const jit::Translator &tr, const ir::Inst *inst)
{
    EMIT(x86::Inst::kIdMov, RD, Imm(UIMM));
}

void TranslateAUIPC(const jit::Translator &tr, const ir::Inst *inst)
{
    EMIT(x86::Inst::kIdMov, TMP, PC);
    EMIT(x86::Inst::kIdAdd, TMP, Imm(OFFSET + UIMM));
    EMIT(x86::Inst::kIdMov, RD, TMP);
}

void TranslateJAL(const jit::Translator &tr, const ir::Inst *inst)
{
    EMIT(x86::Inst::kIdMov, TMP, PC);
    EMIT(x86::Inst::kIdAdd, TMP, Imm(OFFSET));
    EMIT(x86::Inst::kIdMov, RD, TMP);
    EMIT(x86::Inst::kIdAdd, RD, Imm(4));
    EMIT(x86::Inst::kIdAdd, TMP, Imm(IMM * 2));
    EMIT(x86::Inst::kIdMov, PC, TMP);
    END_TRACE();
}

void TranslateJALR(const jit::Translator &tr, const ir::Inst *inst)
{
    EMIT(x86::Inst::kIdMov, TMP, PC);
    EMIT(x86::Inst::kIdAdd, TMP, Imm(OFFSET + 4));
    EMIT(x86::Inst::kIdMov, RD, TMP);
    EMIT(x86::Inst::kIdMov, TMP, RS1);
    EMIT(x86::Inst::kIdAdd, TMP, Imm(IMM));
    EMIT(x86::Inst::kIdAnd, TMP, Imm(~1u));
    EMIT(x86::Inst::kIdMov, PC, TMP);
    END_TRACE();
}

void TranslateBEQ(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateCondBranch(tr, inst, x86::Inst::kIdJe);
}

void TranslateBNE(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateCondBranch(tr, inst, x86::Inst::kIdJne);
}

void TranslateBLT(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateCondBranch(tr, inst, x86::Inst::kIdJl);
}

void TranslateBGE(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateCondBranch(tr, inst, x86::Inst::kIdJge);
}

void TranslateBLTU(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateCondBranch(tr, inst, x86::Inst::kIdJb);
}

void TranslateBGEU(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateCondBranch(tr, inst, x86::Inst::kIdJae);
}

void TranslateADDI(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateImmType(tr, inst, IMM ? x86::Inst::kIdAdd : x86::Inst::kIdMov);
}

void TranslateSLTI(const jit::Translator &tr, const ir::Inst *inst)
{
    EMIT(x86::Inst::kIdCmp, RS1, Imm(IMM));
    EMIT(x86::Inst::kIdMov, RD, Imm(0));
    EMIT(x86::Inst::kIdSetl, RD);
}

void TranslateSLTIU(const jit::Translator &tr, const ir::Inst *inst)
{
    EMIT(x86::Inst::kIdCmp, RS1, Imm(UIMM));
    EMIT(x86::Inst::kIdMov, RD, Imm(0));
    EMIT(x86::Inst::kIdSetb, RD);
}

void TranslateXORI(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateImmType(tr, inst, IMM ? x86::Inst::kIdXor : x86::Inst::kIdMov);
}

void TranslateORI(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateImmType(tr, inst, IMM ? x86::Inst::kIdOr : x86::Inst::kIdMov);
}

void TranslateANDI(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateImmType(tr, inst, x86::Inst::kIdAnd);
}

void TranslateSLLI(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateImmType(tr, inst, UIMM ? x86::Inst::kIdShl : x86::Inst::kIdMov);
}

void TranslateSRLI(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateImmType(tr, inst, UIMM ? x86::Inst::kIdShr : x86::Inst::kIdMov);
}

void TranslateSRAI(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateImmType(tr, inst, UIMM ? x86::Inst::kIdSar : x86::Inst::kIdMov);
}

void TranslateADD(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateRegType(tr, inst, inst->GetRd() ? x86::Inst::kIdAdd : x86::Inst::kIdMov);
}

void TranslateSUB(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateRegType(tr, inst, inst->GetRd() ? x86::Inst::kIdSub : x86::Inst::kIdMov);
}

void TranslateSLL(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateShift(tr, inst, x86::Inst::kIdShl);
}

void TranslateSLT(const jit::Translator &tr, const ir::Inst *inst)
{
    Operand FST = RS1;
    if (RS1.isMem())
    {
        EMIT(x86::Inst::kIdMov, TMP, RS1);
        FST = TMP;
    }
    EMIT(x86::Inst::kIdCmp, FST, RS2);
    EMIT(x86::Inst::kIdMov, RD, Imm(0));
    EMIT(x86::Inst::kIdSetl, RD);
}

void TranslateSLTU(const jit::Translator &tr, const ir::Inst *inst)
{
    Operand FST = RS1;
    if (RS1.isMem())
    {
        EMIT(x86::Inst::kIdMov, TMP, RS1);
        FST = TMP;
    }
    EMIT(x86::Inst::kIdCmp, FST, RS2);
    EMIT(x86::Inst::kIdMov, RD, Imm(0));
    EMIT(x86::Inst::kIdSetb, RD);
}

void TranslateXOR(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateRegType(tr, inst, inst->GetRd() ? x86::Inst::kIdXor : x86::Inst::kIdMov);
}

void TranslateSRL(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateShift(tr, inst, x86::Inst::kIdShr);
}

void TranslateSRA(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateShift(tr, inst, x86::Inst::kIdSar);
}

void TranslateOR(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateRegType(tr, inst, inst->GetRd() ? x86::Inst::kIdOr : x86::Inst::kIdMov);
}

void TranslateAND(const jit::Translator &tr, const ir::Inst *inst)
{
    TranslateRegType(tr, inst, x86::Inst::kIdAnd);
}
