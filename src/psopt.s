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
// Store x,y,z in fr1, fr2, fr3
	psq_l fr1, 0(r3), 0, 0
	psq_l fr2, 0(r4), 0, 0
	psq_l fr3, 0(r5), 0, 0
// Calculate xy (fr4), yz (fr5) and xz (fr6)
	ps_mul fr4, fr1, fr2     // xy  = x * y
	ps_mul fr5, fr2, fr3     // yz  = y * z
	ps_mul fr6, fr1, fr3     // xz  = x * z
// Calculate w (xy[1] * z[1] - xy[0] * z[0]) and store
	ps_mul fr7, fr4, fr3     // fr7 = xy * z
	ps_merge11 fr8, fr7, fr7 // fr8 = (fr7[1], fr7[1])
	ps_sub fr7, fr8, fr7     // fr7 = fr8 - fr7
	psq_st fr7, 12(r6), 1, 0 // out.w = fr7[0]
// Swap xy, yz, xz
	ps_merge10 fr4, fr4, fr4 // xy = (xy[1], xy[0])
	ps_merge10 fr5, fr5, fr5 // yz = (yz[1], yz[0])
	ps_merge10 fr6, fr6, fr6 // xz = (xz[1], xz[0])
// Calculate x (xy[0] * z[0] + xy[1] * z[1]) and store
	ps_mul fr7, fr4, fr3     // fr7 = xy * z
	ps_merge11 fr8, fr7, fr7 // fr8 = (fr7[1], fr7[1])
	ps_add fr7, fr7, fr8     // fr7 = fr7 + fr8
	psq_st fr7, 0(r6), 1, 0  // out.x = fr7[0]
// Calculate y (xz[0] * y[0] + xz[1] * y[1]) and store
	ps_mul fr7, fr6, fr2     // fr7 = xy * z
	ps_merge11 fr8, fr7, fr7 // fr8 = (fr7[1], fr7[1])
	ps_add fr7, fr7, fr8     // fr7 = fr7 + fr8
	psq_st fr7, 4(r6), 1, 0  // out.y = fr7[0]
// Calculate z (yz[0] * x[0] - yz[1] * x[1]) and store
	ps_mul fr7, fr5, fr1     // fr7 = yz * x
	ps_merge11 fr8, fr7, fr7 // fr8 = (fr7[1], fr7[1])
	ps_sub fr7, fr7, fr8     // fr7 = fr8 - fr7
	psq_st fr7, 8(r6), 1, 0  // out.z = fr7[0]
// We're done!
	blr