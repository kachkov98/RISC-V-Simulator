#include "sim.h"
#include "jit.h"
#include "stats.h"

namespace sim {
// State
TraceCache State::trace_cache(options::cache_size);
MMU State::mmu;
uint32_t MMU::satp = 0x00000017u;

State::State(const std::vector<std::vector<uint32_t>> &commands,
             const std::vector<uint32_t> &seg_va, uint32_t pc)
    : pc(pc), mem(mmu.getMem()), executed_insts(0) {
  regs.fill(0u);
  // Allocate 2 pages for stack
  setReg(isa::Regs::sp, mmu.getMemSize() - 2 * MMU::pagesize);
  // put segment in pmem_ (pa = va)
  int i = 0;
  for (auto va : seg_va) {
    if (va + commands[i].size() * 4 > mmu.getMemSize())
      throw SimException("Not enough memory to load segment");
    memcpy(mmu.getMemPtr<uint8_t>(va), commands[i].data(), commands[i].size() * 4);
    ++i;
  }
}

void State::dump(FILE *f) const {
  fprintf(f, "Processor state:\n");
  for (uint8_t i = 0; i < 32; ++i) {
    ir::Reg(i).dump(f);
    fprintf(f, ": 0x%08X%c", regs[i], (i + 1) % 4 ? '\t' : '\n');
  }
  fprintf(f, "PC: 0x%08X\n", pc);
}

// Trace
Trace::Trace(const Decoder &decoder, State &state) {
  uint32_t address = state.getPC();
  while (true) {
    ir::Inst inst = decoder.decode(state.getCmd(address));
    trace_.push_back(inst);
    if (inst.isTerminator())
      break;
    address += 4;
  }
  trace_.shrink_to_fit();
  if (options::execution_log) {
    fprintf(options::log, "Fetching trace:\n");
    dump(options::log);
  }
}

void Trace::execute(State *state) const {
  if (options::jit && is_eligible_ && !exec_trace_ && options::jit_threshold == exec_num_++) {
    if (options::execution_log)
      log("Attempt to translate trace...\n");
    exec_trace_ = ExecTraceType(jit::Translator(trace_).getFunc());
    if (!exec_trace_) {
      is_eligible_ = false;
      ++stats::jit_failed_translations;
      if (options::execution_log)
        log("Fail\n");
    } else {
      ++stats::jit_finished_translations;
      if (options::execution_log)
        log("Success\n");
    }
  }
  if (options::execution_log)
    log("Executing trace...\n");
  if (exec_trace_) {
    (*exec_trace_)(state);
    ++stats::translated_executions;
  } else {
    trace_.data()->exec(trace_.data(), state);
    state->executed_insts += trace_.size();
    ++stats::interpreted_executions;
  }
  if (options::execution_log)
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
      State::trace_cache.refer(decoder_, state_, state_.getPC()).execute(&state_);
      if (options::max_insts && state_.executed_insts >= options::max_insts)
        break;
    }
  } catch (SimException &e) {
    fprintf(options::log, "%s\n", e.what());
  }
  uint64_t time = timer.getMicroseconds();
  state_.dump(options::log);
  log("Some statistics:\n");
  log("Insts num: %lu, Time: %lu ms, MIPS: %.3lf\n", state_.executed_insts, time / 1000,
      (double)state_.executed_insts / time);
  stats::PrintStatistics(options::log);
}
} // namespace sim
