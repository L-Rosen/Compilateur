				# This code was produced by the CERI Compiler
	.data
a:	.quad 0
b:	.quad 0
c:	.quad 0
z:	.quad 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $8
	push $3
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	je Vrai1
	push $0
	jmp Suite1
Vrai1:	push $0xFFFFFFFFFFFFFFFF
Suite1:
	push $4
	push $2
	push $2
	pop %rbx
	pop %rax
	mulq %rbx
	push %rax
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	je Vrai2
	push $0
	jmp Suite2
Vrai2:	push $0xFFFFFFFFFFFFFFFF
Suite2:
	pop %rbx
	pop %rax
	addq	%rbx, %rax
	push %rax
	pop z
	push $5
	push $65
	pop %rbx
	pop %rax
	movq $0, %rdx
	divq %rbx
	push %rax
	push $2
	pop %rbx
	pop %rax
	addq	%rbx, %rax
	push %rax
	push $7
	push $5
	pop %rbx
	pop %rax
	movq $0, %rdx
	divq %rbx
	push %rdx
	pop %rbx
	pop %rax
	cmpq %rbx, %rax
	jb Vrai3
	push $0
	jmp Suite3
Vrai3:	push $0xFFFFFFFFFFFFFFFF
Suite3:
	pop b
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
