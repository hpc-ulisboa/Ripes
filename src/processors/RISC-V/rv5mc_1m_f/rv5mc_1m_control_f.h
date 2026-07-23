#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"
#include "processors/RISC-V/riscv.h"
#include <magic_enum/magic_enum.hpp>

namespace vsrtl {
namespace core {
using namespace Ripes;

namespace rv5mc1mf {
enum class AluSrc1 { REG1, PC, PCOLD };
enum class AluSrc2 { REG2, IMM, PC_INC };
enum class ALUControl { ADD, SUB, INSTRUCTION_DEPENDENT };
enum class MemAddrSrc { PC, ALUOUT };

enum class FSMState {
  IF,
  ID,

  EX,
  EXJALR,
  EXBRANCH,
  EXALUR,
  EXALUI,
  EXMEMOP,
  EXECALL,

  MEM,

  WBJ,
  MEMLOAD,
  MEMSTORE,

  WB,

  WBALU,
  WBMEMLOAD,
};
struct StateSignals {
  bool ir_write = false;
  bool pc_write = false;
  bool branch = false;
  PcSrc pc_src = PcSrc::ALU;
  bool reg_write = false;
  bool regF_write = false;
  RegWrSrc reg_src = RegWrSrc::ALURES;
  AluSrc1 alu_op1_src = AluSrc1::PC;
  AluSrc2 alu_op2_src = AluSrc2::IMM;
  ALUControl alu_control = ALUControl::ADD;
  bool mem_write = false;
  MemAddrSrc mem_addr_src = MemAddrSrc::PC;
  bool ecall = false;
};
using TransitionFunc = FSMState (*)(RVInstr);

struct StateInfo {
  StateSignals outs;
  TransitionFunc transitions;
};

class RVMC1MControlF : public Component {
public:
  /* clang-format off */
  std::vector<StateInfo> states;

  static CompOp do_comp_ctrl(RVInstr opc) {
    switch(opc){
    case RVInstr::BEQ: return CompOp::EQ;
    case RVInstr::BNE: return CompOp::NE;
    case RVInstr::BLT: return CompOp::LT;
    case RVInstr::BGE: return CompOp::GE;
    case RVInstr::BLTU: return CompOp::LTU;
    case RVInstr::BGEU: return CompOp::GEU;
    default: return CompOp::NOP;
    }
  }

  static ALUOp do_alu_ctrl(RVInstr opc) {
    switch(opc) {
    case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU:
    case RVInstr::SB: case RVInstr::SH: case RVInstr::SW: case RVInstr::LWU: case RVInstr::LD:
    case RVInstr::SD:
    case RVInstr::FLW: case RVInstr::FSW:
      return ALUOp::ADD;
    case RVInstr::LUI:
      return ALUOp::LUI;
    case RVInstr::JAL: case RVInstr::JALR: case RVInstr::AUIPC:
    case RVInstr::ADD: case RVInstr::ADDI:
    case RVInstr::BEQ: case RVInstr::BNE: case RVInstr::BLT:
    case RVInstr::BGE: case RVInstr::BLTU: case RVInstr::BGEU:
      return ALUOp::ADD;
    case RVInstr::SUB: return ALUOp::SUB;
    case RVInstr::SLT: case RVInstr::SLTI:
      return ALUOp::LT;
    case RVInstr::SLTU: case RVInstr::SLTIU:
      return ALUOp::LTU;
    case RVInstr::XOR: case RVInstr::XORI:
      return ALUOp::XOR;
    case RVInstr::OR: case RVInstr::ORI:
      return ALUOp::OR;
    case RVInstr::AND: case RVInstr::ANDI:
      return ALUOp::AND;
    case RVInstr::SLL: case RVInstr::SLLI:
      return ALUOp::SL;
    case RVInstr::SRL: case RVInstr::SRLI:
      return ALUOp::SRL;
    case RVInstr::SRA: case RVInstr::SRAI:
      return ALUOp::SRA;
    case RVInstr::MUL   : return ALUOp::MUL;
    case RVInstr::MULH  : return ALUOp::MULH;
    case RVInstr::MULHU : return ALUOp::MULHU;
    case RVInstr::MULHSU: return ALUOp::MULHSU;
    case RVInstr::DIV   : return ALUOp::DIV;
    case RVInstr::DIVU  : return ALUOp::DIVU;
    case RVInstr::REM   : return ALUOp::REM;
    case RVInstr::REMU  : return ALUOp::REMU;
    case RVInstr::ADDIW : return ALUOp::ADDW;
    case RVInstr::SLLIW : return ALUOp::SLW;
    case RVInstr::SRLIW : return ALUOp::SRLW;
    case RVInstr::SRAIW : return ALUOp::SRAW;
    case RVInstr::ADDW  : return ALUOp::ADDW ;
    case RVInstr::SUBW  : return ALUOp::SUBW ;
    case RVInstr::SLLW  : return ALUOp::SLW ;
    case RVInstr::SRLW  : return ALUOp::SRLW ;
    case RVInstr::SRAW  : return ALUOp::SRAW ;
    case RVInstr::MULW  : return ALUOp::MULW ;
    case RVInstr::DIVW  : return ALUOp::DIVW ;
    case RVInstr::DIVUW : return ALUOp::DIVUW;
    case RVInstr::REMW  : return ALUOp::REMW ;
    case RVInstr::REMUW : return ALUOp::REMUW;
    default: return ALUOp::NOP;
    }
  }

  static MemOp do_mem_ctrl(RVInstr opc) {
    switch(opc){
      case RVInstr::SB: return MemOp::SB;
      case RVInstr::SH: return MemOp::SH;
      case RVInstr::SW: return MemOp::SW;
      case RVInstr::SD: return MemOp::SD;
      case RVInstr::LB: return MemOp::LB;
      case RVInstr::LH: return MemOp::LH;
      case RVInstr::LW: return MemOp::LW;
      case RVInstr::LD: return MemOp::LD;
      case RVInstr::LBU: return MemOp::LBU;
      case RVInstr::LHU: return MemOp::LHU;
      case RVInstr::LWU: return MemOp::LWU;
      case RVInstr::FLW: return MemOp::LW;
      default:
        return MemOp::NOP;
    }
  }

  static bool doesWriteIntReg(RVInstr opc) {
    switch(opc) {
    case RVInstr::ADD: case RVInstr::SUB: case RVInstr::SLT: case RVInstr::SLTU:
    case RVInstr::AND: case RVInstr::OR: case RVInstr::XOR:
    case RVInstr::SLL: case RVInstr::SRL: case RVInstr::SRA:
    case RVInstr::MUL: case RVInstr::MULH: case RVInstr::MULHSU: case RVInstr::MULHU:
    case RVInstr::DIV: case RVInstr::DIVU: case RVInstr::REM: case RVInstr::REMU:
    case RVInstr::ADDI: case RVInstr::SLTI: case RVInstr::SLTIU:
    case RVInstr::ANDI: case RVInstr::ORI: case RVInstr::XORI:
    case RVInstr::SLLI: case RVInstr::SRLI: case RVInstr::SRAI:
    case RVInstr::LUI: case RVInstr::AUIPC:
    case RVInstr::JAL: case RVInstr::JALR:
    case RVInstr::ADDW: case RVInstr::SUBW: case RVInstr::SLLW: case RVInstr::SRLW: case RVInstr::SRAW:
    case RVInstr::MULW: case RVInstr::DIVW: case RVInstr::DIVUW: case RVInstr::REMW: case RVInstr::REMUW:
    case RVInstr::ADDIW: case RVInstr::SLLIW: case RVInstr::SRLIW: case RVInstr::SRAIW:
    case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LD:
    case RVInstr::LBU: case RVInstr::LHU: case RVInstr::LWU:
      return true;
    case RVInstr::FCVT_W_S: case RVInstr::FCVT_WU_S:
    case RVInstr::FCVT_L_S: case RVInstr::FCVT_LU_S:
    case RVInstr::FMV_X_W:
    case RVInstr::FEQ_S: case RVInstr::FLT_S: case RVInstr::FLE_S:
      return true;
    default:
      return false;
    }
  }

  static bool doesWriteFloatReg(RVInstr opc) {
    switch(opc) {
    case RVInstr::FLW:
    case RVInstr::FADD_S: case RVInstr::FSUB_S: case RVInstr::FMUL_S:
    case RVInstr::FDIV_S: case RVInstr::FSQRT_S:
    case RVInstr::FMIN_S: case RVInstr::FMAX_S:
    case RVInstr::FSGNJ_S: case RVInstr::FSGNJN_S: case RVInstr::FSGNJX_S:
    case RVInstr::FCVT_S_W: case RVInstr::FCVT_S_WU:
    case RVInstr::FCVT_S_L: case RVInstr::FCVT_S_LU:
    case RVInstr::FMV_W_X:
    case RVInstr::FMADD_S: case RVInstr::FMSUB_S:
    case RVInstr::FNMSUB_S: case RVInstr::FNMADD_S:
      return true;
    default:
      return false;
    }
  }

  static bool doesWriteReg(RVInstr opc) {
    return doesWriteIntReg(opc) || doesWriteFloatReg(opc);
  }

  static ImmRegFileSrc do_reg1_do_read_src(RVInstr opc) {
    switch(getExtensionType(opc)) {
    case RVISA::Extension::Id::F:
      switch(opc) {
      case RVInstr::FCVT_S_W: case RVInstr::FCVT_S_WU:
      case RVInstr::FCVT_S_L: case RVInstr::FCVT_S_LU:
      case RVInstr::FMV_W_X:
        return ImmRegFileSrc::INTEGER;
      default:
        return ImmRegFileSrc::FLOAT;
      }
    default:
      return ImmRegFileSrc::INTEGER;
    }
  }

  static RegFileSrc do_reg2_do_read_src(RVInstr opc) {
    switch(getExtensionType(opc)) {
    case RVISA::Extension::Id::F:
      return RegFileSrc::FLOAT;
    default:
      return RegFileSrc::INTEGER;
    }
  }

  static RegFileSrc do_alu_fpu_src(RVInstr opc) {
    switch(getExtensionType(opc)) {
    case RVISA::Extension::Id::F:
      switch(opc) {
      case RVInstr::FLW:
      case RVInstr::FSW:
        return RegFileSrc::INTEGER;
      default:
        return RegFileSrc::FLOAT;
      }
    default:
      return RegFileSrc::INTEGER;
    }
  }

  static FPUOp do_fpu_ctrl(RVInstr opc) {
    switch(opc) {
    case RVInstr::FADD_S:    return FPUOp::FADD_S;
    case RVInstr::FSUB_S:    return FPUOp::FSUB_S;
    case RVInstr::FMUL_S:    return FPUOp::FMUL_S;
    case RVInstr::FDIV_S:    return FPUOp::FDIV_S;
    case RVInstr::FSQRT_S:   return FPUOp::FSQRT_S;
    case RVInstr::FMIN_S:    return FPUOp::FMIN_S;
    case RVInstr::FMAX_S:    return FPUOp::FMAX_S;
    case RVInstr::FSGNJ_S:   return FPUOp::FSGNJ_S;
    case RVInstr::FSGNJN_S:  return FPUOp::FSGNJN_S;
    case RVInstr::FSGNJX_S:  return FPUOp::FSGNJX_S;
    case RVInstr::FCVT_W_S:  return FPUOp::FCVT_W_S;
    case RVInstr::FCVT_WU_S: return FPUOp::FCVT_WU_S;
    case RVInstr::FCVT_S_W:  return FPUOp::FCVT_S_W;
    case RVInstr::FCVT_S_WU: return FPUOp::FCVT_S_WU;
    case RVInstr::FCVT_L_S:  return FPUOp::FCVT_L_S;
    case RVInstr::FCVT_LU_S: return FPUOp::FCVT_LU_S;
    case RVInstr::FCVT_S_L:  return FPUOp::FCVT_S_L;
    case RVInstr::FCVT_S_LU: return FPUOp::FCVT_S_LU;
    case RVInstr::FMV_X_W:   return FPUOp::FMV_X_W;
    case RVInstr::FMV_W_X:   return FPUOp::FMV_W_X;
    case RVInstr::FEQ_S:     return FPUOp::EQ;
    case RVInstr::FLT_S:     return FPUOp::LT;
    case RVInstr::FLE_S:     return FPUOp::LE;
    case RVInstr::FCLASS_S:  return FPUOp::FCLASS_S;
    case RVInstr::FMADD_S:   return FPUOp::FMADD_S;
    case RVInstr::FMSUB_S:   return FPUOp::FMSUB_S;
    case RVInstr::FNMSUB_S:  return FPUOp::FNMSUB_S;
    case RVInstr::FNMADD_S:  return FPUOp::FNMADD_S;
    default: return FPUOp::NOP;
    }
  }

  RVInstr getCurrentOpcode() const {
    return opcode.eValue<RVInstr>();
  }

  FSMState getCurrentState() const {
    return state_reg->out.eValue<FSMState>();
  }

  void setInitialState() const {
    state_reg->forceValue(0, magic_enum::enum_integer(FSMState::IF));
  }

  void addState(FSMState s, StateSignals o, TransitionFunc t) {
    auto idx = static_cast<int>(s);
    states.at(idx) = {o, t};
  }

  const StateInfo& getStateInfo(FSMState s) const {
    auto idx = static_cast<int>(s);
    return states[idx];
  }

  const StateInfo& getCurrentStateInfo() const {
    return getStateInfo(getCurrentState());
  }

  FSMState getNextState() const {
    return getCurrentStateInfo().transitions(getCurrentOpcode());
  }

public:
  RVMC1MControlF(const std::string &name, SimComponent *parent)
  : Component(name, parent) {
    assert (states.empty());
    states.resize(magic_enum::enum_count<FSMState>());

#define to(x) [](RVInstr i){ ((void)i); return FSMState::x; }

    addState(FSMState::IF, {
        .ir_write = true,
        .pc_write = true,
        .pc_src = PcSrc::PC4,
        .alu_op1_src = AluSrc1::PC,
        .alu_op2_src = AluSrc2::PC_INC,
        .alu_control = ALUControl::ADD,
        .mem_addr_src = MemAddrSrc::PC,
    }, to(ID));

    addState(FSMState::ID, {
        .alu_op1_src = AluSrc1::PCOLD,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
      }, [](RVInstr i){
        switch (i) {
        case RVInstr::JAL:
          return FSMState::WBJ;
        case RVInstr::JALR:
          return FSMState::EXJALR;
        case RVInstr::BEQ: case RVInstr::BNE: case RVInstr::BLT: case RVInstr::BGE: case RVInstr::BLTU: case RVInstr::BGEU:
          return FSMState::EXBRANCH;
        case RVInstr::ADD: case RVInstr::SUB: case RVInstr::SLT: case RVInstr::SLTU: case RVInstr::AND: case RVInstr::OR:
        case RVInstr::XOR: case RVInstr::SLL: case RVInstr::SRL: case RVInstr::SRA: case RVInstr::MUL: case RVInstr::MULH:
        case RVInstr::MULHSU: case RVInstr::MULHU: case RVInstr::DIV: case RVInstr::DIVU: case RVInstr::REM: case RVInstr::REMU:
        case RVInstr::MULW: case RVInstr::DIVW: case RVInstr::DIVUW: case RVInstr::REMW: case RVInstr::REMUW: case RVInstr::ADDW:
        case RVInstr::SUBW: case RVInstr::SLLW: case RVInstr::SRLW: case RVInstr::SRAW:
        case RVInstr::FADD_S: case RVInstr::FSUB_S: case RVInstr::FMUL_S: case RVInstr::FDIV_S:
        case RVInstr::FSQRT_S: case RVInstr::FMIN_S: case RVInstr::FMAX_S:
        case RVInstr::FSGNJ_S: case RVInstr::FSGNJN_S: case RVInstr::FSGNJX_S:
        case RVInstr::FCVT_W_S: case RVInstr::FCVT_WU_S: case RVInstr::FCVT_L_S: case RVInstr::FCVT_LU_S:
        case RVInstr::FCVT_S_W: case RVInstr::FCVT_S_WU: case RVInstr::FCVT_S_L: case RVInstr::FCVT_S_LU:
        case RVInstr::FMV_X_W: case RVInstr::FMV_W_X:
        case RVInstr::FEQ_S: case RVInstr::FLT_S: case RVInstr::FLE_S:
        case RVInstr::FCLASS_S:
        case RVInstr::FMADD_S: case RVInstr::FMSUB_S: case RVInstr::FNMSUB_S: case RVInstr::FNMADD_S:
          return FSMState::EXALUR;
        case RVInstr::ADDI: case RVInstr::SLTI: case RVInstr::SLTIU: case RVInstr::ANDI: case RVInstr::ORI: case RVInstr::XORI:
        case RVInstr::SLLI: case RVInstr::SRLI: case RVInstr::SRAI: case RVInstr::LUI: case RVInstr::SRLIW: case RVInstr::ADDIW:
        case RVInstr::SLLIW: case RVInstr::SRAIW:
          return FSMState::EXALUI;
        case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU: case RVInstr::LWU:
        case RVInstr::LD: case RVInstr::SB: case RVInstr::SH: case RVInstr::SW: case RVInstr::SD:
        case RVInstr::FLW: case RVInstr::FSW:
          return FSMState::EXMEMOP;
        case RVInstr::ECALL:
          return FSMState::EXECALL;
        case RVInstr::AUIPC:
          return FSMState::WBALU;
        default:
          return FSMState::IF;
      }});

    addState(FSMState::EXJALR, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
      }, to(WBJ));

    addState(FSMState::EXBRANCH, {
        .branch = true,
        .pc_src = PcSrc::ALU,
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::SUB,
      }, to(IF));

    addState(FSMState::EXALUR, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::REG2,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(WBALU));

    addState(FSMState::EXALUI, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::INSTRUCTION_DEPENDENT,
      }, to(WBALU));

    addState(FSMState::EXMEMOP, {
        .alu_op1_src = AluSrc1::REG1,
        .alu_op2_src = AluSrc2::IMM,
        .alu_control = ALUControl::ADD,
    }, [](RVInstr i){
      switch (i) {
        case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LD:
        case RVInstr::LBU: case RVInstr::LHU: case RVInstr::LWU:
        case RVInstr::FLW:
          return FSMState::MEMLOAD;
        case RVInstr::SB: case RVInstr::SH: case RVInstr::SW: case RVInstr::SD:
        case RVInstr::FSW:
          return FSMState::MEMSTORE;
        default:
          assert(false);
      }
    });

    addState(FSMState::EXECALL, {
      .ecall = true,
    }, to(IF));

    addState(FSMState::MEMLOAD, {
        .mem_addr_src = MemAddrSrc::ALUOUT,
      }, to(WBMEMLOAD));

    addState(FSMState::MEMSTORE, {
        .mem_write = true,
        .mem_addr_src = MemAddrSrc::ALUOUT,
      }, to(IF));

    addState(FSMState::WBALU, {}, to(IF));

    addState(FSMState::WBMEMLOAD, {}, to(IF));

    addState(FSMState::WBJ, {
        .pc_write = true,
        .pc_src = PcSrc::ALU,
      }, to(IF));

#undef to

    ir_write << [this] { return getCurrentStateInfo().outs.ir_write; };
    pc_write << [this] { return getCurrentStateInfo().outs.pc_write; };
    branch << [this] { return getCurrentStateInfo().outs.branch; };
    pc_src << [this] { return getCurrentStateInfo().outs.pc_src; };

    reg_write << [this] {
      auto s = getCurrentState();
      auto opc = getCurrentOpcode();
      if (s == FSMState::WBALU)
        return doesWriteIntReg(opc);
      if (s == FSMState::WBMEMLOAD)
        return doesWriteIntReg(opc);
      if (s == FSMState::WBJ)
        return doesWriteIntReg(opc);
      return false;
    };

    regF_write << [this] {
      auto s = getCurrentState();
      auto opc = getCurrentOpcode();
      if (s == FSMState::WBALU)
        return doesWriteFloatReg(opc);
      if (s == FSMState::WBMEMLOAD)
        return doesWriteFloatReg(opc);
      return false;
    };

    reg_src << [this] { return getCurrentStateInfo().outs.reg_src; };
    alu_op1_src << [this] { return getCurrentStateInfo().outs.alu_op1_src; };
    alu_op2_src << [this] { return getCurrentStateInfo().outs.alu_op2_src; };
    alu_ctrl << [this] {
      auto c = getCurrentStateInfo().outs.alu_control;
      return c == ALUControl::INSTRUCTION_DEPENDENT ? do_alu_ctrl(getCurrentOpcode())
      : c == ALUControl::ADD ? ALUOp::ADD
      : (assert(c == ALUControl::SUB), ALUOp::SUB);
    };
    mem_write << [this] { return getCurrentStateInfo().outs.mem_write; };
    mem_addr_src << [this] { return getCurrentStateInfo().outs.mem_addr_src; };

    mem_ctrl << [this] { return (mem_addr_src.eValue<MemAddrSrc>() == MemAddrSrc::PC) ? MemOp::LW : do_mem_ctrl(getCurrentOpcode());};
    comp_ctrl << [this] { return do_comp_ctrl(getCurrentOpcode()); };

    ecall << [this] { return !getCurrentStateInfo().outs.ecall; };

    reg1_do_read_src << [this] { return do_reg1_do_read_src(getCurrentOpcode()); };
    reg2_do_read_src << [this] { return do_reg2_do_read_src(getCurrentOpcode()); };
    alu_fpu_src << [this] { return do_alu_fpu_src(getCurrentOpcode()); };
    fpu_ctrl << [this] { return do_fpu_ctrl(getCurrentOpcode()); };

    next_state << [this] { return getNextState(); };
    current_state << [this] { return getCurrentState(); };

    next_state >> state_reg->in;

    setInitialState();
  }

  bool inFirstState() const {
    return getCurrentState() == FSMState::IF;
  }

  bool inLastState() const {
    return getNextState() == FSMState::IF;
  }

  SUBCOMPONENT(state_reg, Register<enumBitWidth<FSMState>()>);

  OUTPUTPORT_ENUM(next_state,FSMState);
  OUTPUTPORT_ENUM(current_state,FSMState);

  INPUTPORT_ENUM(opcode, RVInstr);
  OUTPUTPORT(ir_write, 1);
  OUTPUTPORT(pc_write, 1);
  OUTPUTPORT(branch, 1);
  OUTPUTPORT_ENUM(pc_src,PcSrc);
  OUTPUTPORT(reg_write, 1);
  OUTPUTPORT(regF_write, 1);
  OUTPUTPORT_ENUM(reg_src, RegWrSrc);
  OUTPUTPORT_ENUM(alu_op1_src, AluSrc1);
  OUTPUTPORT_ENUM(alu_op2_src, AluSrc2);
  OUTPUTPORT_ENUM(alu_ctrl, ALUOp);
  OUTPUTPORT(mem_write, 1);
  OUTPUTPORT_ENUM(mem_addr_src, MemAddrSrc);
  OUTPUTPORT_ENUM(mem_ctrl, MemOp);
  OUTPUTPORT_ENUM(comp_ctrl, CompOp);
  OUTPUTPORT(ecall,1);

  OUTPUTPORT_ENUM(reg1_do_read_src, ImmRegFileSrc);
  OUTPUTPORT_ENUM(reg2_do_read_src, RegFileSrc);
  OUTPUTPORT_ENUM(alu_fpu_src, RegFileSrc);
  OUTPUTPORT_ENUM(fpu_ctrl, FPUOp);
};
  
}
} // namespace core
} // namespace vsrtl
