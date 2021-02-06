#ifndef JIT_H
#define JIT_H
#include "asmjit/asmjit.h"
#include "ir.h"
#include <bitset>
#include <memory>
#include <queue>
#include <vector>

namespace jit {

class Runtime {
public:
  static asmjit::JitRuntime &get() {
    static asmjit::JitRuntime runtime;
    return runtime;
  }

private:
  Runtime(){};
  Runtime(const Runtime &);
  Runtime &operator=(const Runtime &);
};

class Translator {
public:
  Translator(const std::vector<ir::Inst> &trace);
  ExecTracePtr getFunc() const { return func_; }
  asmjit::x86::Assembler &getAsm() const { return x86asm_; }
  asmjit::Operand getReg(ir::Reg reg) const;
  asmjit::Operand getPc() const;
  asmjit::Operand getTmp() const { return asmjit::x86::esi; }
  asmjit::Operand getMem() const;
  asmjit::Operand getLoadFunc() const;
  asmjit::Operand getStoreFunc() const;
  asmjit::Label getFunctionStart() const { return func_start_; };
  size_t getOffset() const { return cur_inst_ * 4; }

  void saveAllRegs() const;
  void restoreAllRegs() const;

  void deallocateAllRegs() const {
    for (unsigned i = 1; i < 32; ++i)
      deallocateReg(i, true);
  }
private:
  ExecTracePtr func_ = nullptr;
  asmjit::Label func_start_;
  size_t cur_inst_;

  mutable asmjit::CodeHolder code_;
  mutable asmjit::x86::Assembler x86asm_;
  mutable asmjit::FileLogger logger_;

  mutable std::array<asmjit::Operand, 32> reg_mapping_;
  mutable std::queue<asmjit::x86::Gp> reg_pool_;
  std::vector<std::bitset<32>> liveness_;
  void calcLiveness(const std::vector<ir::Inst> &trace);
  asmjit::x86::Mem getRegMemOp(ir::Reg reg) const;
  void allocateReg(ir::Reg reg, bool load = false) const;
  void deallocateReg(ir::Reg reg, bool sink = false) const;
  // free regs - edx, ecx, r8d, r9d, r10d, r11d
};
} // namespace jit

#endif
