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

class State {
public:
  std::array<uint32_t, 32> regs_;
  uint32_t pc_;
  MMU mmu_;
  uint8_t *mem_;
  // TODO: system registers
  TraceCache trace_cache;
  uint32_t satp = 0x00000017u; // reg
  State(const std::vector<std::vector<uint32_t>> &commands, const std::vector<uint32_t> &seg_va,
        uint32_t pc)
      : pc_(pc), mmu_(satp), mem_(mmu_.getMemPtr<uint8_t>(0)), trace_cache(options::cache_size) {
    regs_.fill(0u);
    regs_[2] = mmu_.getMemSize() - 2 * MMU::pagesize;
    // put segment in pmem_ (pa = va)
    int i = 0;
    for (auto va : seg_va) {
      if (va + commands[i].size() * 4 > mmu_.getMemSize())
        throw SimException("Not enough memory to load segment");
      memcpy(mmu_.getMemPtr<uint8_t>(va), commands[i].data(), commands[i].size() * 4);
      ++i;
    }
  }
  uint32_t getReg(ir::Reg reg) const {
    assert(reg < 32 && "Invalid register number");
    return regs_[reg];
  }
  void setReg(ir::Reg reg, uint32_t val) {
    assert(reg < 32 && "Invalid register number");
    if (reg) {
      if (options::verbose) {
        fprintf(options::log, "\t");
        reg.dump(options::log);
        fprintf(options::log, ": 0x%08X => 0x%08X\n", regs_[reg], val);
      }
      regs_[reg] = val;
    }
  }

  uint32_t getPC() const { return pc_; }
  void setPC(uint32_t pc) {
    log("\tPC: 0x%08X => 0x%08X\n", pc_, pc);
    pc_ = pc;
  }

  void dump(FILE *f) const;

  uint32_t getCmd(uint32_t va) { return mmu_.load(va, sizeof(va)); }

  void write(uint32_t va, uint8_t nbytes, uint32_t data) {
    log("\tM: 0x%08X <= 0x%08X\n", va, nbytes == 4 ? data : data & ((1 << (8 * nbytes)) - 1));
    mmu_.store(va, nbytes, data);
  }

  uint32_t read(uint32_t va, uint8_t nbytes) { return mmu_.load(va, nbytes, false); }

  void flush() {
    trace_cache.flush();
    mmu_.flush();
  }
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
