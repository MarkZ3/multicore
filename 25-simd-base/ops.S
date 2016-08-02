#
# x86_64 calling convention:
# https://en.wikipedia.org/wiki/X86_calling_conventions
#
# The first six integer or pointer arguments are passed in
# registers RDI, RSI, RDX, RCX, R8, and R9,
# while XMM0-7 are used for floating point arguments.
#

.global hello, add2, add10_v1, add10_v2, min3_v1, max3_v1, eq2, fadd2

.text

# the classical hello world test
msg:
        .ascii  "hello\n"
msg_len = . - msg
hello:
    mov     $1, %rax
    mov     $1, %rdi
    mov     $msg, %rsi
    mov     $msg_len, %rdx
    syscall
    ret

# add two integers, do not modify arguments
add2:
    mov %rdi, %rax
    add %rsi, %rax
    ret

add10_v1:
    # arguments in registers
    mov %rdi, %rax
    add %rsi, %rax
    add %rdx, %rax
    add %rcx, %rax
    add %r8, %rax
    add %r9, %rax

    # arguments on the stack
    add 0x08(%rsp), %rax
    add 0x10(%rsp), %rax
    add 0x18(%rsp), %rax
    add 0x20(%rsp), %rax

    ret

add10_v2:
    # arguments in registers
    mov %rdi, %rax
    add %rsi, %rax
    add %rdx, %rax
    add %rcx, %rax
    add %r8, %rax
    add %r9, %rax

    # aguments on the stack
    # using base pointer instead of stack pointer
    push %rbp
    mov %rsp, %rbp
    add 0x10(%rbp), %rax
    add 0x18(%rbp), %rax
    add 0x20(%rbp), %rax
    add 0x28(%rbp), %rax
    pop %rbp

    ret

# return the minimum value of three numbers
# update rax to the lowest number
# local label 1f means goto label 1 defined Forward
# can also be 1b if the label 1 was before (usually for loops)
min3_v1:
    mov %rdi, %rax # rax = a
    mov %rsi, %rcx # rcx = b
    cmp %rcx, %rax # if (rax <= rcx) goto 1
    jle 1f
    mov %rcx, %rax # else rax = rcx
1:
    mov %rdx, %rcx # rcx = c
    cmp %rcx, %rax # if (rax <= rcx) goto 2
    jle 2f
    mov %rcx, %rax # else rax = rcx
2:
    ret

max3_v1:
    mov %rdi, %rax # rax = a
    mov %rsi, %rcx # rcx = b
    cmp %rcx, %rax # if (rax >= rcx) goto 1
    jge 1f
    mov %rcx, %rax # else rax = rcx
1:
    mov %rdx, %rcx # rcx = c
    cmp %rcx, %rax # if (rax >= rcx) goto 2
    jge 2f
    mov %rcx, %rax # else rax = rcx
2:
    ret

eq2:
    cmp %rdi, %rsi
    sete %al # if (rsi == rsi) al = 1 else al = 0
    ret

fadd2:
    # addss: Add Scalar Single-precision floating point
    addss %xmm1, %xmm0 # xmm0 = xmm0 + xmm1
    # floating point return value in xmm0 (instead of rax for integer)
    ret
