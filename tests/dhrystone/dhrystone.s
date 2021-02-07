	.file	"dhrystone.c"
	.option nopic
	.text
	.align	2
	.globl	Proc2
	.type	Proc2, @function
Proc2:
	lui	a5,%hi(Char1Glob)
	lbu	a4,%lo(Char1Glob)(a5)
	li	a5,65
	beq	a4,a5,.L4
	ret
.L4:
	lw	a5,0(a0)
	lui	a4,%hi(IntGlob)
	lw	a4,%lo(IntGlob)(a4)
	addi	a5,a5,9
	sub	a5,a5,a4
	sw	a5,0(a0)
	ret
	.size	Proc2, .-Proc2
	.align	2
	.globl	Proc3
	.type	Proc3, @function
Proc3:
	lui	a3,%hi(PtrGlb)
	lw	a5,%lo(PtrGlb)(a3)
	beqz	a5,.L6
	lw	a5,0(a5)
	lui	a4,%hi(IntGlob)
	lw	a4,%lo(IntGlob)(a4)
	sw	a5,0(a0)
	lw	a5,%lo(PtrGlb)(a3)
	addi	a4,a4,12
	sw	a4,12(a5)
	ret
.L6:
	lui	a4,%hi(IntGlob)
	li	a3,100
	sw	a3,%lo(IntGlob)(a4)
	li	a4,112
	sw	a4,12(a5)
	ret
	.size	Proc3, .-Proc3
	.align	2
	.globl	Proc4
	.type	Proc4, @function
Proc4:
	lui	a5,%hi(Char2Glob)
	li	a4,66
	sb	a4,%lo(Char2Glob)(a5)
	ret
	.size	Proc4, .-Proc4
	.align	2
	.globl	Proc5
	.type	Proc5, @function
Proc5:
	lui	a5,%hi(Char1Glob)
	li	a4,65
	sb	a4,%lo(Char1Glob)(a5)
	lui	a5,%hi(BoolGlob)
	sw	zero,%lo(BoolGlob)(a5)
	ret
	.size	Proc5, .-Proc5
	.align	2
	.globl	Proc6
	.type	Proc6, @function
Proc6:
	li	a5,2
	beq	a0,a5,.L11
	li	a4,3
	sw	a4,0(a1)
	li	a4,1
	beq	a0,a4,.L12
	beqz	a0,.L16
	li	a4,4
	beq	a0,a4,.L14
	ret
.L14:
	sw	a5,0(a1)
.L10:
	ret
.L12:
	lui	a5,%hi(IntGlob)
	lw	a4,%lo(IntGlob)(a5)
	li	a5,100
	ble	a4,a5,.L10
.L16:
	sw	zero,0(a1)
	ret
.L11:
	li	a5,1
	sw	a5,0(a1)
	ret
	.size	Proc6, .-Proc6
	.align	2
	.globl	Proc1
	.type	Proc1, @function
Proc1:
	addi	sp,sp,-16
	sw	s0,8(sp)
	lui	s0,%hi(PtrGlb)
	lw	a5,%lo(PtrGlb)(s0)
	lw	a4,0(a0)
	sw	ra,12(sp)
	lw	a3,0(a5)
	lw	a1,40(a5)
	lw	t2,4(a5)
	lw	t0,8(a5)
	lw	t6,12(a5)
	lw	t5,16(a5)
	lw	t4,20(a5)
	lw	t3,24(a5)
	lw	t1,28(a5)
	lw	a7,32(a5)
	lw	a6,36(a5)
	lw	a2,44(a5)
	sw	s1,4(sp)
	sw	a3,0(a4)
	lw	a3,0(a0)
	sw	a1,40(a4)
	sw	t2,4(a4)
	sw	t0,8(a4)
	sw	t6,12(a4)
	sw	t5,16(a4)
	sw	t4,20(a4)
	sw	t3,24(a4)
	sw	t1,28(a4)
	sw	a7,32(a4)
	sw	a6,36(a4)
	sw	a2,44(a4)
	li	a4,5
	sw	a4,12(a0)
	sw	a3,0(a3)
	lw	a2,0(a0)
	lw	a5,0(a5)
	sw	a4,12(a3)
	sw	a5,0(a2)
	lw	a1,0(a0)
	lui	a5,%hi(IntGlob)
	lw	a5,%lo(IntGlob)(a5)
	lw	a3,%lo(PtrGlb)(s0)
	lw	a4,4(a1)
	addi	a5,a5,12
	sw	a5,12(a3)
	beqz	a4,.L21
	lw	t0,0(a1)
	lw	t6,8(a1)
	lw	t5,12(a1)
	lw	t4,16(a1)
	lw	t3,20(a1)
	lw	t1,24(a1)
	lw	a7,28(a1)
	lw	a6,32(a1)
	lw	a2,36(a1)
	lw	a3,40(a1)
	lw	a5,44(a1)
	lw	ra,12(sp)
	lw	s0,8(sp)
	sw	t0,0(a0)
	sw	a4,4(a0)
	sw	t6,8(a0)
	sw	t5,12(a0)
	sw	t4,16(a0)
	sw	t3,20(a0)
	sw	t1,24(a0)
	sw	a7,28(a0)
	sw	a6,32(a0)
	sw	a2,36(a0)
	sw	a3,40(a0)
	sw	a5,44(a0)
	lw	s1,4(sp)
	addi	sp,sp,16
	jr	ra
.L21:
	mv	s1,a0
	lw	a0,8(a0)
	li	a5,6
	sw	a5,12(a1)
	addi	a1,a1,8
	call	Proc6
	lw	a4,%lo(PtrGlb)(s0)
	lw	a5,0(s1)
	lw	ra,12(sp)
	lw	a4,0(a4)
	lw	s0,8(sp)
	sw	a4,0(a5)
	lw	a4,0(s1)
	lw	s1,4(sp)
	lw	a5,12(a4)
	addi	a5,a5,12
	sw	a5,12(a4)
	addi	sp,sp,16
	jr	ra
	.size	Proc1, .-Proc1
	.align	2
	.globl	Proc7
	.type	Proc7, @function
Proc7:
	addi	a0,a0,2
	add	a1,a0,a1
	sw	a1,0(a2)
	ret
	.size	Proc7, .-Proc7
	.align	2
	.globl	Proc8
	.type	Proc8, @function
Proc8:
	addi	a4,a2,5
	li	a6,204
	mul	a6,a4,a6
	slli	a2,a2,2
	slli	a5,a4,2
	add	a0,a0,a5
	sw	a3,0(a0)
	sw	a4,120(a0)
	sw	a3,4(a0)
	add	a5,a6,a2
	add	a5,a1,a5
	lw	a3,16(a5)
	sw	a4,20(a5)
	sw	a4,24(a5)
	addi	a4,a3,1
	sw	a4,16(a5)
	lw	a5,0(a0)
	add	a1,a1,a6
	add	a1,a1,a2
	li	a2,4096
	add	a1,a2,a1
	sw	a5,4(a1)
	li	a4,5
	lui	a5,%hi(IntGlob)
	sw	a4,%lo(IntGlob)(a5)
	ret
	.size	Proc8, .-Proc8
	.align	2
	.globl	Func1
	.type	Func1, @function
Func1:
	sub	a0,a0,a1
	seqz	a0,a0
	ret
	.size	Func1, .-Func1
	.align	2
	.globl	Func2
	.type	Func2, @function
Func2:
	addi	sp,sp,-16
	sw	ra,12(sp)
	lbu	a4,1(a0)
	lbu	a5,2(a1)
.L26:
	beq	a4,a5,.L26
	call	strcmp
	lw	ra,12(sp)
	sgt	a0,a0,zero
	addi	sp,sp,16
	jr	ra
	.size	Func2, .-Func2
	.align	2
	.globl	Proc0
	.type	Proc0, @function
Proc0:
	addi	sp,sp,-192
	li	a0,48
	sw	ra,188(sp)
	sw	s0,184(sp)
	sw	s1,180(sp)
	sw	s2,176(sp)
	sw	s3,172(sp)
	sw	s4,168(sp)
	sw	s5,164(sp)
	sw	s6,160(sp)
	sw	s7,156(sp)
	sw	s8,152(sp)
	sw	s9,148(sp)
	sw	s10,144(sp)
	sw	s11,140(sp)
	call	malloc
	mv	s0,a0
	lui	a5,%hi(PtrGlbNext)
	li	a0,48
	sw	s0,%lo(PtrGlbNext)(a5)
	call	malloc
	lui	a3,%hi(.LC0)
	addi	a5,a3,%lo(.LC0)
	lw	a3,%lo(.LC0)(a3)
	lhu	a6,28(a5)
	lw	t6,4(a5)
	lw	t5,8(a5)
	lw	t4,12(a5)
	lw	t3,16(a5)
	lw	t1,20(a5)
	lw	a7,24(a5)
	lbu	a2,30(a5)
	li	a5,2
	sw	a5,8(a0)
	li	a5,40
	mv	a4,a0
	sw	s0,0(a0)
	sw	a5,12(a0)
	sw	a3,16(a0)
	sw	zero,4(a0)
	lui	s2,%hi(Array2Glob)
	sh	a6,44(a4)
	li	a5,10
	addi	a3,s2,%lo(Array2Glob)
	sw	t6,20(a4)
	sw	t5,24(a4)
	sw	t4,28(a4)
	sw	t3,32(a4)
	sw	t1,36(a4)
	sw	a7,40(a4)
	sb	a2,46(a4)
	lui	s1,%hi(PtrGlb)
	li	a1,0
	addi	a0,sp,32
	sw	a4,%lo(PtrGlb)(s1)
	sw	a5,1660(a3)
	call	gettimeofday
	lui	a4,%hi(.LC1)
	addi	a5,a4,%lo(.LC1)
	lw	s11,%lo(.LC1)(a4)
	lw	a4,12(a5)
	lw	s10,4(a5)
	lw	s9,8(a5)
	sw	a4,12(sp)
	lw	a4,16(a5)
	li	a6,9998336
	addi	s8,a6,1664
	sw	a4,16(sp)
	lw	a4,20(a5)
	lui	s6,%hi(Char1Glob)
	lui	s0,%hi(BoolGlob)
	sw	a4,20(sp)
	lw	a4,24(a5)
	lui	s7,%hi(Char2Glob)
	lui	s5,%hi(Array1Glob)
	sw	a4,24(sp)
	lhu	a4,28(a5)
	lbu	a5,30(a5)
	li	s4,65
	sh	a4,28(sp)
	sb	a5,31(sp)
	li	s3,66
.L32:
	lw	a5,12(sp)
	addi	a1,sp,96
	addi	a0,sp,64
	sw	a5,108(sp)
	lw	a5,16(sp)
	sb	s4,%lo(Char1Glob)(s6)
	sw	zero,%lo(BoolGlob)(s0)
	sw	a5,112(sp)
	lw	a5,20(sp)
	sb	s3,%lo(Char2Glob)(s7)
	sw	s11,96(sp)
	sw	a5,116(sp)
	lw	a5,24(sp)
	sw	s10,100(sp)
	sw	s9,104(sp)
	sw	a5,120(sp)
	lhu	a5,28(sp)
	sh	a5,124(sp)
	lbu	a5,31(sp)
	sb	a5,126(sp)
	call	Func2
	seqz	a5,a0
	li	a3,7
	li	a2,3
	addi	a1,s2,%lo(Array2Glob)
	addi	a0,s5,%lo(Array1Glob)
	sw	a5,%lo(BoolGlob)(s0)
	call	Proc8
	lw	a0,%lo(PtrGlb)(s1)
	call	Proc1
	lbu	a4,%lo(Char2Glob)(s7)
	li	a5,64
	bleu	a4,a5,.L30
	li	a5,65
.L31:
	addi	a5,a5,1
	andi	a5,a5,0xff
	bleu	a5,a4,.L31
.L30:
	addi	s8,s8,-1
	bnez	s8,.L32
	li	a1,0
	addi	a0,sp,48
	call	gettimeofday
	lw	a5,48(sp)
	li	a4,999424
	addi	a4,a4,576
	mul	a5,a5,a4
	lw	a3,32(sp)
	li	a2,9998336
	lui	a1,%hi(Version)
	lui	a0,%hi(.LC2)
	addi	a2,a2,1664
	addi	a1,a1,%lo(Version)
	addi	a0,a0,%lo(.LC2)
	mul	a4,a3,a4
	lw	a3,40(sp)
	sub	a3,a5,a3
	lw	a5,56(sp)
	add	a3,a3,a5
	li	a5,710000640
	sub	a3,a3,a4
	srai	a4,a3,3
	addi	a5,a5,-640
	div	a4,a5,a4
	call	printf
	lw	ra,188(sp)
	lw	s0,184(sp)
	lw	s1,180(sp)
	lw	s2,176(sp)
	lw	s3,172(sp)
	lw	s4,168(sp)
	lw	s5,164(sp)
	lw	s6,160(sp)
	lw	s7,156(sp)
	lw	s8,152(sp)
	lw	s9,148(sp)
	lw	s10,144(sp)
	lw	s11,140(sp)
	addi	sp,sp,192
	jr	ra
	.size	Proc0, .-Proc0
	.section	.text.startup,"ax",@progbits
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-16
	sw	ra,12(sp)
	call	Proc0
	li	a0,0
	call	exit
	.size	main, .-main
	.text
	.align	2
	.globl	Func3
	.type	Func3, @function
Func3:
	addi	a0,a0,-2
	seqz	a0,a0
	ret
	.size	Func3, .-Func3
	.comm	PtrGlbNext,4,4
	.comm	PtrGlb,4,4
	.comm	Array2Glob,10404,4
	.comm	Array1Glob,204,4
	.comm	Char2Glob,1,1
	.comm	Char1Glob,1,1
	.comm	BoolGlob,4,4
	.comm	IntGlob,4,4
	.globl	Version
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
.LC0:
	.string	"DHRYSTONE PROGRAM, SOME STRING"
	.zero	1
.LC2:
	.string	"Dhrystone(%s), %ld passes, %ld microseconds, %ld DMIPS\n"
.LC1:
	.string	"DHRYSTONE PROGRAM, 2'ND STRING"
	.section	.sdata,"aw"
	.align	2
	.type	Version, @object
	.size	Version, 7
Version:
	.string	"1.1-mc"
	.ident	"GCC: (GNU) 8.2.0"
