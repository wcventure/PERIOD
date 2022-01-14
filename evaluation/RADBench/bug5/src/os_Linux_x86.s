
    .text
    .globl _PR_x86_AtomicIncrement
    .align 4
_PR_x86_AtomicIncrement:
    movl 4(%esp), %ecx
    movl $1, %eax
    lock
    xaddl %eax, (%ecx)
    incl %eax
    ret

    .text
    .globl _PR_x86_AtomicDecrement
    .align 4
_PR_x86_AtomicDecrement:
    movl 4(%esp), %ecx
    movl $-1, %eax
    lock
    xaddl %eax, (%ecx)
    decl %eax
    ret

    .text
    .globl _PR_x86_AtomicSet
    .align 4
_PR_x86_AtomicSet:
    movl 4(%esp), %ecx
    movl 8(%esp), %eax
    xchgl %eax, (%ecx)
    ret

    .text
    .globl _PR_x86_AtomicAdd
    .align 4
_PR_x86_AtomicAdd:
    movl 4(%esp), %ecx
    movl 8(%esp), %eax
    movl %eax, %edx
    lock
    xaddl %eax, (%ecx)
    addl %edx, %eax
    ret

.section .note.GNU-stack, "", @progbits ; .previous
