#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "processors/RISC-V/riscv.h"

#include "../rv5s_float_no_fw_hz/rv5s_float_no_fw_hz_memwb.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The RV5S_FLOAT_MEMWB class
 * Extends the MEMWB_F register with a stalled port for pipeline stall
 * visualization in processors with hazard detection.
 */
template <unsigned XLEN>
class RV5S_FLOAT_MEMWB : public MEMWB_F<XLEN> {
public:
  RV5S_FLOAT_MEMWB(const std::string &name, SimComponent *parent)
      : MEMWB_F<XLEN>(name, parent) {
    CONNECT_REGISTERED_INPUT(stalled);
  }

  REGISTERED_INPUT(stalled, 1);
};

} // namespace core
} // namespace vsrtl
