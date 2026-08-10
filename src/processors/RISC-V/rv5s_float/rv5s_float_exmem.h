#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "processors/RISC-V/riscv.h"

#include "../rv5s_float_no_fw_hz/rv5s_float_no_fw_hz_exmem.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The RV5S_FLOAT_EXMEM class
 * Extends the EXMEM_F register with a stalled port for pipeline stall
 * visualization in processors with hazard detection.
 */
template <unsigned XLEN>
class RV5S_FLOAT_EXMEM : public EXMEM_F<XLEN> {
public:
  RV5S_FLOAT_EXMEM(const std::string &name, SimComponent *parent)
      : EXMEM_F<XLEN>(name, parent) {
    // We want stalling info to persist through clearing of the register, so
    // stalled register is always enabled and never cleared.
    CONNECT_REGISTERED_CLEN_INPUT(stalled, 0, 1);
  }

  REGISTERED_CLEN_INPUT(stalled, 1);
};

} // namespace core
} // namespace vsrtl
