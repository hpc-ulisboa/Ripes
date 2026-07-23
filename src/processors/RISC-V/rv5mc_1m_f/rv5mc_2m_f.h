#pragma once

#include "rv5mc_base_f.h"
#include "processors/RISC-V/rv_memory.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV5MC2MF : public RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState> {
  using FSMState = rv5mc1mf::FSMState;

public:
  RV5MC2MF(const ExtensionSetInfo &extensions)
      : RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>(
            extensions, "Multicycle 2-Memory RISC-V Processor with F extension") {
    // Instruction memory
    this->pc_reg->out >> this->instr_mem->addr;
    this->instr_mem->setMemory(this->m_memory);

    // Decode
    this->instr_mem->data_out >> this->decode->instr;

    // Data memory
    this->data_mem->data_out >> this->mem_out->in;
    this->mem_out->out >> this->reg_src->get(RegWrSrc::MEMREAD);
    this->ALU_out->out >> this->data_mem->addr;
    this->control->mem_write >> this->data_mem->wr_en;
    this->b->out >> this->data_mem->data_in;
    this->control->mem_ctrl >> this->data_mem->op;
    this->data_mem->setMemory(this->m_memory);
  }

  MemoryAccess dataMemAccess() const override {
    return this->memToAccessInfo(data_mem);
  }
  MemoryAccess instrMemAccess() const override {
    auto instrAccess = this->memToAccessInfo(instr_mem);
    instrAccess.type = MemoryAccess::Read;
    return instrAccess;
  }

  // Memories
  SUBCOMPONENT(instr_mem,
               TYPE(ROM<RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>::XLEN,
                               c_RVInstrWidth>));
  SUBCOMPONENT(
      data_mem,
      TYPE(RVMemory<RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>::XLEN,
                     RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>::XLEN>));
};

} // namespace core
} // namespace vsrtl
