#include "jit.h"
#include "common.h"
#include "sim.h"

using namespace asmjit;

uint32_t Load(MMU *mmu, uint32_t va, uint8_t nbytes)
{
    return mmu->Load(va, nbytes, true);
}

void Store(MMU *mmu, uint32_t va, uint8_t nbytes, uint32_t data)
{
    mmu->Store(va, nbytes, data);
}

namespace jit
{

Translator::Translator(const std::vector<ir::Inst> &trace)
    : logger_(options::log)
{
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
    
    code_.init(Runtime::Get().codeInfo());
    code_.attach(&x86asm_);

    for (uint8_t i = 1; i < 32; ++i)
        reg_mapping_[i] = GetRegMemOp(i);
    for (auto reg: {x86::edx, x86::ecx, x86::r8d, x86::r9d, x86::r10d, x86::r11d})
        reg_pool_.push(reg);
    CalcLiveness(trace);
    if (options::verbose)
    {
        log("Liveness:\n");
        for (const auto& info : liveness_)
            log("%s\n", info.to_string().c_str());
    }

    for (cur_inst_ = 0; cur_inst_ < trace.size(); ++cur_inst_)
    {
        const ir::Inst &inst = trace[cur_inst_];
        // Regalloc pre-work
        if (inst.IsRs1() && inst.GetRs1() && liveness_[cur_inst_][inst.GetRs1()])
            AllocateReg(inst.GetRs1(), true);
        if (inst.IsRs2() && inst.GetRs2() && liveness_[cur_inst_][inst.GetRs2()])
            AllocateReg(inst.GetRs1(), true);
        if (inst.IsRd() && inst.GetRd() && liveness_[cur_inst_][inst.GetRd()])
            AllocateReg(inst.GetRd(), true);
        // Translate inst
        inst.Translate(*this);
        // Regalloc post-work
        if (inst.IsRs1() && inst.GetRs1() && !liveness_[cur_inst_][inst.GetRs1()])
            DeallocateReg(inst.GetRs1(), true);
        if (inst.IsRs2() && inst.GetRs2() && !liveness_[cur_inst_][inst.GetRs2()])
            DeallocateReg(inst.GetRs2(), true);
        if (inst.IsRd() && inst.GetRd() && !liveness_[cur_inst_][inst.GetRd()])
            DeallocateReg(inst.GetRd(), true);
    }
    GetAsm().emit(asmjit::x86::Inst::kIdRet);
    Runtime::Get().add(&func_, &code_);
}

Operand Translator::GetReg(ir::Reg reg) const
{
    return reg ? reg_mapping_[reg] : Imm(0);
}

Operand Translator::GetMMU() const
{
    size_t offset = offsetof(sim::State, mmu_);
    return x86::ptr_64(asmjit::x86::rdi, offset);
}

Operand Translator::GetLoadFunc() const
{
    return Imm(reinterpret_cast<uint64_t>(&Load));
}

Operand Translator::GetStoreFunc() const
{
    return Imm(reinterpret_cast<uint64_t>(&Store));
}

Operand Translator::GetPc() const
{
    size_t offset = offsetof(sim::State, pc_);
    return x86::ptr_32(x86::rdi, offset);
}

void Translator::SaveAllRegs() const
{
    for (uint8_t i = 1; i < 32; ++i)
        if (reg_mapping_[i].isReg())
            GetAsm().emit(x86::Inst::kIdPush, reg_mapping_[i]);
}

void Translator::RestoreAllRegs() const
{
    for (uint8_t i = 31; i > 0; --i)
        if (reg_mapping_[i].isReg())
            GetAsm().emit(x86::Inst::kIdPop, reg_mapping_[i]);
}

void Translator::CalcLiveness(const std::vector<ir::Inst> &trace)
{
    liveness_.resize(trace.size());
    std::bitset<32> cur_info;
    for (int i = trace.size() - 1; i >= 0; --i)
    {
        liveness_[i] = cur_info;
        if (trace[i].IsRd())
            cur_info[trace[i].GetRd()] = 0;
        if (trace[i].IsRs1())
            cur_info[trace[i].GetRs1()] = 1;
        if (trace[i].IsRs2())
            cur_info[trace[i].GetRs2()] = 1;
    }
}

x86::Mem Translator::GetRegMemOp(ir::Reg reg) const
{
    return x86::ptr_32(x86::rdi, offsetof(sim::State, regs_) + reg * 4);
}

void Translator::AllocateReg(ir::Reg reg, bool load) const
{
    if (reg_mapping_[reg].isReg())
        return;
    if (reg_pool_.empty())
        return;
    reg_mapping_[reg] = reg_pool_.front();
    reg_pool_.pop();
    if (load)
        GetAsm().emit(x86::Inst::kIdMov, reg_mapping_[reg], GetRegMemOp(reg));
}

void Translator::DeallocateReg(ir::Reg reg, bool sink) const
{
    if (reg_mapping_[reg].isMem())
        return;
    if (sink)
        GetAsm().emit(x86::Inst::kIdMov, GetRegMemOp(reg), reg_mapping_[reg]);
    reg_pool_.push(reg_mapping_[reg].as<x86::Gp>());
    reg_mapping_[reg] = GetRegMemOp(reg);
}

}   // namespace jit
