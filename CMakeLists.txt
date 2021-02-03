cmake_minimum_required(VERSION 3.8)

project(riscvsim)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(ELF_LIB elf)
find_library(M_LIB m)

option(ASMJIT_STATIC "" ON)
option(ASMJIT_NO_BUILDER "" ON)
option(ASMJIT_NO_COMPILER "" ON)
add_subdirectory(asmjit)

set(SIM_DIR sim)
set(SIM_SRC
    ${SIM_DIR}/decoder.cpp
    ${SIM_DIR}/elf_reader.cpp
    ${SIM_DIR}/exec_inst.cpp
    ${SIM_DIR}/ir.cpp
    ${SIM_DIR}/isa_desc.cpp
    ${SIM_DIR}/jit.cpp
    ${SIM_DIR}/main.cpp
    ${SIM_DIR}/options.cpp
    ${SIM_DIR}/sim.cpp
    ${SIM_DIR}/stats.cpp
    ${SIM_DIR}/syscall.cpp
    ${SIM_DIR}/translate_inst.cpp)

add_executable(${PROJECT_NAME} ${SIM_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE asmjit/src)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} asmjit ${ELF_LIB} ${M_LIB})
