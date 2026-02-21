.code
	addi r1, 15
	addi r2, 5
	xor r3, r1, r2
	mov r1, r0
	addi r1, 1
	priv r1, r3, r0, 4
	halt
