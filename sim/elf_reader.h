#ifndef ELF_READER_H
#define ELF_READER_H

#include "common.h"
#include <fcntl.h>
#include <gelf.h>
#include <string>
#include <unistd.h>
#include <vector>

namespace elf {

class Elf_reader {
private:
  Elf *e_ = nullptr;
  int fd_ = -1;
  GElf_Phdr phdr_;
  size_t phdrnum_ = 0;
  size_t cur_phdr_ = 0;
  uint64_t entry_ = 0;

public:
  Elf_reader() {}
  Elf_reader(const Elf_reader &) = delete;
  Elf_reader(Elf_reader &&) = delete;
  void init(const char *filename);
  void init(const std::string &filename) { return init(filename.c_str()); }
  void clear();
  bool load(std::vector<std::vector<uint32_t>> &cmds /* out */,
            std::vector<uint32_t> &seg_va /* out */, uint32_t &offset /* out */);
  Elf_reader &operator=(const Elf_reader &) = delete;
  ~Elf_reader();
};
} // namespace elf

#endif
