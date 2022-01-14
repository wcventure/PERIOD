    .text
    .globl _PR_x86_64_AtomicIncrement
    .align 4
_PR_x86_64_AtomicIncrement:
    movl $1, %eax
    lock
    xaddl %eax, (%rdi)
    incl %eax
    ret

    .text
    .globl _PR_x86_64_AtomicDecrement
    .align 4
_PR_x86_64_AtomicDecrement:
    movl $-1, %eax
    lock
    xaddl %eax, (%rdi)
    decl %eax
    ret

    .text
    .globl _PR_x86_64_AtomicSet
    .align 4
_PR_x86_64_AtomicSet:
    movl %esi, %eax
    xchgl %eax, (%rdi)
    ret


    .text
    .globl _PR_x86_64_AtomicAdd
    .align 4
_PR_x86_64_AtomicAdd:
    movl %esi, %eax
    lock
    xaddl %eax, (%rdi)
    addl %esi, %eax
    ret

.section .note.GNU-stack, "", @progbits ; .previous
