	.file	"8-queens.c"
	.option nopic
	.text
	.comm	solution_count,4,4
	.globl	finished
	.section	.sbss,"aw",@nobits
	.align	2
	.type	finished, @object
	.size	finished, 4
finished:
	.zero	4
	.text
	.align	2
	.globl	backtrack
	.type	backtrack, @function
backtrack:
	addi	sp,sp,-448
	sw	ra,444(sp)
	sw	s0,440(sp)
	addi	s0,sp,448
	sw	a0,-436(s0)
	sw	a1,-440(s0)
	sw	a2,-444(s0)
	lw	a2,-444(s0)
	lw	a1,-440(s0)
	lw	a0,-436(s0)
	call	is_a_solution
	mv	a5,a0
	beqz	a5,.L2
	lw	a2,-444(s0)
	lw	a1,-440(s0)
	lw	a0,-436(s0)
	call	process_solution
	j	.L1
.L2:
	lw	a5,-440(s0)
	addi	a5,a5,1
	sw	a5,-440(s0)
	addi	a4,s0,-424
	addi	a5,s0,-420
	mv	a3,a5
	lw	a2,-444(s0)
	lw	a1,-440(s0)
	lw	a0,-436(s0)
	call	construct_candidates
	sw	zero,-20(s0)
	j	.L4
.L7:
	lw	a5,-440(s0)
	slli	a5,a5,2
	lw	a4,-436(s0)
	add	a4,a4,a5
	lw	a5,-20(s0)
	slli	a5,a5,2
	addi	a3,s0,-16
	add	a5,a3,a5
	lw	a5,-404(a5)
	sw	a5,0(a4)
	lw	a2,-444(s0)
	lw	a1,-440(s0)
	lw	a0,-436(s0)
	call	backtrack
	lui	a5,%hi(finished)
	lw	a5,%lo(finished)(a5)
	bnez	a5,.L8
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L4:
	lw	a5,-424(s0)
	lw	a4,-20(s0)
	blt	a4,a5,.L7
	j	.L1
.L8:
	nop
.L1:
	lw	ra,444(sp)
	lw	s0,440(sp)
	addi	sp,sp,448
	jr	ra
	.size	backtrack, .-backtrack
	.align	2
	.globl	process_solution
	.type	process_solution, @function
process_solution:
	addi	sp,sp,-32
	sw	s0,28(sp)
	addi	s0,sp,32
	sw	a0,-20(s0)
	sw	a1,-24(s0)
	lui	a5,%hi(solution_count)
	lw	a5,%lo(solution_count)(a5)
	addi	a4,a5,1
	lui	a5,%hi(solution_count)
	sw	a4,%lo(solution_count)(a5)
	nop
	mv	a0,a5
	lw	s0,28(sp)
	addi	sp,sp,32
	jr	ra
	.size	process_solution, .-process_solution
	.align	2
	.globl	is_a_solution
	.type	is_a_solution, @function
is_a_solution:
	addi	sp,sp,-32
	sw	s0,28(sp)
	addi	s0,sp,32
	sw	a0,-20(s0)
	sw	a1,-24(s0)
	sw	a2,-28(s0)
	lw	a4,-24(s0)
	lw	a5,-28(s0)
	sub	a5,a4,a5
	seqz	a5,a5
	andi	a5,a5,0xff
	mv	a0,a5
	lw	s0,28(sp)
	addi	sp,sp,32
	jr	ra
	.size	is_a_solution, .-is_a_solution
	.align	2
	.globl	construct_candidates
	.type	construct_candidates, @function
construct_candidates:
	addi	sp,sp,-64
	sw	s0,60(sp)
	addi	s0,sp,64
	sw	a0,-36(s0)
	sw	a1,-40(s0)
	sw	a2,-44(s0)
	sw	a3,-48(s0)
	sw	a4,-52(s0)
	lw	a5,-52(s0)
	sw	zero,0(a5)
	li	a5,1
	sw	a5,-20(s0)
	j	.L13
.L19:
	li	a5,1
	sw	a5,-28(s0)
	li	a5,1
	sw	a5,-24(s0)
	j	.L14
.L17:
	lw	a4,-40(s0)
	lw	a5,-24(s0)
	sub	a4,a4,a5
	srai	a5,a4,31
	xor	a4,a5,a4
	sub	a4,a4,a5
	lw	a5,-24(s0)
	slli	a5,a5,2
	lw	a3,-36(s0)
	add	a5,a3,a5
	lw	a5,0(a5)
	lw	a3,-20(s0)
	sub	a5,a3,a5
	srai	a3,a5,31
	xor	a5,a3,a5
	sub	a5,a5,a3
	bne	a4,a5,.L15
	sw	zero,-28(s0)
.L15:
	lw	a5,-24(s0)
	slli	a5,a5,2
	lw	a4,-36(s0)
	add	a5,a4,a5
	lw	a5,0(a5)
	lw	a4,-20(s0)
	bne	a4,a5,.L16
	sw	zero,-28(s0)
.L16:
	lw	a5,-24(s0)
	addi	a5,a5,1
	sw	a5,-24(s0)
.L14:
	lw	a4,-24(s0)
	lw	a5,-40(s0)
	blt	a4,a5,.L17
	lw	a4,-28(s0)
	li	a5,1
	bne	a4,a5,.L18
	lw	a5,-52(s0)
	lw	a5,0(a5)
	slli	a5,a5,2
	lw	a4,-48(s0)
	add	a5,a4,a5
	lw	a4,-20(s0)
	sw	a4,0(a5)
	lw	a5,-52(s0)
	lw	a5,0(a5)
	addi	a4,a5,1
	lw	a5,-52(s0)
	sw	a4,0(a5)
.L18:
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L13:
	lw	a4,-20(s0)
	lw	a5,-44(s0)
	ble	a4,a5,.L19
	nop
	mv	a0,a5
	lw	s0,60(sp)
	addi	sp,sp,64
	jr	ra
	.size	construct_candidates, .-construct_candidates
	.section	.rodata
	.align	2
.LC1:
	.string	"n=%d  solution_count=%d\n"
	.align	2
.LC0:
	.word	1
	.word	0
	.word	0
	.word	2
	.word	10
	.word	4
	.word	40
	.word	92
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-464
	sw	ra,460(sp)
	sw	s0,456(sp)
	addi	s0,sp,464
	lui	a5,%hi(.LC0)
	lw	a7,%lo(.LC0)(a5)
	addi	a4,a5,%lo(.LC0)
	lw	a6,4(a4)
	addi	a4,a5,%lo(.LC0)
	lw	a0,8(a4)
	addi	a4,a5,%lo(.LC0)
	lw	a1,12(a4)
	addi	a4,a5,%lo(.LC0)
	lw	a2,16(a4)
	addi	a4,a5,%lo(.LC0)
	lw	a3,20(a4)
	addi	a4,a5,%lo(.LC0)
	lw	a4,24(a4)
	addi	a5,a5,%lo(.LC0)
	lw	a5,28(a5)
	sw	a7,-452(s0)
	sw	a6,-448(s0)
	sw	a0,-444(s0)
	sw	a1,-440(s0)
	sw	a2,-436(s0)
	sw	a3,-432(s0)
	sw	a4,-428(s0)
	sw	a5,-424(s0)
	li	a5,1
	sw	a5,-20(s0)
	j	.L21
.L23:
	lui	a5,%hi(solution_count)
	sw	zero,%lo(solution_count)(a5)
	lw	a4,-20(s0)
	addi	a5,s0,-420
	mv	a2,a4
	li	a1,0
	mv	a0,a5
	call	backtrack
	lui	a5,%hi(solution_count)
	lw	a5,%lo(solution_count)(a5)
	mv	a2,a5
	lw	a1,-20(s0)
	lui	a5,%hi(.LC1)
	addi	a0,a5,%lo(.LC1)
	call	printf
	lw	a5,-20(s0)
	addi	a5,a5,-1
	slli	a5,a5,2
	addi	a4,s0,-16
	add	a5,a4,a5
	lw	a4,-436(a5)
	lui	a5,%hi(solution_count)
	lw	a5,%lo(solution_count)(a5)
	beq	a4,a5,.L22
	call	abort
.L22:
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L21:
	lw	a4,-20(s0)
	li	a5,8
	ble	a4,a5,.L23
	li	a5,0
	mv	a0,a5
	lw	ra,460(sp)
	lw	s0,456(sp)
	addi	sp,sp,464
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 8.2.0"
