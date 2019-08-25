#include "isa_desc.h"
#include "common.h"
#include "exec_inst.h"
#include "translate_inst.h"
#include <cassert>

namespace isa
{
// clang-format off
static constexpr OpcodeDesc opcode_desc[] =
{
    {0b00000, CmdFormat::I}, // LOAD
    {0b01000, CmdFormat::S}, // STORE
    {0b11000, CmdFormat::B}, // BRANCH
    {0b11001, CmdFormat::I}, // JALR
    {0b11011, CmdFormat::J}, // JAL
    {0b00100, CmdFormat::I}, // OP_IMM
    {0b01100, CmdFormat::R}, // OP
    {0b11100, CmdFormat::I}, // SYSTEM
    {0b00101, CmdFormat::U}, // AUIPC
    {0b01101, CmdFormat::U}, // LUI
    {0b00011, CmdFormat::I}  // MSCMEM
};

static constexpr CmdDesc cmd_desc[] =
{
    {"LUI",    &ExecLUI,   &TranslateLUI,   Opcode::LUI   },
    {"AUIPC",  &ExecAUIPC, &TranslateAUIPC, Opcode::AUIPC },
    {"JAL",    &ExecJAL,   &TranslateJAL,   Opcode::JAL   },
    {"JALR",   &ExecJALR,  &TranslateJALR,  Opcode::JALR,   0b000},
    {"BEQ",    &ExecBEQ,   &TranslateBEQ,   Opcode::BRANCH, 0b000},
    {"BNE",    &ExecBNE,   &TranslateBNE,   Opcode::BRANCH, 0b001},
    {"BLT",    &ExecBLT,   &TranslateBLT,   Opcode::BRANCH, 0b100},
    {"BGE",    &ExecBGE,   &TranslateBGE,   Opcode::BRANCH, 0b101},
    {"BLTU",   &ExecBLTU,  &TranslateBLTU,  Opcode::BRANCH, 0b110},
    {"BGEU",   &ExecBGEU,  &TranslateBGEU,  Opcode::BRANCH, 0b111},
    {"LB",     &ExecLB,    &TranslateLB,    Opcode::LOAD,   0b000},
    {"LH",     &ExecLH,    &TranslateLH,    Opcode::LOAD,   0b001},
    {"LW",     &ExecLW,    &TranslateLW,    Opcode::LOAD,   0b010},
    {"LBU",    &ExecLBU,   &TranslateLBU,   Opcode::LOAD,   0b100},
    {"LHU",    &ExecLHU,   &TranslateLHU,   Opcode::LOAD,   0b101},
    {"SB",     &ExecSB,    &TranslateSB,    Opcode::STORE,  0b000},
    {"SH",     &ExecSH,    &TranslateSH,    Opcode::STORE,  0b001},
    {"SW",     &ExecSW,    &TranslateSW,    Opcode::STORE,  0b010},
    {"ADDI",   &ExecADDI,  &TranslateADDI,  Opcode::OP_IMM, 0b000},
    {"SLTI",   &ExecSLTI,  &TranslateSLTI,  Opcode::OP_IMM, 0b010},
    {"SLTIU",  &ExecSLTIU, &TranslateSLTIU, Opcode::OP_IMM, 0b011},
    {"XORI",   &ExecXORI,  &TranslateXORI,  Opcode::OP_IMM, 0b100},
    {"ORI",    &ExecORI,   &TranslateORI,   Opcode::OP_IMM, 0b110},
    {"ANDI",   &ExecANDI,  &TranslateANDI,  Opcode::OP_IMM, 0b111},
    {"SLLI",   &ExecSLLI,  &TranslateSLLI,  Opcode::OP_IMM, 0b001},
    {"SRLI",   &ExecSRLI,  &TranslateSRLI,  Opcode::OP_IMM, 0b101},
    {"SRAI",   &ExecSRAI,  &TranslateSRAI,  Opcode::OP_IMM, 0b101},
    {"ADD",    &ExecADD,   &TranslateADD,   Opcode::OP,     0b000, 0b0000000},
    {"SUB",    &ExecSUB,   &TranslateSUB,   Opcode::OP,     0b000, 0b0100000},
    {"SLL",    &ExecSLL,   &TranslateSLL,   Opcode::OP,     0b001, 0b0000000},
    {"SLT",    &ExecSLT,   &TranslateSLT,   Opcode::OP,     0b010, 0b0000000},
    {"SLTU",   &ExecSLTU,  &TranslateSLTU,  Opcode::OP,     0b011, 0b0000000},
    {"XOR",    &ExecXOR,   &TranslateXOR,   Opcode::OP,     0b100, 0b0000000},
    {"SRL",    &ExecSRL,   &TranslateSRL,   Opcode::OP,     0b101, 0b0000000},
    {"SRA",    &ExecSRA,   &TranslateSRA,   Opcode::OP,     0b101, 0b0100000},
    {"OR",     &ExecOR,    &TranslateOR,    Opcode::OP,     0b110, 0b0000000},
    {"AND",    &ExecAND,   &TranslateAND,   Opcode::OP,     0b111, 0b0000000},
    {"ECALL",  &ExecECALL, nullptr,         Opcode::SYSTEM, 0b000},
    {"EBREAK", &ExecDummy, nullptr,         Opcode::SYSTEM, 0b000},
    {"URET",   &ExecDummy, nullptr,         Opcode::SYSTEM, 0b000},
    {"SRET",   &ExecDummy, nullptr,         Opcode::SYSTEM, 0b000},
    {"MRET",   &ExecDummy, nullptr,         Opcode::SYSTEM, 0b000},
    {"WFI",    &ExecDummy, nullptr,         Opcode::SYSTEM, 0b000},
    {"FENCE",  &ExecFENCE, nullptr,         Opcode::MSCMEM, 0b000},
    {"CSRRW",  &ExecCSRRW, nullptr,         Opcode::SYSTEM, 0b001},
    {"CSRRS",  &ExecDummy, nullptr,         Opcode::SYSTEM, 0b010},
    {"CSRRC",  &ExecDummy, nullptr,         Opcode::SYSTEM, 0b011},
    {"CSRRWI", &ExecDummy, nullptr,         Opcode::SYSTEM, 0b101},
    {"CSRRSI", &ExecDummy, nullptr,         Opcode::SYSTEM, 0b110},
    {"CSRRCI", &ExecDummy, nullptr,         Opcode::SYSTEM, 0b111},
    // RV32M standard extension
    {"MUL",    &ExecMUL,   nullptr,         Opcode::OP,     0b000, 0b0000001},
    {"MULH",   &ExecMULH,  nullptr,         Opcode::OP,     0b001, 0b0000001},
    {"MULHSU", &ExecMULHSU,nullptr,         Opcode::OP,     0b010, 0b0000001},
    {"MULHU",  &ExecMULHU, nullptr,         Opcode::OP,     0b011, 0b0000001},
    {"DIV",    &ExecDIV,   nullptr,         Opcode::OP,     0b100, 0b0000001},
    {"DIVU",   &ExecDIVU,  nullptr,         Opcode::OP,     0b101, 0b0000001},
    {"REM",    &ExecREM,   nullptr,         Opcode::OP,     0b110, 0b0000001},
    {"REMU",   &ExecREMU,  nullptr,         Opcode::OP,     0b111, 0b0000001}
};
// clang-format on

size_t GetOpcodesNum()
{
    return ArrSize(opcode_desc);
}

const OpcodeDesc &GetOpcodeDesc(uint8_t opcode)
{
    assert(opcode < GetOpcodesNum());
    return opcode_desc[opcode];
}

const OpcodeDesc &GetOpcodeDesc(Opcode opcode)
{
    return GetOpcodeDesc(static_cast<uint8_t>(opcode));
}

size_t GetCmdsNum()
{
    return ArrSize(cmd_desc);
}

const CmdDesc &GetCmdDesc(uint8_t cmd)
{
    assert(cmd < GetCmdsNum());
    return cmd_desc[cmd];
}

const CmdDesc &GetCmdDesc(Cmd cmd)
{
    return GetCmdDesc(static_cast<uint8_t>(cmd));
}
}   // namespace isa
