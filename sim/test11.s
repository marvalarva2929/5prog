.code
	addi r1, 3
	addi r2, 2
	shftl r3, r1, r2
	mov r1, r0
	addi r1, 1
	priv r1, r3, r0, 4
	halt
