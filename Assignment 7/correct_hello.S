# This program includes the _exit system call so that no SIGSEGV signal is sent by the kernel

.global _start

_start:
    	mov $1, %rax
    	mov $1, %rdi
    	mov $msg, %rsi
    	mov $15, %rdx
    	syscall

    	mov $60, %rax
    	mov $122, %rdi
    	syscall

    	msg:
    	.ascii "Hello World...\n"
        
        
