#pragma once

#include "processors/RISC-V/riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The HazardUnit_FLOAT class
 * A hazard detection unit for 5-stage pipelined processors with both integer
 * and floating-point register files. Detects RAW data hazards on both register
 * files and stalls the pipeline when necessary.
 *
 * This unit supports two modes of operation depending on the processor variant:
 *
 * 1. With forwarding (rv5s_float): Only load-use hazards need stalling, since
 *    the forwarding unit handles all other RAW hazards. A load-use hazard occurs
 *    when the EX stage is executing a load (LW or FLW) and the ID stage reads
 *    the same register from the same register file.
 *
 * 2. Without forwarding (rv5s_float_no_fw): ALL data hazards between
 *    consecutive instructions (i+1 vs i, i+2 vs i) must be detected and
 *    stalled, since there is no forwarding to resolve them. This requires
 *    checking both integer and float write enables from EX and MEM stages
 *    against the ID stage's register file sources.
 *
 * Register file awareness: The unit checks reg_do_write (integer) and
 * regF_do_write (float) independently, and matches them against the ID
 * stage's register file source selection (reg1_do_read_src, reg2_do_read_src)
 * to avoid false-positive hazards on same-index different-file conflicts.
 */
class HazardUnit_FLOAT : public Component {
public:
  HazardUnit_FLOAT(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    hazardFEEnable << [this] { return !hasHazard(); };
    hazardIDEXEnable << [this] { return !hasEcallHazard(); };
    hazardEXMEMClear << [this] { return hasEcallHazard(); };
    hazardIDEXClear << [this] { return hasDataOrLoadUseHazard(); };
    stallEcallHandling << [this] { return hasEcallHazard(); };
  }

  // --- ID stage inputs (from decode/control) ---
  INPUTPORT(id_reg1_idx, c_RVRegsBits);
  INPUTPORT(id_reg2_idx, c_RVRegsBits);
  INPUTPORT(id_reg1_src, 2); // ImmRegFileSrc: 0=INTEGER, 1=FLOAT, 2=IMMEDIATE
  INPUTPORT(id_reg2_src, 1); // RegFileSrc: 0=INTEGER, 1=FLOAT
  INPUTPORT(id_do_branch, 1);
  INPUTPORT(id_mem_do_write, 1);
  INPUTPORT_ENUM(id_alu_op_ctrl_2, AluSrc2);

  // --- EX stage inputs (from IDEX register) ---
  INPUTPORT(ex_reg_wr_idx, c_RVRegsBits);
  INPUTPORT(ex_do_reg_write, 1);   // Integer register file write enable
  INPUTPORT(ex_regF_do_write, 1);  // Float register file write enable
  INPUTPORT(ex_do_mem_read_en, 1); // Load signal (LW or FLW)
  INPUTPORT(ex_do_jump, 1);
  INPUTPORT(ex_branch_taken, 1);
  INPUTPORT_ENUM(ex_opcode, RVInstr);

  // --- MEM stage inputs (from EXMEM register) ---
  INPUTPORT(mem_reg_wr_idx, c_RVRegsBits);
  INPUTPORT(mem_do_reg_write, 1);  // Integer write enable
  INPUTPORT(mem_regF_do_write, 1); // Float write enable

  // --- WB stage inputs (from MEMWB register) ---
  INPUTPORT(wb_do_reg_write, 1);  // Integer write enable
  INPUTPORT(wb_regF_do_write, 1); // Float write enable

  // Hazard Front End enable: Low when stalling the front end (shall be
  // connected to a register 'enable' input port).
  OUTPUTPORT(hazardFEEnable, 1);

  // Hazard IDEX enable: Low when stalling due to an ECALL hazard
  OUTPUTPORT(hazardIDEXEnable, 1);

  // EXMEM clear: High when an ECALL hazard is detected
  OUTPUTPORT(hazardEXMEMClear, 1);
  // IDEX clear: High when a data or load-use hazard is detected
  OUTPUTPORT(hazardIDEXClear, 1);

  // Stall Ecall Handling: High whenever we are about to handle an ecall, but
  // have outstanding writes in the pipeline which must be committed to the
  // register file before handling the ecall.
  OUTPUTPORT(stallEcallHandling, 1);

private:
  bool hasHazard() { return hasDataOrLoadUseHazard() || hasEcallHazard(); }

  bool hasDataOrLoadUseHazard() {
    return hasDataHazardExMem() || hasLoadUseHazard();
  }

  bool hasDataHazardExMem() { return hasDataHazardEx() || hasDataHazardMem(); }

  // --- Load-use hazard detection ---
  // A load-use hazard occurs when the EX stage is executing a load and the ID
  // stage reads the same register from the same register file.
  bool hasLoadUseHazard() const {
    if (!ex_do_mem_read_en.uValue())
      return false;

    const unsigned exidx = ex_reg_wr_idx.uValue();
    if (exidx == 0)
      return false;

    const bool exIsFltLoad = ex_regF_do_write.uValue();
    const bool exIsIntLoad = !exIsFltLoad;

    const unsigned idx1 = id_reg1_idx.uValue();
    const unsigned idx2 = id_reg2_idx.uValue();
    const bool r1FromInt =
        id_reg1_src.uValue() == static_cast<unsigned>(ImmRegFileSrc::INTEGER);
    const bool r1FromFlt =
        id_reg1_src.uValue() == static_cast<unsigned>(ImmRegFileSrc::FLOAT);
    const bool r2FromInt =
        id_reg2_src.uValue() == static_cast<unsigned>(RegFileSrc::INTEGER);
    const bool r2FromFlt =
        id_reg2_src.uValue() == static_cast<unsigned>(RegFileSrc::FLOAT);

    // Check reg1: integer load hazards against integer readers, float loads
    // against float readers
    const bool r1Hazard =
        (r1FromInt && exIsIntLoad && exidx == idx1) ||
        (r1FromFlt && exIsFltLoad && exidx == idx1);

    // Check reg2: same logic, but only if the instruction reads reg2 from a
    // register file (not immediate)
    const bool r2Hazard =
        (r2FromInt && exIsIntLoad && exidx == idx2) ||
        (r2FromFlt && exIsFltLoad && exidx == idx2);

    return r1Hazard || r2Hazard;
  }

  // --- ECALL hazard detection ---
  // ECALL implicitly depends on all registers (both integer and float), so
  // stall if any outstanding writes in MEM or WB.
  bool hasEcallHazard() const {
    const bool isEcall = ex_opcode.eValue<RVInstr>() == RVInstr::ECALL;
    return isEcall && (mem_do_reg_write.uValue() ||
                       mem_regF_do_write.uValue() ||
                       wb_do_reg_write.uValue() ||
                       wb_regF_do_write.uValue());
  }

  // --- Data hazard detection (for no-forwarding variant) ---

  // Check data hazards between EX stage (i+1) and ID stage (i)
  bool hasDataHazardEx() const {
    const unsigned exWriteIdx = ex_reg_wr_idx.uValue();
    const bool exIntWrite = ex_do_reg_write.uValue();
    const bool exFltWrite = ex_regF_do_write.uValue();
    return hasDataHazard(exWriteIdx, exIntWrite, exFltWrite);
  }

  // Check data hazards between MEM stage (i+2) and ID stage (i)
  bool hasDataHazardMem() const {
    const unsigned memWriteIdx = mem_reg_wr_idx.uValue();
    const bool memIntWrite = mem_do_reg_write.uValue();
    const bool memFltWrite = mem_regF_do_write.uValue();
    return hasDataHazard(memWriteIdx, memIntWrite, memFltWrite);
  }

  /**
   * @brief Core data hazard check for a single write stage.
   * Checks whether a write to register @p writeIdx in a upstream stage
   * conflicts with a read in the ID stage, considering which register file
   * each operation targets/reads.
   *
   * @param writeIdx The register index being written by the upstream instruction
   * @param intWrite Whether the upstream instruction writes to the integer RF
   * @param fltWrite Whether the upstream instruction writes to the float RF
   */
  bool hasDataHazard(unsigned writeIdx, bool intWrite, bool fltWrite) const {
    // No hazard if destination is x0/f0 (x0 is hardwired to zero, f0 is a
    // regular register but writeIdx==0 with no write is still safe to skip)
    if (writeIdx == 0)
      return false;

    // If a jump is in EX, the pipeline will be flushed; no stall needed
    if (ex_do_jump.uValue())
      return false;

    // If a branch is taken in EX, the pipeline will be flushed
    if (ex_branch_taken.uValue())
      return false;

    const unsigned idx1 = id_reg1_idx.uValue();
    const unsigned idx2 = id_reg2_idx.uValue();

    // Determine which register file the ID stage reads from
    const bool r1FromInt =
        id_reg1_src.uValue() == static_cast<unsigned>(ImmRegFileSrc::INTEGER);
    const bool r1FromFlt =
        id_reg1_src.uValue() == static_cast<unsigned>(ImmRegFileSrc::FLOAT);
    const bool r2FromInt =
        id_reg2_src.uValue() == static_cast<unsigned>(RegFileSrc::INTEGER);
    const bool r2FromFlt =
        id_reg2_src.uValue() == static_cast<unsigned>(RegFileSrc::FLOAT);

    // Determine whether reg2 is actually used as a register source.
    // For integer instructions: reg2 is used when ALU source 2 is REG2, or
    // the instruction is a branch or memory write.
    const bool idx2isReg =
        id_alu_op_ctrl_2.eValue<AluSrc2>() == AluSrc2::REG2;
    const bool isBranch = id_do_branch.uValue();
    const bool isMemWrite = id_mem_do_write.uValue();
    const bool usesIntReg2 = idx2isReg || isBranch || isMemWrite;
    // For float instructions: reg2 is always read from the float register file
    // when reg2_do_read_src selects FLOAT (FADD, FSUB, FSW, etc.)

    // Check reg1 hazard: match index AND same register file AND upstream writes
    // to that file
    const bool r1IntHazard = r1FromInt && intWrite && (writeIdx == idx1);
    const bool r1FltHazard = r1FromFlt && fltWrite && (writeIdx == idx1);

    // Check reg2 hazard: match index AND same register file AND upstream writes
    // to that file AND instruction actually reads reg2 from that file
    const bool r2IntHazard =
        r2FromInt && intWrite && usesIntReg2 && (writeIdx == idx2);
    const bool r2FltHazard = r2FromFlt && fltWrite && (writeIdx == idx2);

    return r1IntHazard || r1FltHazard || r2IntHazard || r2FltHazard;
  }
};
} // namespace core
} // namespace vsrtl
