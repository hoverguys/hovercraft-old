#include "../tools/asm.h"

	.globl ps_float2Mul
	// r3 = optA, r4 = optB, r5 = result
ps_float2Mul:
	psq_l fr1, 0(r3), 0, 0
	psq_l fr2, 0(r4), 0, 0
	ps_mul fr1, fr1, fr2
	psq_st fr1, 0(r5), 0, 0
	blr