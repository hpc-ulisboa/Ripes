#pragma once

#include "processors/RISC-V/riscv.h"

#include "VSRTL/core/vsrtl_component.h"

// Reuse the ForwardingSrc enum defined in the integer forwarding unit.
#include "../rv5s/rv5s_forwardingunit.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The ForwardingUnit_FLOAT class
 * A forwarding unit for 5-stage pipelined processors with both integer and
 * floating-point register files. Produces independent forwarding controls for
 * the integer ALU path (r1, r2) and the floating-point FPU path (f1, f2, f3).
 *
 * The integer path checks reg_do_write (integer register file write enable)
 * from MEM/WB stages. The float path checks regF_do_write (float register
 * file write enable) from MEM/WB stages. Both paths use the same register
 * index, which is shared between the two register files in the RISC-V encoding.
 *
 * Single-cycle FPU assumption: FPU results are available in the EX stage,
 * same as ALU results, so the forwarding source is always the ALU/FPU result
 * muxed through exmem_reg->alures_out.
 */
class ForwardingUnit_FLOAT : public Component {
public:
  ForwardingUnit_FLOAT(const std::string &name, SimComponent *parent)
      : Component(name, parent) {

    // --- Integer ALU forwarding (checks integer reg_do_write) ---

    alu_reg1_forwarding_ctrl << [this] {
      const auto idx = id_reg1_idx.uValue();
      if (idx == 0) {
        return ForwardingSrc::IdStage;
      } else if (idx == mem_int_wr_idx.uValue() &&
                 mem_int_wr_en.uValue()) {
        return ForwardingSrc::MemStage;
      } else if (idx == wb_int_wr_idx.uValue() && wb_int_wr_en.uValue()) {
        return ForwardingSrc::WbStage;
      } else {
        return ForwardingSrc::IdStage;
      }
    };

    alu_reg2_forwarding_ctrl << [this] {
      const auto idx = id_reg2_idx.uValue();
      if (idx == 0) {
        return ForwardingSrc::IdStage;
      } else if (idx == mem_int_wr_idx.uValue() &&
                 mem_int_wr_en.uValue()) {
        return ForwardingSrc::MemStage;
      } else if (idx == wb_int_wr_idx.uValue() && wb_int_wr_en.uValue()) {
        return ForwardingSrc::WbStage;
      } else {
        return ForwardingSrc::IdStage;
      }
    };

    // --- Float FPU forwarding (checks float regF_do_write) ---

    fpu_reg1_forwarding_ctrl << [this] {
      const auto idx = id_reg1_idx.uValue();
      if (idx == 0) {
        return ForwardingSrc::IdStage;
      } else if (idx == mem_flt_wr_idx.uValue() &&
                 mem_flt_wr_en.uValue()) {
        return ForwardingSrc::MemStage;
      } else if (idx == wb_flt_wr_idx.uValue() && wb_flt_wr_en.uValue()) {
        return ForwardingSrc::WbStage;
      } else {
        return ForwardingSrc::IdStage;
      }
    };

    fpu_reg2_forwarding_ctrl << [this] {
      const auto idx = id_reg2_idx.uValue();
      if (idx == 0) {
        return ForwardingSrc::IdStage;
      } else if (idx == mem_flt_wr_idx.uValue() &&
                 mem_flt_wr_en.uValue()) {
        return ForwardingSrc::MemStage;
      } else if (idx == wb_flt_wr_idx.uValue() && wb_flt_wr_en.uValue()) {
        return ForwardingSrc::WbStage;
      } else {
        return ForwardingSrc::IdStage;
      }
    };

    // f3 forwarding uses rd_reg3_idx (rs3 = bits [31:27], for FMADD/FMSUB)
    fpu_reg3_forwarding_ctrl << [this] {
      const auto idx = id_reg3_idx.uValue();
      if (idx == 0) {
        return ForwardingSrc::IdStage;
      } else if (idx == mem_flt_wr_idx.uValue() &&
                 mem_flt_wr_en.uValue()) {
        return ForwardingSrc::MemStage;
      } else if (idx == wb_flt_wr_idx.uValue() && wb_flt_wr_en.uValue()) {
        return ForwardingSrc::WbStage;
      } else {
        return ForwardingSrc::IdStage;
      }
    };
  }

  // --- ID stage register indices (from IDEX rd_reg outputs) ---
  INPUTPORT(id_reg1_idx, c_RVRegsBits);
  INPUTPORT(id_reg2_idx, c_RVRegsBits);
  INPUTPORT(id_reg3_idx, c_RVRegsBits);

  // --- MEM stage integer write info ---
  INPUTPORT(mem_int_wr_idx, c_RVRegsBits);
  INPUTPORT(mem_int_wr_en, 1);

  // --- MEM stage float write info ---
  INPUTPORT(mem_flt_wr_idx, c_RVRegsBits);
  INPUTPORT(mem_flt_wr_en, 1);

  // --- WB stage integer write info ---
  INPUTPORT(wb_int_wr_idx, c_RVRegsBits);
  INPUTPORT(wb_int_wr_en, 1);

  // --- WB stage float write info ---
  INPUTPORT(wb_flt_wr_idx, c_RVRegsBits);
  INPUTPORT(wb_flt_wr_en, 1);

  // --- Integer ALU forwarding controls ---
  OUTPUTPORT_ENUM(alu_reg1_forwarding_ctrl, ForwardingSrc);
  OUTPUTPORT_ENUM(alu_reg2_forwarding_ctrl, ForwardingSrc);

  // --- Float FPU forwarding controls ---
  OUTPUTPORT_ENUM(fpu_reg1_forwarding_ctrl, ForwardingSrc);
  OUTPUTPORT_ENUM(fpu_reg2_forwarding_ctrl, ForwardingSrc);
  OUTPUTPORT_ENUM(fpu_reg3_forwarding_ctrl, ForwardingSrc);
};
} // namespace core
} // namespace vsrtl
