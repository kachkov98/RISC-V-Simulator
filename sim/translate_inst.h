#ifndef TRANSLATE_INST_H
#define TRANSLATE_INST_H

namespace ir
{
class Inst;
}

namespace jit
{
class Translator;
}

void TranslateDummy(const jit::Translator &tr, const ir::Inst *inst);

void TranslateLUI(const jit::Translator &tr, const ir::Inst *inst);
void TranslateAUIPC(const jit::Translator &tr, const ir::Inst *inst);

void TranslateJAL(const jit::Translator &tr, const ir::Inst *inst);
void TranslateJALR(const jit::Translator &tr, const ir::Inst *inst);
void TranslateBEQ(const jit::Translator &tr, const ir::Inst *inst);
void TranslateBNE(const jit::Translator &tr, const ir::Inst *inst);
void TranslateBLT(const jit::Translator &tr, const ir::Inst *inst);
void TranslateBGE(const jit::Translator &tr, const ir::Inst *inst);
void TranslateBLTU(const jit::Translator &tr, const ir::Inst *inst);
void TranslateBGEU(const jit::Translator &tr, const ir::Inst *inst);

void TranslateADDI(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSLTI(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSLTIU(const jit::Translator &tr, const ir::Inst *inst);
void TranslateXORI(const jit::Translator &tr, const ir::Inst *inst);
void TranslateORI(const jit::Translator &tr, const ir::Inst *inst);
void TranslateANDI(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSLLI(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSRLI(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSRAI(const jit::Translator &tr, const ir::Inst *inst);

void TranslateADD(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSUB(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSLL(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSLT(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSLTU(const jit::Translator &tr, const ir::Inst *inst);
void TranslateXOR(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSRL(const jit::Translator &tr, const ir::Inst *inst);
void TranslateSRA(const jit::Translator &tr, const ir::Inst *inst);
void TranslateOR(const jit::Translator &tr, const ir::Inst *inst);
void TranslateAND(const jit::Translator &tr, const ir::Inst *inst);

#endif
