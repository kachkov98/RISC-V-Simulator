#include "jit.h"
#include "common.h"
#include "sim.h"

using namespace asmjit;

uint32_t Load(MMU *mmu, uint32_t va, uint8_t nbytes) { return mmu->load(va, nbytes, true); }

void Store(MMU *mmu, uint32_t va, uint8_t nbytes, uint32_t data) { mmu->store(va, nbytes, data); }

namespace jit {

Translator::Translator(const std::vector<ir::Inst> &trace) : logger_(options::log) {
  for (const ir::Inst &inst : trace) {
    if (options::verbose)
      inst.dump(options::log);
    if (!inst.isTranslationSupported()) {
      log("Unsupported instruction!\n");
      return;
    }
  }
  if (options::verbose)
    code_.setLogger(&logger_);

  code_.init(Runtime::get().codeInfo());
  code_.attach(&x86asm_);

  for (uint8_t i = 1; i < 32; ++i)
    reg_mapping_[i] = getRegMemOp(i);
  for (auto reg : {x86::edx, x86::ecx, x86::r8d, x86::r9d, x86::r10d, x86::r11d})
    reg_pool_.push(reg);
  calcLiveness(trace);
  if (options::verbose) {
    log("Liveness:\n");
    for (const auto &info : liveness_)
      log("%s\n", info.to_string().c_str());
  }

  for (cur_inst_ = 0; cur_inst_ < trace.size(); ++cur_inst_) {
    const ir::Inst &inst = trace[cur_inst_];
    // Regalloc pre-work
    if (inst.isRs1() && inst.getRs1() && liveness_[cur_inst_][inst.getRs1()])
      allocateReg(inst.getRs1(), true);
    if (inst.isRs2() && inst.getRs2() && liveness_[cur_inst_][inst.getRs2()])
      allocateReg(inst.getRs2(), true);
    if (inst.isRd() && inst.getRd() && liveness_[cur_inst_][inst.getRd()])
      allocateReg(inst.getRd());
    // Translate inst
    inst.translate(*this);
    // Regalloc post-work
    if (inst.isRs1() && inst.getRs1() && !liveness_[cur_inst_][inst.getRs1()])
      deallocateReg(inst.getRs1(), true);
    if (inst.isRs2() && inst.getRs2() && !liveness_[cur_inst_][inst.getRs2()])
      deallocateReg(inst.getRs2(), true);
    if (inst.isRd() && inst.getRd() && !liveness_[cur_inst_][inst.getRd()])
      deallocateReg(inst.getRd(), true);
  }
  getAsm().emit(asmjit::x86::Inst::kIdRet);
  Runtime::get().add(&func_, &code_);
}

Operand Translator::getReg(ir::Reg reg) const { return reg ? reg_mapping_[reg] : Imm(0); }

Operand Translator::getMMU() const {
  size_t offset = offsetof(sim::State, mmu_);
  return x86::ptr_64(asmjit::x86::rdi, offset);
}

Operand Translator::getLoadFunc() const { return Imm(reinterpret_cast<uint64_t>(&Load)); }

Operand Translator::getStoreFunc() const { return Imm(reinterpret_cast<uint64_t>(&Store)); }

Operand Translator::getMem() const { return x86::ptr_64(x86::rdi, offsetof(sim::State, mem_)); }

Operand Translator::getPc() const {
  size_t offset = offsetof(sim::State, pc_);
  return x86::ptr_32(x86::rdi, offset);
}

void Translator::saveAllRegs() const {
  for (uint8_t i = 1; i < 32; ++i)
    if (reg_mapping_[i].isReg())
      getAsm().emit(x86::Inst::kIdPush, reg_mapping_[i]);
}

void Translator::restoreAllRegs() const {
  for (uint8_t i = 31; i > 0; --i)
    if (reg_mapping_[i].isReg())
      getAsm().emit(x86::Inst::kIdPop, reg_mapping_[i]);
}

void Translator::calcLiveness(const std::vector<ir::Inst> &trace) {
  liveness_.resize(trace.size());
  std::bitset<32> cur_info;
  for (int i = trace.size() - 1; i >= 0; --i) {
    liveness_[i] = cur_info;
    if (trace[i].isRd())
      cur_info[trace[i].getRd()] = 0;
    if (trace[i].isRs1())
      cur_info[trace[i].getRs1()] = 1;
    if (trace[i].isRs2())
      cur_info[trace[i].getRs2()] = 1;
  }
}

x86::Mem Translator::getRegMemOp(ir::Reg reg) const {
  return x86::ptr_32(x86::rdi, offsetof(sim::State, regs_) + reg * 4);
}

void Translator::allocateReg(ir::Reg reg, bool load) const {
  if (reg_mapping_[reg].isReg())
    return;
  if (reg_pool_.empty())
    return;
  reg_mapping_[reg] = reg_pool_.front();
  reg_pool_.pop();
  if (load)
    getAsm().emit(x86::Inst::kIdMov, reg_mapping_[reg], getRegMemOp(reg));
}

void Translator::deallocateReg(ir::Reg reg, bool sink) const {
  if (reg_mapping_[reg].isMem())
    return;
  if (sink)
    getAsm().emit(x86::Inst::kIdMov, getRegMemOp(reg), reg_mapping_[reg]);
  reg_pool_.push(reg_mapping_[reg].as<x86::Gp>());
  reg_mapping_[reg] = getRegMemOp(reg);
}

} // namespace jit
