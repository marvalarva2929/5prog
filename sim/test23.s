.code
	addi r1, 15
	addi r2, 3
	and r3, r1, r2
	or r4, r3, r2
	mov r5, r0
	addi r5, 1
	priv r5, r4, r0, 4
	halt
