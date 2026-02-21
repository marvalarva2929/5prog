.code
	addi r1, 15
	addi r2, 7
	and r3, r1, r2
	ld r1, 1
	priv r1, r3, r0, 4
	halt
