.globl matmul

.text
# =======================================================
# FUNCTION: Matrix Multiplication of 2 integer matrices
# 	d = matmul(m0, m1)
# Arguments:
# 	a0 (int*)  is the pointer to the start of m0 
#	a1 (int)   is the # of rows (height) of m0
#	a2 (int)   is the # of columns (width) of m0
#	a3 (int*)  is the pointer to the start of m1
# 	a4 (int)   is the # of rows (height) of m1
#	a5 (int)   is the # of columns (width) of m1
#	a6 (int*)  is the pointer to the the start of d
# Returns:
#	None (void), sets d = matmul(m0, m1)
# Exceptions:
#   Make sure to check in top to bottom order!
#   - If the dimensions of m0 do not make sense,
#     this function terminates the program with exit code 72.
#   - If the dimensions of m1 do not make sense,
#     this function terminates the program with exit code 73.
#   - If the dimensions of m0 and m1 don't match,
#     this function terminates the program with exit code 74.
# =======================================================
matmul:

    # Error checks
    bge  zero, a1, throw_mat0
    bge  zero, a2, throw_mat0
    bge  zero, a4, throw_mat1
    bge  zero, a5, throw_mat1
    bne  a2, a4, throw_mismatch


    # Prologue
    addi sp, sp, -32
    sw   ra, 0(sp)
    sw   s0, 4(sp)
    sw   s1, 8(sp)
    sw   s2, 12(sp)
    sw   s3, 16(sp)
    sw   s4, 20(sp)
    sw   s5, 24(sp)
    sw   s6, 28(sp)
    mv   s0, a0       # m0(m * n)
    mv   s1, a1       # m
    mv   s2, a2       # n
    mv   s3, a3       # m1(n * k)
    mv   s4, a4       # n
    mv   s5, a5       # k
    mv   s6, a6       # d

    mv   t6, s6       # ptr = d

    li   t0, 0        # i = 0
    mv   t2, s0       # m0_ptr = m0

outer_loop_start:

    li   t1, 0        # j = 0
    mv   t3, s3       # m1_ptr = m1

inner_loop_start:

    mv   a0, t2
    mv   a1, t3
    mv   a2, s2       # length = n
    li   a3, 1        # row stride = 1
    mv   a4, s5       # col stride = k

    addi sp, sp, -20
    sw   t0, 0(sp)
    sw   t1, 4(sp)
    sw   t2, 8(sp)
    sw   t3, 12(sp)
    sw   t6, 16(sp)

    jal  dot
    mv   t4, a0       # result of product

    lw   t0, 0(sp)
    lw   t1, 4(sp)
    lw   t2, 8(sp)
    lw   t3, 12(sp)
    lw   t6, 16(sp)
    addi sp, sp, 20

    # calculate the pointer to d[i][j]
    sw   t4, 0(t6)
    addi t6, t6, 4    # ptr++
    addi t1, t1, 1    # j++
    addi t3, t3, 4    # m1_ptr++

    blt  t1, s5, inner_loop_start

inner_loop_end:
    addi t0, t0, 1    # i++
    slli t5, s2, 2
    add  t2, t2, t5   # m0_ptr points at next row

    blt  t0, s1, outer_loop_start

outer_loop_end:


    # Epilogue
    lw   ra, 0(sp)
    lw   s0, 4(sp)
    lw   s1, 8(sp)
    lw   s2, 12(sp)
    lw   s3, 16(sp)
    lw   s4, 20(sp)
    lw   s5, 24(sp)
    lw   s6, 28(sp)
    addi sp, sp, 32

    ret

throw_mat0:
    li   a0, 17
    li   a1, 72
    ecall

throw_mat1:
    li   a0, 17
    li   a1, 73
    ecall

throw_mismatch:
    li   a0, 17
    li   a1, 74
    ecall
