#include "sim.h"
#include "jit.h"
#include "stats.h"

namespace sim {
// State
void State::dump(FILE *f) const {
  fprintf(f, "Processor state:\n");
  for (uint8_t i = 0; i < 32; ++i) {
    ir::Reg(i).dump(f);
    fprintf(f, ": 0x%08X%c", regs_[i], (i + 1) % 4 ? '\t' : '\n');
  }
  fprintf(f, "PC: 0x%08X\n", pc_);
}

// Trace
Trace::Trace(const Decoder &decoder, State &state) {
  uint32_t address = state.getPC();
  while (true) {
    ir::Inst inst = decoder.decode(state.getCmd(address));
    trace_.push_back(inst);
    isa::Opcode opcode = isa::getCmdDesc(trace_.back().getCmd()).opcode;
    if (opcode == isa::Opcode::BRANCH || opcode == isa::Opcode::JALR ||
        opcode == isa::Opcode::JAL || inst.getCmd() == isa::Cmd::FENCE)
      break;
    address += 4;
  }
  trace_.shrink_to_fit();
  if (options::verbose) {
    fprintf(options::log, "Fetching trace:\n");
    dump(options::log);
  }
}

void Trace::execute(State *state) const {
  if (options::jit && is_eligible_ && !exec_trace_ && options::jit_threshold == exec_num_++) {
    log("Attempt to translate trace...\n");
    exec_trace_ = ExecTraceType(jit::Translator(trace_).getFunc());
    if (!exec_trace_) {
      is_eligible_ = false;
      ++stats::jit_failed_translations;
      log("Fail\n");
    } else {
      ++stats::jit_finished_translations;
      log("Success\n");
    }
  }
  log("Executing trace...\n");
  if (exec_trace_) {
    (*exec_trace_)(state);
    ++stats::translated_executions;
  } else {
    trace_.data()->exec(trace_.data(), state);
    ++stats::interpreted_executions;
  }
  stats::executed_insts += trace_.size();
  if (options::verbose)
    state->dump(options::log);
}

void Trace::dump(FILE *f) const {
  for (const auto &inst : trace_)
    inst.dump(f);
}

void Trace::ExecTraceDeleter::operator()(ExecTracePtr trace) {
  if (trace)
    jit::Runtime::get().release(trace);
}

// TraceCache
void TraceCache::dump(FILE *f) const {
  fprintf(f, "Cached traces:\n");
  for (const auto &trace : cache_) {
    fprintf(f, "Address: 0x%#08X\n", trace.first);
    trace.second.dump(f);
  }
}

// Sim
Sim::Sim(const std::vector<std::vector<uint32_t>> &commands, const std::vector<uint32_t> &seg_va,
         uint32_t pc)
    : state_(commands, seg_va, pc) {}

void Sim::execute() {
  Timer timer;
  try {
    while (true) {
      state_.trace_cache.refer(decoder_, state_, state_.getPC()).execute(&state_);
      if (options::max_insts && stats::executed_insts >= options::max_insts)
        break;
    }
  } catch (SimException &e) {
    fprintf(options::log, "%s\n", e.what());
  }
  uint64_t time = timer.getMilliseconds();
  state_.dump(options::log);
  fprintf(options::log, "Some statistics:\n");
  fprintf(options::log, "Time: %lu ms, MIPS: %.3lf\n", time,
          (double)stats::executed_insts / (time * 1000));
  stats::PrintStatistics(options::log);
}
} // namespace sim
