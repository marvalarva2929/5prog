.code
	addi r1, 10
	subi r1, 3
	mov r2, r0
	addi r2, 1
	priv r2, r1, r0, 4
	halt
