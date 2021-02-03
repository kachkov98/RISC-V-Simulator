#include "ir.h"
#include "common.h"

namespace ir {
// Reg
void Reg::dump(FILE *f) const { fprintf(f, "x%02u", reg_); }

// Imm
void Imm::dump(FILE *f) const { fprintf(f, "i:0x%08X", imm_); }

// Inst
void Inst::dump(FILE *f) const {
  fprintf(f, "\t%-7s", isa::getCmdDesc(cmd_).name);
  switch (getCmdFormat()) {
  case isa::CmdFormat::R:
    rs1_.dump(f);
    fputc(',', f);
    rs2_.dump(f);
    fputs("->", f);
    rd_.dump(f);
    break;
  case isa::CmdFormat::I:
    if ((uint8_t)cmd_ >= (uint8_t)isa::Cmd::ECALL && (uint8_t)cmd_ <= (uint8_t)isa::Cmd::WFI)
      break;
    if ((uint8_t)cmd_ >= (uint8_t)isa::Cmd::CSRRWI && (uint8_t)cmd_ <= (uint8_t)isa::Cmd::CSRRCI)
      Imm(rs1_).dump(f);
    else
      rs1_.dump(f);
    fputc(',', f);
    imm_.dump(f);
    fputs("->", f);
    rd_.dump(f);
    break;
  case isa::CmdFormat::S:
  case isa::CmdFormat::B:
    rs1_.dump(f);
    fputc(',', f);
    rs2_.dump(f);
    fputs(": ", f);
    imm_.dump(f);
    break;
  case isa::CmdFormat::U:
  case isa::CmdFormat::J:
    imm_.dump(f);
    fputs("->", f);
    rd_.dump(f);
    break;
  case isa::CmdFormat::UNDEFINED:
    fputs("UNDEFINED", f);
  };
  fputc('\n', f);
}
} // namespace ir
