# RISC-V-Simulator
## Functional RV32M simulator with MMU and JIT

### Build
<pre>
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
cmake --build .
</pre>

### External libraries
* [asmjit](https://github.com/asmjit/asmjit)
* [robin-hood-hashing](https://github.com/martinus/robin-hood-hashing)

### Usage
<pre>
./riscvsim -help
Sim options:
-verbose (value = false) - enable logging of instructions execution
-execution-log (value = false) - enable trace exectuion logging
-log-file [filename] (value = <empty>) - write excution output to file
-mem-pages [pages num] (value = 128) - number of allocated pages
-cache-size [size] (value = 256) - number of saved traces in LRU cache
-itlb-size [size] (value = 256) - number of instruction TLB entries
-dtlb-size [size] (value = 256) - number of data TLB entries
-jit (value = false) - enable just-in-time translation
-jit-threshold [exec num] (value = 100) - number of trace executions when JIT is applied
-jit-log (value = false) - enable logging of just-in-time-translation
-max-insts [insts num] (value = 2000000) - number of instructions, after which simulator stops
-elf [filename] (value = <empty>) - elf file to execute
</pre>

### Some benchmark results
CPU: Core i3-4030u (Haswell @ 1.9GHz)  
Queens:  
<pre>
./riscvsim -elf ../tests/queens/queens-opt -jit
n=1  solution_count=1
n=2  solution_count=0
n=3  solution_count=0
n=4  solution_count=2
n=5  solution_count=10
n=6  solution_count=4
n=7  solution_count=40
n=8  solution_count=92
Successfully finished!

Processor state:
x00: 0x00000000 x01: 0x00010450 x02: 0x0007DFF0 x03: 0x000231E8
x04: 0x00000000 x05: 0x000105A0 x06: 0x00000013 x07: 0x00000000
x08: 0x00000000 x09: 0x00000000 x10: 0x00000000 x11: 0x00000000
x12: 0x00000000 x13: 0x00000000 x14: 0x00000000 x15: 0x00000000
x16: 0xA0000000 x17: 0x0000005D x18: 0x00000000 x19: 0x00000000
x20: 0x00000000 x21: 0x00000000 x22: 0x00000000 x23: 0x00000000
x24: 0x00000000 x25: 0x00000000 x26: 0x00000000 x27: 0x00000000
x28: 0x00000000 x29: 0x00000009 x30: 0x00000001 x31: 0x00000009
PC: 0x0001C1B8
Some statistics:
Insts num: 1936886, Time: 4 ms, MIPS: 407.422
Trace cache hits: 112498
Trace cache misses: 337
iTLB hits: 0
iTLB misses: 0
dTLB load hits: 0
dTLB load misses: 0
dTLB store hits: 0
dTLB store misses: 0
Number of successful JIT translations: 21
Number of failed JIT translations: 0
Number of interpreted trace executions: 4525
Number of translated (JIT) trace executions: 108309
</pre>
Dhrystone:  
<pre>
./riscvsim -elf ../tests/dhrystone/dhrystone -max-insts 2000000000 -jit
Dhrystone(1.1-mc), 10000000 passes, 7513346 microseconds, 755 DMIPS
Successfully finished!

Processor state:
x00: 0x00000000 x01: 0x0001078C x02: 0x0007DFE0 x03: 0x00023B58
x04: 0x00000000 x05: 0x00010F54 x06: 0x00000012 x07: 0x00000000
x08: 0x00000000 x09: 0x00000000 x10: 0x00000000 x11: 0x00000000
x12: 0x00000000 x13: 0x00000000 x14: 0x00000000 x15: 0x00000000
x16: 0xA0000000 x17: 0x0000005D x18: 0x00000000 x19: 0x00000000
x20: 0x00000000 x21: 0x00000000 x22: 0x00000000 x23: 0x00000000
x24: 0x00000000 x25: 0x00000000 x26: 0x00000000 x27: 0x00000000
x28: 0x00000000 x29: 0x4E4F5453 x30: 0x59524844 x31: 0x00000012
PC: 0x0001C2EC
Some statistics:
Insts num: 1730010297, Time: 7513 ms, MIPS: 230.241
Trace cache hits: 240001067
Trace cache misses: 367
iTLB hits: 0
iTLB misses: 0
dTLB load hits: 0
dTLB load misses: 0
dTLB store hits: 0
dTLB store misses: 0
Number of successful JIT translations: 25
Number of failed JIT translations: 0
Number of interpreted trace executions: 3840
Number of translated (JIT) trace executions: 239997593
</pre>
