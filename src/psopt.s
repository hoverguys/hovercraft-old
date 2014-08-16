#include "../tools/asm.h"

	.globl ps_float2Mul
	// r3 = optA, r4 = optB, r5 = result
ps_float2Mul:
	psq_l fr1, 0(r3), 0, 0
	psq_l fr2, 0(r4), 0, 0
	ps_mul fr1, fr1, fr2
	psq_st fr1, 0(r5), 0, 0
	blr

	.globl ps_eulerQuat
	// r3 = x, r4 = y, r5 = z, r6 = out
ps_eulerQuat:
// Load x(1) y(2) z(3)
	psq_l fr1, 0(r3), 0, 0
	psq_l fr2, 0(r4), 0, 0
	psq_l fr3, 0(r5), 0, 0
// xy(4) yz(5) xz(6)
	ps_mul fr4, fr1, fr2
	ps_mul fr5, fr2, fr3
	ps_mul fr6, fr1, fr3
// W (xy.1 * z.1 - xy.0 * z.0)
	ps_mul fr7, fr4, fr3
	ps_merge11 fr8, fr7, fr7
	ps_sub fr7, fr8, fr7
	psq_st fr7, 12(r6), 1, 0
// Swap vars
	ps_merge10 fr4, fr4, fr4
	ps_merge10 fr5, fr5, fr5
	ps_merge10 fr6, fr6, fr6
// X (xy.0 * z.0 + xy.1 * z.1)
	ps_mul fr7, fr4, fr3
	ps_sum0 fr7, fr7, fr7, fr7
	psq_st fr7, 0(r6), 1, 0
// Y (xz.0 * y.0 + xz.1 * y.1)
	ps_mul fr7, fr6, fr2
	ps_sum0 fr7, fr7, fr7, fr7
	psq_st fr7, 4(r6), 1, 0
// Z (yz.0 * x.0 - yz.1 * x.1)
	ps_mul fr7, fr5, fr1
	ps_merge11 fr8, fr7, fr7
	ps_sub fr7, fr7, fr8
	psq_st fr7, 8(r6), 1, 0
	blr

	.globl QUAT_dotProduct // FROM LIBOGC's gu_psasm.S
	//r3 = p, r4 = q, r5 = res
QUAT_dotProduct:
	psq_l		fr2,0(r3),0,0
	psq_l		fr4,0(r4),0,0
	ps_mul		fr1,fr2,fr4
	psq_l		fr3,8(r3),0,0
	psq_l		fr5,8(r4),0,0
	ps_madd		fr1,fr3,fr5,fr1
	ps_sum0		fr1,fr1,fr1,fr1
	psq_st      fr1, 0(r5), 1, 0
	blr

	.globl QUAT_scale // FROM LIBOGC's gu_psasm.S
	//r3 = q, r4 = r, r5 = scale
QUAT_scale:
	psq_l		fr4,0(r3),0,0
	psq_l		fr5,8(r3),0,0
	psq_l       fr1,0(r5),1,0
	ps_muls0	fr4,fr4,fr1
	psq_st		fr4,0(r4),0,0
	ps_muls0	fr5,fr5,fr1
	psq_st		fr5,8(r4),0,0
	blr