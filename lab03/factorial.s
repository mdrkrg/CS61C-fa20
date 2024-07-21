.globl factorial

.data
n: .word 8

.text
main:
    la t0, n
    lw a0, 0(t0)
    jal ra, factorial

    addi a1, a0, 0
    addi a0, x0, 1
    ecall # Print Result

    addi a1, x0, '\n'
    addi a0, x0, 11
    ecall # Print newline

    addi a0, x0, 10
    ecall # Exit

factorial:
    # YOUR CODE HERE
    blt x0 a0 recur
    jr ra # Returning 1
recur:
    addi sp sp -8
    sw a0 0(sp)
    sw ra 4(sp)
    addi a0 a0 -1
    jal factorial
    add t0 a0 x0
    lw a0 0(sp)
    lw ra 4(sp)
    addi sp sp 8
    sw a0 0(sp)
    mul a0 a0 t0
    jr ra
