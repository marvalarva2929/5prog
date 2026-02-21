.code
	addi r1, 0
	not r2, r1
	mov r1, r0
	addi r1, 1
	priv r1, r2, r0, 4
	halt
