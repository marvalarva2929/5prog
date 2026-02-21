.code
	addi r1, 1
	mov r2, r0
	addi r2, 1
	priv r2, r1, r0, 4
	clr r1
	addi r1, 2
	priv r2, r1, r0, 4
	clr r1
	addi r1, 3
	priv r2, r1, r0, 4
	halt
