# RISC-V Base Integer ISA (RV32I) — Instruction Reference

Reference for the RISC-V base integer instruction set (RV32I), the foundation all extensions build on top of.

## Register-Register (R-type)

| Instruction | Parameters | Description |
|---|---|---|
| `add` | `rd, rs1, rs2` | `rd = rs1 + rs2` |
| `sub` | `rd, rs1, rs2` | `rd = rs1 - rs2` |
| `sll` | `rd, rs1, rs2` | `rd = rs1 << rs2` |
| `slt` | `rd, rs1, rs2` | `rd = (rs1 < rs2) ? 1 : 0` (signed) |
| `sltu` | `rd, rs1, rs2` | `rd = (rs1 < rs2) ? 1 : 0` (unsigned) |
| `xor` | `rd, rs1, rs2` | `rd = rs1 ^ rs2` |
| `srl` | `rd, rs1, rs2` | `rd = rs1 >> rs2` (logical) |
| `sra` | `rd, rs1, rs2` | `rd = rs1 >> rs2` (arithmetic) |
| `or` | `rd, rs1, rs2` | `rd = rs1 \| rs2` |
| `and` | `rd, rs1, rs2` | `rd = rs1 & rs2` |

## Register-Immediate (I-type, arithmetic/logical)

| Instruction | Parameters | Description |
|---|---|---|
| `addi` | `rd, rs1, imm` | `rd = rs1 + imm` |
| `slti` | `rd, rs1, imm` | `rd = (rs1 < imm) ? 1 : 0` (signed) |
| `sltiu` | `rd, rs1, imm` | `rd = (rs1 < imm) ? 1 : 0` (unsigned) |
| `xori` | `rd, rs1, imm` | `rd = rs1 ^ imm` |
| `ori` | `rd, rs1, imm` | `rd = rs1 \| imm` |
| `andi` | `rd, rs1, imm` | `rd = rs1 & imm` |
| `slli` | `rd, rs1, shamt` | `rd = rs1 << shamt` |
| `srli` | `rd, rs1, shamt` | `rd = rs1 >> shamt` (logical) |
| `srai` | `rd, rs1, shamt` | `rd = rs1 >> shamt` (arithmetic) |

## Loads (I-type)

| Instruction | Parameters | Description |
|---|---|---|
| `lb` | `rd, imm(rs1)` | `rd = sign_extend(M[rs1 + imm][7:0])` |
| `lh` | `rd, imm(rs1)` | `rd = sign_extend(M[rs1 + imm][15:0])` |
| `lw` | `rd, imm(rs1)` | `rd = M[rs1 + imm][31:0]` |
| `lbu` | `rd, imm(rs1)` | `rd = zero_extend(M[rs1 + imm][7:0])` |
| `lhu` | `rd, imm(rs1)` | `rd = zero_extend(M[rs1 + imm][15:0])` |

## Stores (S-type)

| Instruction | Parameters | Description |
|---|---|---|
| `sb` | `rs2, imm(rs1)` | `M[rs1 + imm][7:0] = rs2[7:0]` |
| `sh` | `rs2, imm(rs1)` | `M[rs1 + imm][15:0] = rs2[15:0]` |
| `sw` | `rs2, imm(rs1)` | `M[rs1 + imm][31:0] = rs2[31:0]` |

## Branches (B-type)

| Instruction | Parameters | Description |
|---|---|---|
| `beq` | `rs1, rs2, imm` | `if (rs1 == rs2) pc += imm` |
| `bne` | `rs1, rs2, imm` | `if (rs1 != rs2) pc += imm` |
| `blt` | `rs1, rs2, imm` | `if (rs1 < rs2) pc += imm` (signed) |
| `bge` | `rs1, rs2, imm` | `if (rs1 >= rs2) pc += imm` (signed) |
| `bltu` | `rs1, rs2, imm` | `if (rs1 < rs2) pc += imm` (unsigned) |
| `bgeu` | `rs1, rs2, imm` | `if (rs1 >= rs2) pc += imm` (unsigned) |

## Jumps

| Instruction | Parameters | Description |
|---|---|---|
| `jal` | `rd, imm` | `rd = pc + 4; pc += imm` |
| `jalr` | `rd, rs1, imm` | `rd = pc + 4; pc = (rs1 + imm) & ~1` |

## Upper Immediate (U-type)

| Instruction | Parameters | Description |
|---|---|---|
| `lui` | `rd, imm` | `rd = imm << 12` |
| `auipc` | `rd, imm` | `rd = pc + (imm << 12)` |

## System / Environment

| Instruction | Parameters | Description |
|---|---|---|
| `ecall` | — | Makes a request to the execution environment (e.g. syscall) |
| `ebreak` | — | Transfers control to a debugger |
| `fence` | `pred, succ` | Orders device I/O and memory accesses |

## Common Pseudo-Instructions

| Pseudo-instruction | Parameters | Description | Expands to |
|---|---|---|---|
| `nop` | — | No operation | `addi x0, x0, 0` |
| `mv` | `rd, rs1` | `rd = rs1` | `addi rd, rs1, 0` |
| `not` | `rd, rs1` | `rd = ~rs1` | `xori rd, rs1, -1` |
| `neg` | `rd, rs1` | `rd = -rs1` | `sub rd, x0, rs1` |
| `li` | `rd, imm` | `rd = imm` | `lui` + `addi` (or single `addi`, depending on size) |
| `j` | `imm` | `pc += imm` | `jal x0, imm` |
| `jr` | `rs1` | `pc = rs1` | `jalr x0, rs1, 0` |
| `ret` | — | Return from subroutine | `jalr x0, ra, 0` |
| `beqz` | `rs1, imm` | `if (rs1 == 0) pc += imm` | `beq rs1, x0, imm` |
| `bnez` | `rs1, imm` | `if (rs1 != 0) pc += imm` | `bne rs1, x0, imm` |
