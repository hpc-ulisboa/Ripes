
Full instruction reference for the single-precision floating-point (F-extension) support added in this fork. For implementation status, see the table in the main README.

## Memory

|Instruction|Parameters|Description|
|---|---|---|
|`flw`|`fd, imm(rs1)`|`fd = M[rs1 + imm]`|
|`fsw`|`fs2, imm(rs1)`|`M[rs1 + imm] = fs2`|

## Fused Multiply-Add

|Instruction|Parameters|Description|
|---|---|---|
|`fmadd.s`|`fd, fs1, fs2, fs3 [, rm]`|`fd = fs1 * fs2 + fs3`|
|`fmsub.s`|`fd, fs1, fs2, fs3 [, rm]`|`fd = fs1 * fs2 - fs3`|
|`fnmadd.s`|`fd, fs1, fs2, fs3 [, rm]`|`fd = -(fs1 * fs2) - fs3`|
|`fnmsub.s`|`fd, fs1, fs2, fs3 [, rm]`|`fd = -(fs1 * fs2) + fs3`|

## Arithmetic

|Instruction|Parameters|Description|
|---|---|---|
|`fadd.s`|`fd, fs1, fs2 [, rm]`|`fd = fs1 + fs2`|
|`fsub.s`|`fd, fs1, fs2 [, rm]`|`fd = fs1 - fs2`|
|`fmul.s`|`fd, fs1, fs2 [, rm]`|`fd = fs1 * fs2`|
|`fdiv.s`|`fd, fs1, fs2 [, rm]`|`fd = fs1 / fs2`|
|`fsqrt.s`|`fd, fs1 [, rm]`|`fd = sqrt(fs1)`|
|`fmin.s`|`fd, fs1, fs2`|`fd = min(fs1, fs2)`|
|`fmax.s`|`fd, fs1, fs2`|`fd = max(fs1, fs2)`|

## Sign Injection

|Instruction|Parameters|Description|
|---|---|---|
|`fsgnj.s`|`fd, fs1, fs2`|`fd = abs(fs1) * sgn(fs2)`|
|`fsgnjn.s`|`fd, fs1, fs2`|`fd = abs(fs1) * -sgn(fs2)`|
|`fsgnjx.s`|`fd, fs1, fs2`|`fd = fs1 * sgn(fs2)`|

## Conversion

|Instruction|Parameters|Description|
|---|---|---|
|`fcvt.s.w`|`fd, rs1 [, rm]`|`fd = (float) rs1`|
|`fcvt.s.wu`|`fd, rs1 [, rm]`|`fd = (float) rs1`|
|`fcvt.w.s`|`rd, fs1 [, rm]`|`rd = (int32_t) fs1`|
|`fcvt.wu.s`|`rd, fs1 [, rm]`|`rd = (uint32_t) fs1`|
|`fcvt.s.l`|`fd, rs1 [, rm]`|`fd = (float) rs1`|
|`fcvt.s.lu`|`fd, rs1 [, rm]`|`fd = (float) rs1`|
|`fcvt.l.s`|`rd, fs1 [, rm]`|`rd = (int64_t) fs1`|
|`fcvt.lu.s`|`rd, fs1 [, rm]`|`rd = (uint64_t) fs1`|

## Move

|Instruction|Parameters|Description|
|---|---|---|
|`fmv.x.w`|`rd, fs1`|`rd = *((int*) &fs1)`|
|`fmv.w.x`|`fd, rs1`|`fd = *((float*) &rs1)`|

## Compare

|Instruction|Parameters|Description|
|---|---|---|
|`feq.s`|`rd, fs1, fs2`|`rd = (fs1 == fs2) ? 1 : 0`|
|`flt.s`|`rd, fs1, fs2`|`rd = (fs1 < fs2) ? 1 : 0`|
|`fle.s`|`rd, fs1, fs2`|`rd = (fs1 <= fs2) ? 1 : 0`|

## Classify

|Instruction|Parameters|Description|
|---|---|---|
|`fclass.s`|`rd, fs1`|`rd = fclass(fs1)`|

## Pseudo Instructions

|Instruction|Parameters|Description|
|---|---|---|
|`fmv.s`|`fd, fs1`|`fd = fs1`|
|`fabs.s`|`fd, fs1`|`fd = \|fs1\|`|
|`fneg.s`|`fd, fs1`|`fd = -fs1`|

## Pseudo CSR Instructions (floating-point control/status)

|Instruction|Parameters|Description|Expands to|
|---|---|---|---|
|`frcsr`|`rd`|`rd = fcsr`|`CSRRS rd, fcsr, x0`|
|`fscsr`|`rd, rs1`|`rd = fcsr; fcsr = rs1`|`CSRRW rd, fcsr, rs1`|
|`frrm`|`rd`|`rd = frm`|`CSRRS rd, frm, x0`|
|`fsrm`|`rd, rs1`|`rd = frm; frm = rs1`|`CSRRW rd, frm, rs1`|
|`frflags`|`rd`|`rd = fflags`|`CSRRS rd, fflags, x0`|
|`fsflags`|`rd, rs1`|`rd = fflags; fflags = rs1`|`CSRRW rd, fflags, rs1`|