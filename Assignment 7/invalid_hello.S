# This program passes an invalid argument to the write system call

.global _start

_start:
    	mov $300, %rax
    	mov $1, %rdi
    	mov $msg, %rsi
    	mov $15, %rdx
    	syscall

    	mov $60, %rax
    	mov $122, %rdi
    	syscall

    	msg:
    	.ascii "Hello World...\n"
      
      
      
