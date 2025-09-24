.global setjmp2
.global longjmp2

setjmp2:
	mov (%rsp), %rax
	mov %rax, 72(%rdi)
	lea 8(%rsp), %rax
	mov %rax, (%rdi)

	mov %rbp,  8(%rdi)
	mov %rbx, 16(%rdi)
	mov %rsi, 24(%rdi)
	mov %r12, 32(%rdi)
	mov %r13, 40(%rdi)
	mov %r14, 48(%rdi)
	mov %r15, 56(%rdi)
	mov %rdi, 64(%rdi)

	xor %rax, %rax # always return 0 when not longjmping

	ret

longjmp2:
	mov   (%rdi), %rsp
	mov  8(%rdi), %rbp
	mov 16(%rdi), %rbx
	mov 24(%rdi), %rsi
	mov 32(%rdi), %r12
	mov 40(%rdi), %r13
	mov 48(%rdi), %r14
	mov 56(%rdi), %r15
	mov 72(%rdi), %rax # rax will never be 0 (if it is then its a bug)
	mov 64(%rdi), %rdi

	test %rax, %rax
	jz failure

	push %rax

	mov %rsp, %rbp
	ret

failure:
	ud2

