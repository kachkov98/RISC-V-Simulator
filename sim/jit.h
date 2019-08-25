#ifndef JIT_H
#define JIT_H
#include "asmjit/asmjit.h"
#include "ir.h"
#include <memory>
#include <vector>

namespace jit
{

class Runtime
{
public:
    static asmjit::JitRuntime &Get()
    {
        static asmjit::JitRuntime runtime;
        return runtime;
    }

private:
    Runtime(){};
    Runtime(const Runtime &);
    Runtime &operator=(const Runtime &);
};

class Translator
{
public:
    Translator(const std::vector<ir::Inst> &trace);
    ExecTracePtr GetFunc() const
    {
        return func_;
    }
    asmjit::x86::Assembler &GetAsm() const
    {
        return x86asm_;
    }
    asmjit::Operand GetReg(ir::Reg reg) const;
    asmjit::Operand GetTmp() const
    {
        return asmjit::x86::esi;
    }
    asmjit::Operand GetMMU() const;
    asmjit::Operand GetLoadFunc() const;
    asmjit::Operand GetStoreFunc() const;
    asmjit::Operand GetPc() const;
    size_t GetOffset() const
    {
        return cur_inst_ * 4;
    }

private:
    ExecTracePtr func_ = nullptr;
    size_t cur_inst_;

    mutable asmjit::CodeHolder code_;
    mutable asmjit::x86::Assembler x86asm_;
    mutable asmjit::FileLogger logger_;
    // free regs - edx, ecx, r8d, r9d, r10d, r11d
};
}   // namespace jit

#endif
