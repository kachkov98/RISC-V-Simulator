#include "sim.h"
#include "jit.h"
#include "stats.h"

namespace sim
{
// State
void State::Dump(FILE *f) const
{
    fprintf(f, "Processor state:\n");
    for (uint8_t i = 0; i < 32; ++i)
    {
        ir::Reg(i).Dump(f);
        fprintf(f, ": 0x%08X%c", regs_[i], (i + 1) % 8 ? '\t' : '\n');
    }
    fprintf(f, "PC: 0x%08X\n", pc_);
}

// Trace
Trace::Trace(const Decoder &decoder, State &state)
{
    uint32_t address = state.GetPC();
    while (true)
    {
        ir::Inst inst = decoder.Decode(state.GetCmd(address));
        trace_.push_back(inst);
        isa::Opcode opcode = isa::GetCmdDesc(trace_.back().GetCmd()).opcode;
        if (opcode == isa::Opcode::BRANCH || opcode == isa::Opcode::JALR ||
            opcode == isa::Opcode::JAL || inst.GetCmd() == isa::Cmd::FENCE)
            break;
        address += 4;
    }
    trace_.shrink_to_fit();
    if (options::verbose)
    {
        fprintf(options::log, "Fetching trace:\n");
        Dump(options::log);
    }
}

void Trace::Execute(State *state) const
{
    if (options::jit && is_eligible_ && !exec_trace_ && options::jit_threshold == exec_num_++)
    {
        log("Attempt to translate trace...\n");
        exec_trace_ = ExecTraceType(jit::Translator(trace_).GetFunc());
        if (!exec_trace_)
        {
            is_eligible_ = false;
            ++stats::jit_failed_translations;
            log("Fail\n");
        }
        else
        {
            ++stats::jit_finished_translations;
            log("Success\n");
        }
    }
    log("Executing trace...\n");
    if (exec_trace_)
    {
        (*exec_trace_)(state);
        ++stats::translated_executions;
    }
    else
    {
        trace_.data()->Exec(trace_.data(), state);
        ++stats::interpreted_executions;
    }
    stats::executed_insts += trace_.size();
    if (options::verbose)
        state->Dump(options::log);
}

void Trace::Dump(FILE *f) const
{
    for (const auto &inst : trace_)
        inst.Dump(f);
}

void Trace::ExecTraceDeleter::operator()(ExecTracePtr trace)
{
    if (trace)
        jit::Runtime::Get().release(trace);
}

// TraceCache
void TraceCache::Dump(FILE *f) const
{
    fprintf(f, "Cached traces:\n");
    for (const auto &trace : cache_)
    {
        fprintf(f, "Address: 0x%#08X\n", trace.first);
        trace.second.Dump(f);
    }
}

// Sim
Sim::Sim(const std::vector<std::vector<uint32_t>> &commands,
         const std::vector<uint32_t> &seg_va,
         uint32_t pc)
    : state_(commands, seg_va, pc)
{
}

void Sim::Execute()
{
    Timer timer;
    timer.Start();
    try
    {
        while (true)
        {
            state_.trace_cache.Refer(decoder_, state_, state_.GetPC()).Execute(&state_);
            if (options::max_insts && stats::executed_insts >= options::max_insts)
                break;
        }
    }
    catch (SimException &e)
    {
        fprintf(options::log, "%s\n", e.what());
    }
    timer.Finish();
    state_.Dump(options::log);
    fprintf(options::log, "Some statistics:\n");
    uint64_t time = timer.GetMilliseconds();
    fprintf(options::log, "Time: %lu ms, MIPS: %.3lf\n",
            time, (double)stats::executed_insts / (time * 1000));
    stats::PrintStatistics(options::log);
}
}   // namespace sim
