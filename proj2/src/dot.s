.globl dot

.text
# =======================================================
# FUNCTION: Dot product of 2 int vectors
# Arguments:
#   a0 (int*) is the pointer to the start of v0
#   a1 (int*) is the pointer to the start of v1
#   a2 (int)  is the length of the vectors
#   a3 (int)  is the stride of v0
#   a4 (int)  is the stride of v1
# Returns:
#   a0 (int)  is the dot product of v0 and v1
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 75.
# - If the stride of either vector is less than 1,
#   this function terminates the program with error code 76.
# =======================================================
dot:
    bge  zero, a2, throw_length
    bge  zero, a3, throw_stride
    bge  zero, a4, throw_stride
    # Prologue
    # addi sp, sp, -20
    # sw   s0, 0(sp)    # v0
    # sw   s1, 4(sp)    # v1
    # sw   s2, 8(sp)    # length
    # sw   s3, 12(sp)   # stride 0
    # sw   s4, 16(sp)   # stride 1

    li   t0, 0        # count
    li   t1, 0        # product

loop_start:
    slli t2, t0, 2    # pointer to v0 elem
    mul  t2, t2, a3
    add  t2, t2, a0
    slli t3, t0, 2    # pointer to v1 elem
    mul  t3, t3, a4
    add  t3, t3, a1
    lw   t4, 0(t2)
    lw   t5, 0(t3)
    mul  t6, t4, t5
    add  t1, t1, t6
    addi t0, t0, 1

    blt t0, a2, loop_start # while count < length

loop_end:
    mv   a0, t1

    # Epilogue
    # lw   s0, 0(sp)
    # lw   s1, 4(sp)
    # lw   s2, 8(sp)
    # lw   s3, 12(sp)
    # lw   s4, 16(sp)
    # addi sp, sp, 20
    ret

throw_length:
    li   a0, 17
    li   a1, 75
    ecall

throw_stride:
    li   a0, 17
    li   a1, 76
    ecall
