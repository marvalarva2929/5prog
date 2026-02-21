.code
	addi r1, 2
	addi r2, 3
	add r3, r1, r2
	mul r4, r3, r2
	mov r5, r0
	addi r5, 1
	priv r5, r4, r0, 4
	halt
