# This program does not include the _exit system call, which ultimately causes an error

.global _start

_start:
    	mov $1, %rax
    	mov $1, %rdi;
    	mov $msg, %rsi
    	mov $15, %rdx
    	syscall

    	msg:
    	.ascii "Hello World...\n"
        
        
