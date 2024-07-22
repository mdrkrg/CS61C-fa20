.globl argmax

.text
# =================================================================
# FUNCTION: Given a int vector, return the index of the largest
#	element. If there are multiple, return the one
#	with the smallest index.
# Arguments:
# 	a0 (int*) is the pointer to the start of the vector
#	a1 (int)  is the # of elements in the vector
# Returns:
#	a0 (int)  is the first index of the largest element
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 77.
# =================================================================
argmax:

    # Prologue
    addi sp, sp, -8
    sw   s0, 0(sp)
    sw   s1, 4(sp)

    blt  zero, a1, loop_start
    li   a0, 17
    li   a1, 77
    ecall

loop_start:
    mv   s0, a0
    mv   s1, a1
    li   t0, 0      # count
    lw   t1, 0(s0)  # load t1 to be max
    li   t2, 0      # index of max

loop_continue:
    addi t0, t0, 1
    beq  t0, s1, loop_end # count == size, end loop
    slli t3, t0, 2
    add  t4, t3, s0 # pointer
    lw   t5, 0(t4)
    bge  t1, t5, loop_continue # max >= curr, continue

    mv   t1, t5
    mv   t2, t0
    j loop_continue

loop_end:
    mv   a0, t2
    # Epilogue
    lw   s0, 0(sp)
    lw   s1, 4(sp)
    addi sp, sp, 8

    ret
