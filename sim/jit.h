#ifndef JIT_H
#define JIT_H
#include "asmjit/asmjit.h"
#include "ir.h"
#include <bitset>
#include <memory>
#include <queue>
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

    void SaveAllRegs() const;
    void RestoreAllRegs() const;

private:
    ExecTracePtr func_ = nullptr;
    size_t cur_inst_;

    mutable asmjit::CodeHolder code_;
    mutable asmjit::x86::Assembler x86asm_;
    mutable asmjit::FileLogger logger_;

    mutable std::array<asmjit::Operand, 32> reg_mapping_;
    mutable std::queue<asmjit::x86::Gp> reg_pool_;
    std::vector<std::bitset<32>> liveness_;
    void CalcLiveness(const std::vector<ir::Inst> &trace);
    asmjit::x86::Mem GetRegMemOp(ir::Reg reg) const;
    void AllocateReg(ir::Reg reg, bool load = false) const;
    void DeallocateReg(ir::Reg reg, bool sink = false) const;
    // free regs - edx, ecx, r8d, r9d, r10d, r11d
};
}   // namespace jit

#endif
