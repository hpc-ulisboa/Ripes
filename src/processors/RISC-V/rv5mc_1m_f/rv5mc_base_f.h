#pragma once

#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "processors/ripesvsrtlprocessor.h"

#include "processors/RISC-V/riscv.h"

#include "processors/RISC-V/rv5mc/rv5mc_alu.h"
#include "processors/RISC-V/rv5mc/rv5mc_branch.h"
#include "rv5mc_decode_and_umcompress_f.h"
#include "rv5mc_1m_control_f.h"

#include "processors/RISC-V/rv_alu.h"
#include "processors/RISC-V/rv_control.h"
#include "processors/RISC-V/rv_ecallchecker.h"
#include "processors/RISC-V/rv_immediate.h"
#include "processors/RISC-V/rv_registerfile.h"

namespace vsrtl {
namespace core {
using namespace Ripes;
using namespace Ripes::RVISA;

template <typename XLEN_T, typename ControlType, typename FSMStateEnum>
class RV5MCBaseF : public RipesVSRTLProcessor {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");

protected:
  static constexpr unsigned XLEN = sizeof(XLEN_T) * CHAR_BIT;

public:
  enum class Stage { IF = 0, ID, EX, MEM, WB, STAGECOUNT };
  static Stage StageFromIndex(int i) {
    assert(0 <= i && i <= static_cast<int>(Stage::STAGECOUNT));
    return static_cast<Stage>(i);
  }

  RV5MCBaseF(const ExtensionSetInfo &extensions, const std::string &procName)
      : RipesVSRTLProcessor(procName) {
    RV_ExtensionSet exts(extensions);
    exts << Extension::F;
    m_enabledISA = ISAInfoRegistry::getISA<XLenToRVISA<XLEN>()>(exts);
    decode->setISA(m_enabledISA);

    // Program counter
    pc_src->out >> pc_reg->in;
    0 >> pc_reg->clear;
    controlflow_or->out >> pc_reg->enable;

    pc_reg->out >> pc_old_reg->in;
    0 >> pc_old_reg->clear;
    control->ir_write >> pc_old_reg->enable;

    2 >> pc_inc->get(PcInc::INC2);
    4 >> pc_inc->get(PcInc::INC4);
    decode->Pc_Inc >> pc_inc->select;

    // -----------------------------------------------------------------------
    // Decode
    control->ir_write >> decode->enable;

    // -----------------------------------------------------------------------
    // Control signals
    decode->opcode >> control->opcode;

    // -----------------------------------------------------------------------
    // Immediate
    decode->opcode >> immediate->opcode;
    decode->exp_instr >> immediate->instr;

    // -----------------------------------------------------------------------
    // Integer registers
    decode->wr_reg_idx >> registerFile->wr_addr;
    decode->r1_reg_idx >> registerFile->r1_addr;
    decode->r2_reg_idx >> registerFile->r2_addr;
    control->reg_write >> registerFile->wr_en;
    reg_src->out >> registerFile->data_in;
    registerFile->setMemory(m_regMem);

    // -----------------------------------------------------------------------
    // Float registers
    decode->wr_reg_idx >> registerFileF->wr_addr;
    decode->r1_reg_idx >> registerFileF->r1_addr;
    decode->r2_reg_idx >> registerFileF->r2_addr;
    decode->r3_reg_idx >> registerFileF->r3_addr;
    control->regF_write >> registerFileF->wr_en;
    reg_src->out >> registerFileF->data_in;
    registerFileF->setMemory(m_FregMem);

    // -----------------------------------------------------------------------
    // Reg1 source mux (int / float / immediate)
    registerFile->r1_out >> reg1_src->get(ImmRegFileSrc::INTEGER);
    registerFileF->r1_out >> reg1_src->get(ImmRegFileSrc::FLOAT);
    immediate->imm >> reg1_src->get(ImmRegFileSrc::IMMEDIATE);
    control->reg1_do_read_src >> reg1_src->select;

    // -----------------------------------------------------------------------
    // Reg2 source mux (int / float)
    registerFile->r2_out >> reg2_src->get(RegFileSrc::INTEGER);
    registerFileF->r2_out >> reg2_src->get(RegFileSrc::FLOAT);
    control->reg2_do_read_src >> reg2_src->select;

    // -----------------------------------------------------------------------
    // Reg write source mux
    ALU_out->out >> reg_src->get(RegWrSrc::ALURES);
    pc_reg->out >> reg_src->get(RegWrSrc::PC4);
    control->reg_src >> reg_src->select;

    // -----------------------------------------------------------------------
    // Control flow
    control->branch >> *br_and->in[1];
    br_and->out >> *controlflow_or->in[0];
    control->pc_write >> *controlflow_or->in[1];

    // -----------------------------------------------------------------------
    // Branch unit
    alu->sign >> branch_unit->sign;
    alu->zero >> branch_unit->zero;
    alu->carry >> branch_unit->carry;
    control->comp_ctrl >> branch_unit->comp_op;
    branch_unit->res >> *br_and->in[0];

    // -----------------------------------------------------------------------
    // ALU - operand 1: reg1_src->out latched through 'a'
    reg1_src->out >> a->in;
    a->out >> alu_op1_src->get(rv5mc1mf::AluSrc1::REG1);
    pc_reg->out >> alu_op1_src->get(rv5mc1mf::AluSrc1::PC);
    pc_old_reg->out >> alu_op1_src->get(rv5mc1mf::AluSrc1::PCOLD);
    control->alu_op1_src >> alu_op1_src->select;

    // -----------------------------------------------------------------------
    // ALU - operand 2: reg2_src->out latched through 'b'
    reg2_src->out >> b->in;
    b->out >> alu_op2_src->get(rv5mc1mf::AluSrc2::REG2);
    immediate->imm >> alu_op2_src->get(rv5mc1mf::AluSrc2::IMM);
    pc_inc->out >> alu_op2_src->get(rv5mc1mf::AluSrc2::PC_INC);
    control->alu_op2_src >> alu_op2_src->select;

    alu_op1_src->out >> alu->op1;
    alu_op2_src->out >> alu->op2;
    control->alu_ctrl >> alu->ctrl;

    // -----------------------------------------------------------------------
    // ALU/FPU result mux
    alu->res >> alu_fpu_src->get(RegFileSrc::INTEGER);
    fpu->res >> alu_fpu_src->get(RegFileSrc::FLOAT);
    control->alu_fpu_src >> alu_fpu_src->select;
    alu_fpu_src->out >> ALU_out->in;

    ALU_out->out >> pc_src->get(PcSrc::ALU);
    alu->res >> pc_src->get(PcSrc::PC4);
    control->pc_src >> pc_src->select;

    // -----------------------------------------------------------------------
    // FPU
    a->out >> fpu->op1;
    b->out >> fpu->op2;
    registerFileF->r3_out >> fpu->op3;
    control->fpu_ctrl >> fpu->ctrl;
    decode->roundMode >> fpu->roundmode;

    // -----------------------------------------------------------------------
    // Ecall checker
    decode->opcode >> ecallChecker->opcode;
    ecallChecker->setSyscallCallback(&trapHandler);
    control->ecall >> ecallChecker->stallEcallHandling;
  }

  // Design subcomponents
  SUBCOMPONENT(registerFile, TYPE(RegisterFile<XLEN, XLEN, false>));
  SUBCOMPONENT(registerFileF, TYPE(RegisterFile3<XLEN, c_RVFBits, true, c_RVFRegs, false>));
  SUBCOMPONENT(alu, TYPE(RVMCALU<XLEN>));
  SUBCOMPONENT(fpu, TYPE(FPU_Fcsr<XLEN>));
  SUBCOMPONENT(control, ControlType);
  SUBCOMPONENT(immediate, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(decode, TYPE(DecodeUncompressF<XLEN>));
  SUBCOMPONENT(branch_unit, TYPE(BranchSimple<XLEN>));

  // Registers
  SUBCOMPONENT(pc_reg, RegisterClEn<XLEN>);
  SUBCOMPONENT(pc_old_reg, RegisterClEn<XLEN>);
  SUBCOMPONENT(a, Register<XLEN>);
  SUBCOMPONENT(b, Register<XLEN>);
  SUBCOMPONENT(ALU_out, Register<XLEN>);
  SUBCOMPONENT(mem_out, Register<XLEN>);

  // Multiplexers
  SUBCOMPONENT(reg_src, TYPE(EnumMultiplexer<RegWrSrc, XLEN>));
  SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, XLEN>));
  SUBCOMPONENT(alu_op1_src, TYPE(EnumMultiplexer<rv5mc1mf::AluSrc1, XLEN>));
  SUBCOMPONENT(alu_op2_src, TYPE(EnumMultiplexer<rv5mc1mf::AluSrc2, XLEN>));
  SUBCOMPONENT(alu_fpu_src, TYPE(EnumMultiplexer<RegFileSrc, XLEN>));
  SUBCOMPONENT(reg1_src, TYPE(EnumMultiplexer<ImmRegFileSrc, XLEN>));
  SUBCOMPONENT(reg2_src, TYPE(EnumMultiplexer<RegFileSrc, XLEN>));
  SUBCOMPONENT(pc_inc, TYPE(EnumMultiplexer<PcInc, XLEN>));

  // Gates
  SUBCOMPONENT(br_and, TYPE(And<1, 2>));
  SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));

  // Address spaces
  ADDRESSSPACEMM(m_memory);
  ADDRESSSPACE(m_regMem);
  ADDRESSSPACE(m_FregMem);

  SUBCOMPONENT(ecallChecker, EcallChecker);

  // Ripes interface compliance
  const ProcessorStructure &structure() const override { return m_structure; }
  unsigned int getPcForStage(StageIndex stage) const override {
    switch (StageFromIndex(stage.index())) {
    case Stage::IF:
      return pc_reg->out.uValue();
    default:
      return pc_old_reg->out.uValue();
    }
  }
  AInt nextFetchedAddress() const override { return pc_src->out.uValue(); }
  QString stageName(StageIndex stage) const override {
    switch (StageFromIndex(stage.index())) {
    case Stage::IF: return "IF";
    case Stage::ID: return "ID";
    case Stage::EX: return "EX";
    case Stage::MEM: return "MEM";
    case Stage::WB: return "WB";
    default: assert(false && "Processor does not contain stage");
    }
    Q_UNREACHABLE();
  }
  StageInfo stageInfo(StageIndex stage) const override {
    bool stageValid = true;
    auto fsmState = control->getCurrentState();
    switch (StageFromIndex(stage.index())) {
    case Stage::IF:
      stageValid &= (fsmState == FSMStateEnum::IF);
      break;
    case Stage::ID:
      stageValid &= (fsmState == FSMStateEnum::ID);
      break;
    case Stage::EX:
      stageValid &= (fsmState >= FSMStateEnum::EX && fsmState < FSMStateEnum::MEM);
      break;
    case Stage::MEM:
      stageValid &= (fsmState >= FSMStateEnum::MEM && fsmState < FSMStateEnum::WB);
      break;
    case Stage::WB:
      stageValid &= (fsmState >= FSMStateEnum::WB);
      break;
    default:
      break;
    }
    StageInfo::State state = StageInfo::State::None;
    return StageInfo{getPcForStage(stage), stageValid, state};
  }
  void setProgramCounter(AInt address) override {
    pc_reg->forceValue(0, address);
    control->setInitialState();
    propagateDesign();
  }
  void setPCInitialValue(AInt address) override {
    pc_reg->setInitValue(address);
  }
  AddressSpaceMM &getMemory() override { return *m_memory; }
  VInt getRegister(const std::string_view &, unsigned i) const override {
    return registerFile->getRegister(i);
  }
  void finalize(FinalizeReason fr) override {
    if (fr == FinalizeReason::exitSyscall) {
      m_finishInNextCycle = true;
    }
  }
  bool finished() const override {
    return m_finished || ((!isExecutableAddress(pc_reg->out.uValue()) &&
                           !isExecutableAddress(pc_reg->in.uValue())) &&
                          control->inLastState());
  }
  const std::vector<StageIndex> breakpointTriggeringStages() const override {
    return {{0, 0}};
  }

  void setRegister(const std::string_view &, unsigned i, VInt v) override {
    setSynchronousValue(registerFile->_wr_mem, i, v);
  }

  void clockProcessor() override {
    if (control->inLastState())
      m_instructionsRetired++;
    const bool finishInThisCycle = m_finishInNextCycle;
    Design::clock();
    if (finishInThisCycle) {
      m_finished = true;
    }
  }

  void reverse() override {
    if (control->inFirstState())
      m_instructionsRetired--;
    Design::reverse();
    m_finishInNextCycle = false;
    m_finished = false;
  }

  void reset() override {
    Design::reset();
    m_finishInNextCycle = false;
    m_finished = false;
  }

  static const ProcessorISAInfo &supportsISA() {
    static ProcessorISAInfo procInfo{
        std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(),
        std::make_shared<RV_ExtensionSet>(Extension::M, Extension::C, Extension::F),
        std::make_shared<RV_ExtensionSet>(Extension::M, Extension::F)};
    return procInfo;
  }
  std::shared_ptr<ISAInfoBase> implementsISA() const override {
    return m_enabledISA;
  }
  std::shared_ptr<const ISAInfoBase> fullISA() const override {
    return std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(
        *(supportsISA().supportedExtensions));
  }

  const std::set<std::string_view> registerFiles() const override {
    std::set<std::string_view> rfs;
    rfs.insert(RVISA::GPR);
    rfs.insert(RVISA::FPR);
    return rfs;
  }

private:
  bool m_finishInNextCycle = false;
  bool m_finished = false;
  std::shared_ptr<ISAInfoBase> m_enabledISA;
  ProcessorStructure m_structure = {{0, 5}};
};

} // namespace core
} // namespace vsrtl
