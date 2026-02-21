.code
	ld r1, 4095
	addi r2, 2
	mul r3, r1, r2
	mov r4, r0
	addi r4, 1
	priv r4, r3, r0, 4
	halt
