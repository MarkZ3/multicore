# registers RDI, RSI, RDX, RCX, R8, and R9,

.global array_addss_iter, array_addps_vect, movss_ex1, movups_ex1

.text

# void array_add_scalar_iter(float *vector, float scalar, long length);
# rdi = vector
# xmm0 = scalar
# rsi  = length
array_addss_iter:
    cmp $0, %rsi
    jle 2f
1:
    movss %xmm0, %xmm1
    addss (%rdi), %xmm1
    movd %xmm1, %eax
    mov %eax, (%rdi)
    add $4, %rdi # next element
    dec %rsi
    jnz 1b
2:
    ret

# void array_add_scalar_iter(float *vector, float scalar, long length);
# rdi = vector
# xmm0 = scalar
# rsi  = length
array_addps_vect:
    push %rbp
    mov %rsp, %rbp
    cmp $0, %rsi
    jle 2f
    # copy the float 4 times on the stack
    sub $0x10, %rsp
    movd %xmm0, -0x04(%rbp)
    movd %xmm0, -0x08(%rbp)
    movd %xmm0, -0x0C(%rbp)
    movd %xmm0, -0x10(%rbp)
    movups -0x10(%rbp), %xmm0 # put everything in the xmm0 vector
1:
    movups %xmm0, %xmm1
    movups %xmm0, %xmm2
    movups (%rdi), %xmm2
    addps %xmm1, %xmm2
    movups %xmm1, (%rdi)
    add $16, %rdi # next chunk of floats + 16 bytes
    sub $4, %rsi
    jge 1b
    mov %rbp, %rsp
2:
    pop %rbp
    ret

movss_ex1:
    xorps %xmm0,       %xmm0 # clear xmm0
    mov   $0xcafebabe, %eax
    movd  %eax,        %xmm0 # copy
    movss %xmm0,       %xmm1 # copy
    ret

movups_ex1:
    xor %eax, %eax
    push %rbp
    mov %rsp, %rbp

    # copy the float 4 times on the stack
    sub $0x10, %rsp
    mov $0xcafebabe, %eax
    mov %eax, -0x04(%rbp)
    mov %eax, -0x08(%rbp)
    mov %eax, -0x0C(%rbp)
    mov %eax, -0x10(%rbp)
    xorps %xmm0,        %xmm0 # clear xmm0
    movups -0x10(%rbp), %xmm0 # put everything in the xmm0 vector

    mov %rbp, %rsp
    pop %rbp
    ret
