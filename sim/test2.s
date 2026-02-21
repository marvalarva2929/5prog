.code
	addi r1, 8
	addi r2, 4
	or r3, r1, r2
	mov r1, r0
	addi r1, 1
	priv r1, r3, r0, 4
	halt
