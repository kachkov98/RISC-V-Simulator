#ifndef SIM_H
#define SIM_H

#include "common.h"
#include "decoder.h"
#include "mmu.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace sim {

class Trace {
private:
  struct ExecTraceDeleter {
    void operator()(ExecTracePtr trace);
  };
  using ExecTraceType = std::unique_ptr<ExecTrace, ExecTraceDeleter>;
  std::vector<ir::Inst> trace_;
  mutable ExecTraceType exec_trace_;
  mutable uint64_t exec_num_ = 0;
  mutable bool is_eligible_ = true;

public:
  Trace(const Decoder &decoder, State &state);
  void execute(State *state) const;
  void dump(FILE *f) const;
};

class TraceCache {
private:
  LRUCache<uint32_t, Trace> cache_;

public:
  TraceCache(size_t size) : cache_(size) {}
  const Trace &refer(const Decoder &decoder, State &state, uint32_t addr) {
    auto res = cache_.insert(addr, decoder, state);
    if (res.second)
      ++stats::trace_cache_misses;
    else
      ++stats::trace_cache_hits;
    return res.first;
  }
  void flush() { cache_.clear(); }
  void dump(FILE *f) const;
};

struct State {
  static TraceCache trace_cache;
  static MMU mmu;
  std::array<uint32_t, 32> regs;
  uint32_t pc;
  uint8_t *mem;
  uint64_t executed_insts;
  // TODO: system registers
  State(const std::vector<std::vector<uint32_t>> &commands, const std::vector<uint32_t> &seg_va,
        uint32_t pc);

  uint32_t getReg(ir::Reg reg) const {
    assert(reg < 32 && "Invalid register number");
    return regs[reg];
  }
  void setReg(ir::Reg reg, uint32_t val) {
    assert(reg < 32 && "Invalid register number");
    if (reg) {
      if (options::execution_log) {
        log("\t");
        reg.dump(options::log);
        log(": 0x%08X => 0x%08X\n", regs[reg], val);
      }
      regs[reg] = val;
    }
  }

  uint32_t getPC() const { return pc; }
  void setPC(uint32_t new_pc) {
    if (options::execution_log)
      log("\tPC: 0x%08X => 0x%08X\n", pc, new_pc);
    pc = new_pc;
  }

  template <typename T> T read(uint32_t va) { return mmu.load<T>(va, false); }
  template <typename T> void write(uint32_t va, T data) {
    if constexpr (std::is_integral_v<T>)
      if (options::execution_log)
        log("\tM: 0x%08X <= 0x%08X\n", va,
            sizeof(T) == 4 ? data : data & ((1 << (8 * sizeof(T))) - 1));
    mmu.store(va, data);
  }

  uint32_t getCmd(uint32_t va) { return mmu.load<uint32_t>(va, true); }

  static void flush() {
    mmu.flush();
    trace_cache.flush();
  }

  void dump(FILE *f) const;
};

class Sim {
private:
  Decoder decoder_;
  State state_;

public:
  Sim(const std::vector<uint32_t> &commands);
  Sim(const std::vector<std::vector<uint32_t>> &commands, const std::vector<uint32_t> &seg_va,
      uint32_t pc);
  Sim(const Sim &rhs) = delete;
  Sim &operator=(const Sim &rhs) = delete;
  void execute();
};
} // namespace sim

#endif
