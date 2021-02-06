	.file	"8-queens.c"
	.option nopic
	.text
	.align	2
	.globl	process_solution
	.type	process_solution, @function
process_solution:
	lui	a4,%hi(solution_count)
	lw	a5,%lo(solution_count)(a4)
	addi	a5,a5,1
	sw	a5,%lo(solution_count)(a4)
	ret
	.size	process_solution, .-process_solution
	.align	2
	.globl	is_a_solution
	.type	is_a_solution, @function
is_a_solution:
	sub	a2,a1,a2
	seqz	a0,a2
	ret
	.size	is_a_solution, .-is_a_solution
	.align	2
	.globl	construct_candidates
	.type	construct_candidates, @function
construct_candidates:
	sw	zero,0(a4)
	blez	a2,.L5
	addi	t6,a2,1
	li	t4,1
	li	t5,1
.L6:
	addi	t1,a0,4
	addi	a7,a1,-1
	li	a6,1
	ble	a1,t5,.L12
.L9:
	lw	a2,0(t1)
	addi	t1,t1,4
	sub	a5,t4,a2
	srai	t3,a5,31
	xor	a5,t3,a5
	sub	a5,a5,t3
	sub	a5,a5,a7
	snez	a5,a5
	sub	a2,a2,t4
	sub	a5,zero,a5
	snez	a2,a2
	and	a6,a6,a5
	sub	a2,zero,a2
	addi	a7,a7,-1
	and	a6,a6,a2
	bnez	a7,.L9
	beq	a6,t5,.L12
.L10:
	addi	t4,t4,1
	bne	t4,t6,.L6
.L5:
	ret
.L12:
	lw	a5,0(a4)
	slli	a5,a5,2
	add	a5,a3,a5
	sw	t4,0(a5)
	lw	a5,0(a4)
	addi	a5,a5,1
	sw	a5,0(a4)
	j	.L10
	.size	construct_candidates, .-construct_candidates
	.align	2
	.globl	backtrack
	.type	backtrack, @function
backtrack:
	bne	a2,a1,.L17
	lui	a4,%hi(solution_count)
	lw	a5,%lo(solution_count)(a4)
	addi	a5,a5,1
	sw	a5,%lo(solution_count)(a4)
	ret
.L17:
	addi	sp,sp,-448
	sw	s1,436(sp)
	addi	s1,a1,1
	addi	a4,sp,12
	addi	a3,sp,16
	mv	a1,s1
	sw	s3,428(sp)
	sw	s4,424(sp)
	sw	ra,444(sp)
	sw	s0,440(sp)
	sw	s2,432(sp)
	sw	s5,420(sp)
	sw	s6,416(sp)
	mv	s3,a2
	mv	s4,a0
	call	construct_candidates
	lw	a5,12(sp)
	blez	a5,.L16
	slli	s5,s1,2
	add	s5,s4,s5
	addi	s0,sp,16
	li	s2,0
	lui	s6,%hi(finished)
.L21:
	lw	a5,0(s0)
	mv	a2,s3
	mv	a1,s1
	sw	a5,0(s5)
	mv	a0,s4
	call	backtrack
	lw	a5,%lo(finished)(s6)
	addi	s2,s2,1
	addi	s0,s0,4
	bnez	a5,.L16
	lw	a5,12(sp)
	bgt	a5,s2,.L21
.L16:
	lw	ra,444(sp)
	lw	s0,440(sp)
	lw	s1,436(sp)
	lw	s2,432(sp)
	lw	s3,428(sp)
	lw	s4,424(sp)
	lw	s5,420(sp)
	lw	s6,416(sp)
	addi	sp,sp,448
	jr	ra
	.size	backtrack, .-backtrack
	.section	.text.startup,"ax",@progbits
	.align	2
	.globl	main
	.type	main, @function
main:
	lui	a5,%hi(.LANCHOR0)
	addi	a5,a5,%lo(.LANCHOR0)
	lw	a7,0(a5)
	lw	a6,4(a5)
	lw	a0,8(a5)
	lw	a1,12(a5)
	lw	a2,16(a5)
	lw	a3,20(a5)
	lw	a4,24(a5)
	lw	a5,28(a5)
	addi	sp,sp,-464
	sw	s0,456(sp)
	sw	s1,452(sp)
	sw	s2,448(sp)
	sw	s3,444(sp)
	sw	s4,440(sp)
	sw	ra,460(sp)
	sw	a7,0(sp)
	sw	a6,4(sp)
	sw	a0,8(sp)
	sw	a1,12(sp)
	sw	a2,16(sp)
	sw	a3,20(sp)
	sw	a4,24(sp)
	sw	a5,28(sp)
	mv	s2,sp
	li	s0,1
	lui	s1,%hi(solution_count)
	lui	s4,%hi(.LC1)
	li	s3,9
.L28:
	mv	a2,s0
	li	a1,0
	addi	a0,sp,32
	sw	zero,%lo(solution_count)(s1)
	call	backtrack
	lw	a2,%lo(solution_count)(s1)
	mv	a1,s0
	addi	a0,s4,%lo(.LC1)
	call	printf
	lw	a4,0(s2)
	lw	a5,%lo(solution_count)(s1)
	bne	a4,a5,.L31
	addi	s0,s0,1
	addi	s2,s2,4
	bne	s0,s3,.L28
	lw	ra,460(sp)
	lw	s0,456(sp)
	lw	s1,452(sp)
	lw	s2,448(sp)
	lw	s3,444(sp)
	lw	s4,440(sp)
	li	a0,0
	addi	sp,sp,464
	jr	ra
.L31:
	call	abort
	.size	main, .-main
	.globl	finished
	.comm	solution_count,4,4
	.section	.rodata
	.align	2
	.set	.LANCHOR0,. + 0
.LC0:
	.word	1
	.word	0
	.word	0
	.word	2
	.word	10
	.word	4
	.word	40
	.word	92
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
.LC1:
	.string	"n=%d  solution_count=%d\n"
	.section	.sbss,"aw",@nobits
	.align	2
	.type	finished, @object
	.size	finished, 4
finished:
	.zero	4
	.ident	"GCC: (GNU) 8.2.0"
