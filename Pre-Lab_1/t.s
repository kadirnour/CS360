	.file	"t.c"
	.comm	FP,4,4
	.section	.rodata
.LC0:
	.string	"declaring vars"
.LC1:
	.string	"enter main"
.LC2:
	.string	"&argc=%x argv=%x env=%x\n"
.LC3:
	.string	"FRAME POINTER = %x\n"
.LC4:
	.string	"&a=%8x &b=%8x &c=%8x\n"
.LC5:
	.string	"argc=%d\n"
.LC6:
	.string	"argv=%s, "
.LC7:
	.string	"exit main"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	andl	$-16, %esp
	subl	$48, %esp
	call	getebp
	movl	%eax, 44(%esp)
	movl	$.LC0, (%esp)
	call	puts
	movl	$.LC1, (%esp)
	call	puts
	movl	16(%ebp), %eax
	movl	%eax, 12(%esp)
	movl	12(%ebp), %eax
	movl	%eax, 8(%esp)
	leal	8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC2, (%esp)
	call	printf
	movl	44(%esp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC3, (%esp)
	call	printf
	leal	36(%esp), %eax
	movl	%eax, 12(%esp)
	leal	32(%esp), %eax
	movl	%eax, 8(%esp)
	leal	28(%esp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC4, (%esp)
	call	printf
	movl	8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC5, (%esp)
	call	printf
	movl	$0, 40(%esp)
	jmp	.L2
.L3:
	movl	40(%esp), %eax
	leal	0(,%eax,4), %edx
	movl	12(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	movl	%eax, 4(%esp)
	movl	$.LC6, (%esp)
	call	printf
	addl	$1, 40(%esp)
.L2:
	movl	8(%ebp), %eax
	cmpl	%eax, 40(%esp)
	jl	.L3
	movl	$10, (%esp)
	call	putchar
	movl	$1, 28(%esp)
	movl	$2, 32(%esp)
	movl	$3, 36(%esp)
	movl	32(%esp), %edx
	movl	28(%esp), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	A
	movl	$.LC7, (%esp)
	call	puts
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.section	.rodata
.LC8:
	.string	"enter A"
.LC9:
	.string	"&d=%8x &e=%8x &f=%8x\n"
.LC10:
	.string	"exit A"
	.text
	.globl	A
	.type	A, @function
A:
.LFB3:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	$.LC8, (%esp)
	call	puts
	leal	-16(%ebp), %eax
	movl	%eax, 12(%esp)
	leal	-20(%ebp), %eax
	movl	%eax, 8(%esp)
	leal	-24(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC9, (%esp)
	call	printf
	call	getebp
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC3, (%esp)
	call	printf
	movl	$4, -24(%ebp)
	movl	$5, -20(%ebp)
	movl	$6, -16(%ebp)
	movl	-20(%ebp), %edx
	movl	-24(%ebp), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	B
	movl	$.LC10, (%esp)
	call	puts
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE3:
	.size	A, .-A
	.section	.rodata
.LC11:
	.string	"enter B"
.LC12:
	.string	"&g=%8x &h=%8x &i=%8x\n"
.LC13:
	.string	"exit B"
	.text
	.globl	B
	.type	B, @function
B:
.LFB4:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	$.LC11, (%esp)
	call	puts
	leal	-16(%ebp), %eax
	movl	%eax, 12(%esp)
	leal	-20(%ebp), %eax
	movl	%eax, 8(%esp)
	leal	-24(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC12, (%esp)
	call	printf
	call	getebp
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC3, (%esp)
	call	printf
	movl	$7, -24(%ebp)
	movl	$8, -20(%ebp)
	movl	$9, -16(%ebp)
	movl	-20(%ebp), %edx
	movl	-24(%ebp), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	C
	movl	$.LC13, (%esp)
	call	puts
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE4:
	.size	B, .-B
	.section	.rodata
.LC14:
	.string	"enter C"
	.align 4
.LC15:
	.string	"&u=%8x &v=%8x &w=%8x &i=%8x &p=%8x\n"
.LC16:
	.string	"FRAME POINTER = %8x\n"
.LC17:
	.string	"->%8x"
.LC18:
	.string	"Address of p = %x\n"
	.text
	.globl	C
	.type	C, @function
C:
.LFB5:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$72, %esp
	movl	$.LC14, (%esp)
	call	puts
	leal	-12(%ebp), %eax
	movl	%eax, 20(%esp)
	leal	-16(%ebp), %eax
	movl	%eax, 16(%esp)
	leal	-20(%ebp), %eax
	movl	%eax, 12(%esp)
	leal	-24(%ebp), %eax
	movl	%eax, 8(%esp)
	leal	-28(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC15, (%esp)
	call	printf
	movl	$10, -28(%ebp)
	movl	$11, -24(%ebp)
	movl	$12, -20(%ebp)
	movl	$13, -16(%ebp)
	call	getebp
	movl	%eax, FP
	movl	FP, %eax
	movl	%eax, 4(%esp)
	movl	$.LC16, (%esp)
	call	printf
	jmp	.L7
.L8:
	movl	FP, %eax
	movl	%eax, 4(%esp)
	movl	$.LC17, (%esp)
	call	printf
	movl	FP, %eax
	movl	(%eax), %eax
	movl	%eax, FP
.L7:
	movl	FP, %eax
	testl	%eax, %eax
	jne	.L8
	movl	$10, (%esp)
	call	putchar
	leal	-12(%ebp), %eax
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC18, (%esp)
	call	printf
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE5:
	.size	C, .-C
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04.4) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
