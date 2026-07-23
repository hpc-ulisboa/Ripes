#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_register.h"

#include "processors/RISC-V/riscv.h"

#include "../rv5s_float_no_fw_hz/rv5s_float_no_fw_hz_idex.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The RV5S_FLOAT_IDEX class
 * A specialization of the IDEX_F stage separating register utilized by the
 * floating-point processors with hazard detection and/or forwarding. Storage of
 * register read indices (for forwarding) and opcode (for ECALL hazard detection)
 * is added on top of the IDEX_F fields. A stalled register is also added for
 * pipeline stall visualization.
 */
template <unsigned XLEN>
class RV5S_FLOAT_IDEX : public IDEX_F<XLEN> {
public:
  RV5S_FLOAT_IDEX(const std::string &name, SimComponent *parent)
      : IDEX_F<XLEN>(name, parent) {
    CONNECT_REGISTERED_CLEN_INPUT(rd_reg1_idx, this->clear, this->enable);
    CONNECT_REGISTERED_CLEN_INPUT(rd_reg2_idx, this->clear, this->enable);
    CONNECT_REGISTERED_CLEN_INPUT(opcode, this->clear, this->enable);

    // We want stalling info to persist through clearing of the register, so
    // stalled register is always enabled and never cleared.
    CONNECT_REGISTERED_CLEN_INPUT(stalled, 0, 1);
  }

  REGISTERED_CLEN_INPUT(rd_reg1_idx, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(rd_reg2_idx, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(opcode, enumBitWidth<RVInstr>());

  REGISTERED_CLEN_INPUT(stalled, 1);
};

} // namespace core
} // namespace vsrtl
