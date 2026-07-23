#pragma once

#include "rv5mc_base_f.h"
#include "processors/RISC-V/rv_memory.h"
#include "processors/RISC-V/rv5mc/rv5mc_zextortruncate.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV5MC1MF : public RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState> {
  using FSMState = rv5mc1mf::FSMState;

public:
  RV5MC1MF(const ExtensionSetInfo &extensions)
      : RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>(
            extensions, "Multicycle 1-Memory RISC-V Processor with F extension") {
    // Memory
    this->pc_reg->out >> this->mem_addr_src->get(rv5mc1mf::MemAddrSrc::PC);
    this->ALU_out->out >> this->mem_addr_src->get(rv5mc1mf::MemAddrSrc::ALUOUT);
    this->control->mem_addr_src >> this->mem_addr_src->select;
    this->mem_addr_src->out >> this->memory->addr;
    this->memory->data_out >> this->mem_out->in;
    this->mem_out->out >> this->reg_src->get(RegWrSrc::MEMREAD);
    this->control->mem_write >> this->memory->wr_en;
    this->b->out >> this->memory->data_in;
    this->control->mem_ctrl >> this->memory->op;
    this->memory->setMemory(this->m_memory);

    // Decode
    this->memory->data_out >> this->ir_widthadjust->in;
    this->ir_widthadjust->out >> this->decode->instr;
    this->decode->instr << [this] { return this->memory->data_out.uValue(); };
  }

  MemoryAccess dataMemAccess() const override {
    return this->memToAccessInfo(memory);
  }
  MemoryAccess instrMemAccess() const override {
    auto instrAccess = this->memToAccessInfo(memory);
    instrAccess.type = MemoryAccess::Read;
    return instrAccess;
  }

  // Memories
  SUBCOMPONENT(
      memory, TYPE(RVMemory<RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>::XLEN,
                            RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>::XLEN>));

  SUBCOMPONENT(mem_addr_src,
               TYPE(EnumMultiplexer<rv5mc1mf::MemAddrSrc,
                                    RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>::XLEN>));
  SUBCOMPONENT(ir_widthadjust,
               TYPE(ZExtOrTruncate<RV5MCBaseF<XLEN_T, rv5mc1mf::RVMC1MControlF, rv5mc1mf::FSMState>::XLEN,
                                   c_RVInstrWidth>));
};

} // namespace core
} // namespace vsrtl
