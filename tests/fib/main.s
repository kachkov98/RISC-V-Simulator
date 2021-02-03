	.file	"main.c"
	.option nopic
	.text
	.section	.rodata
	.align	2
.LC0:
	.string	"%llu\n"
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-48
	sw	ra,44(sp)
	sw	s0,40(sp)
	addi	s0,sp,48
	li	a5,1
	li	a6,0
	sw	a5,-24(s0)
	sw	a6,-20(s0)
	li	a5,1
	li	a6,0
	sw	a5,-32(s0)
	sw	a6,-28(s0)
	sw	zero,-36(s0)
	j	.L2
.L3:
	lw	a5,-24(s0)
	sw	a5,-40(s0)
	lw	a5,-32(s0)
	lw	a6,-28(s0)
	sw	a5,-24(s0)
	sw	a6,-20(s0)
	lw	a5,-40(s0)
	mv	a1,a5
	li	a2,0
	lw	a3,-32(s0)
	lw	a4,-28(s0)
	add	a5,a3,a1
	mv	a0,a5
	sltu	a0,a0,a3
	add	a6,a4,a2
	add	a4,a0,a6
	mv	a6,a4
	sw	a5,-32(s0)
	sw	a6,-28(s0)
	lw	a5,-36(s0)
	addi	a5,a5,1
	sw	a5,-36(s0)
.L2:
	lw	a4,-36(s0)
	li	a5,9998336
	addi	a5,a5,1663
	bleu	a4,a5,.L3
	lw	a2,-32(s0)
	lw	a3,-28(s0)
	lui	a5,%hi(.LC0)
	addi	a0,a5,%lo(.LC0)
	call	printf
	li	a5,0
	mv	a0,a5
	lw	ra,44(sp)
	lw	s0,40(sp)
	addi	sp,sp,48
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 8.2.0"
