#ifndef IR_H
#define IR_H

#include "isa_desc.h"
#include <cassert>
#include <cstdio>
#include <type_traits>

namespace ir {
class Reg {
private:
  uint8_t reg_;

public:
  Reg(uint8_t reg) : reg_(reg) { assert(reg_ < 32); }
  Reg(isa::Regs reg) : reg_(static_cast<uint8_t>(reg)) { assert(reg_ < 32); }
  operator uint8_t() const { return reg_; }
  void dump(FILE *f) const;
};

class Imm {
private:
  uint32_t imm_;

public:
  Imm(uint32_t imm) : imm_(imm) {}
  operator uint32_t() const { return imm_; }
  void dump(FILE *f) const;
};

class Inst {
private:
  Imm imm_;
  Reg rs1_, rs2_, rd_;
  isa::Cmd cmd_;
  ExecFunc func_;

public:
  Inst(isa::Cmd cmd, Reg rd, Reg rs1, Reg rs2, Imm imm)
      : imm_(imm), rs1_(rs1), rs2_(rs2), rd_(rd), cmd_(cmd),
        func_(isa::getCmdDesc(cmd_).exec_func) {}
  bool isImm() const { return getCmdFormat() != isa::CmdFormat::R; }
  Imm getImm() const {
    assert(isImm());
    return imm_;
  }
  bool isRs1() const {
    return getCmdFormat() != isa::CmdFormat::U && getCmdFormat() != isa::CmdFormat::J;
  }
  Reg getRs1() const {
    assert(isRs1());
    return rs1_;
  }
  bool isRs2() const {
    return getCmdFormat() != isa::CmdFormat::I && getCmdFormat() != isa::CmdFormat::U &&
           getCmdFormat() != isa::CmdFormat::J;
  }
  Reg getRs2() const {
    assert(isRs2());
    return rs2_;
  }
  bool isRd() const {
    return getCmdFormat() != isa::CmdFormat::S && getCmdFormat() != isa::CmdFormat::B;
  }
  Reg getRd() const {
    assert(isRd());
    return rd_;
  }
  isa::Cmd getCmd() const { return cmd_; }
  isa::Opcode getOpcode() const {
    return isa::getCmdDesc(cmd_).opcode;
    ;
  }
  isa::CmdFormat getCmdFormat() const { return isa::getOpcodeDesc(getOpcode()).format; }
  void dump(FILE *f) const;
  void exec(const ir::Inst *fst_inst, sim::State *state) const { func_(fst_inst, this, state); }
  bool isTranslationSupported() const { return isa::getCmdDesc(cmd_).translate_func != nullptr; }
  void translate(const jit::Translator &tr) const {
    isa::getCmdDesc(cmd_).translate_func(tr, this);
  }
};

template <isa::CmdFormat Format>
typename std::enable_if<Format == isa::CmdFormat::R, Inst>::type GenInst(isa::Cmd cmd, Reg rd,
                                                                         Reg rs1, Reg rs2) {
  Inst inst(cmd, rd, rs1, rs2, 0);
  assert(inst.getCmdFormat() == isa::CmdFormat::R);
  return inst;
}

template <isa::CmdFormat Format>
typename std::enable_if<Format == isa::CmdFormat::I, Inst>::type GenInst(isa::Cmd cmd, Reg rd,
                                                                         Reg rs1, Imm imm) {
  Inst inst(cmd, rd, rs1, 0, imm);
  assert(inst.getCmdFormat() == isa::CmdFormat::I);
  return inst;
}

template <isa::CmdFormat Format>
typename std::enable_if<Format == isa::CmdFormat::S, Inst>::type GenInst(isa::Cmd cmd, Imm imm,
                                                                         Reg rs1, Reg rs2) {
  Inst inst(cmd, 0, rs1, rs2, imm);
  assert(inst.getCmdFormat() == isa::CmdFormat::S);
  return inst;
}

template <isa::CmdFormat Format>
typename std::enable_if<Format == isa::CmdFormat::U, Inst>::type GenInst(isa::Cmd cmd, Reg rd,
                                                                         Imm imm) {
  Inst inst(cmd, rd, 0, 0, imm);
  assert(inst.getCmdFormat() == isa::CmdFormat::U);
  return inst;
}

template <isa::CmdFormat Format>
typename std::enable_if<Format == isa::CmdFormat::B, Inst>::type GenInst(isa::Cmd cmd, Imm imm,
                                                                         Reg rs1, Reg rs2) {
  Inst inst(cmd, 0, rs1, rs2, imm);
  assert(inst.getCmdFormat() == isa::CmdFormat::B);
  return inst;
}

template <isa::CmdFormat Format>
typename std::enable_if<Format == isa::CmdFormat::J, Inst>::type GenInst(isa::Cmd cmd, Reg rd,
                                                                         Imm imm) {
  Inst inst(cmd, rd, 0, 0, imm);
  assert(inst.getCmdFormat() == isa::CmdFormat::J);
  return inst;
}
} // namespace ir

#endif
