#include "jit.h"
#include "common.h"
#include "sim.h"

namespace jit
{

Translator::Translator(const std::vector<ir::Inst> &trace)
    : logger_(options::log)
{
    code_.init(Runtime::Get().codeInfo());
    code_.attach(&x86asm_);

    for (const ir::Inst &inst : trace)
    {
        if (options::verbose)
            inst.Dump(options::log);
        if (!inst.IsTranslationSupported())
        {
            log("Unsupported instruction!\n");
            return;
        }
    }
    if (options::verbose)
        code_.setLogger(&logger_);

    for (cur_inst_ = 0; cur_inst_ < trace.size(); ++cur_inst_)
    {
        // Regalloc pre-work
        trace[cur_inst_].Translate(*this);
        // Regalloc post-work
    }
    Runtime::Get().add(&func_, &code_);
}

asmjit::Operand Translator::GetReg(ir::Reg reg) const
{
    if (reg)
    {
        size_t offset = offsetof(sim::State, regs_) + reg * 4;
        return asmjit::x86::ptr_32(asmjit::x86::rdi, offset);
    }
    else
        return asmjit::Imm(0);
}

asmjit::Operand Translator::GetPc() const
{
    size_t offset = offsetof(sim::State, pc_);
    return asmjit::x86::ptr_32(asmjit::x86::rdi, offset);
}

}   // namespace jit
