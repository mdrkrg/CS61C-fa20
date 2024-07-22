.globl relu

.text
# ==============================================================================
# FUNCTION: Performs an inplace element-wise ReLU on an array of ints
# Arguments:
# 	a0 (int*) is the pointer to the array
#	a1 (int)  is the # of elements in the array
# Returns:
#	None
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 78.
# ==============================================================================
relu:
    # Prologue
    addi sp, sp -8
    sw   s0, 0(sp)
    sw   s1, 4(sp)

    blt  zero, a1, loop_start
    li   a0, 17
    li   a1, 78
    ecall         # exit 78

loop_start:
    mv   s0, a0   # load array address
    mv   s1, a1   # load array length
    li   t0, 0    # loop count

loop_continue:
    bge  t0, s1, loop_end
    slli t1, t0, 2  # shift to word size
    add  t2, t1, s0 # pointer to element
    lw   t3, 0(t2)  # load element
    addi t0, t0, 1

    bge  t3, zero, loop_continue
    li   t3, 0
    sw   t3, 0(t2)  # save back 0 to array
    j loop_continue

loop_end:

    # Epilogue
    lw   s0, 0(sp)
    lw   s1, 4(sp)
    addi sp, sp, 8
	  ret
