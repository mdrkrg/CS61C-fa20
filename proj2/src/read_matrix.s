.globl read_matrix

.text
# ==============================================================================
# FUNCTION: Allocates memory and reads in a binary file as a matrix of integers
#
# FILE FORMAT:
#   The first 8 bytes are two 4 byte ints representing the # of rows and columns
#   in the matrix. Every 4 bytes afterwards is an element of the matrix in
#   row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is a pointer to an integer, we will set it to the number of rows
#   a2 (int*)  is a pointer to an integer, we will set it to the number of columns
# Returns:
#   a0 (int*)  is the pointer to the matrix in memory
# Exceptions:
# - If malloc returns an error,
#   this function terminates the program with error code 88.
# - If you receive an fopen error or eof, 
#   this function terminates the program with error code 90.
# - If you receive an fread error or eof,
#   this function terminates the program with error code 91.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 92.
# ==============================================================================
read_matrix:

    # Prologue
    addi sp, sp, -32
    sw   ra, 0(sp)
    sw   s1, 4(sp)  # *row
    sw   s2, 8(sp)  # *col
    sw   s3, 12(sp) # FILE
    sw   s4, 16(sp) # num_row
    sw   s5, 20(sp) # num_col
    sw   s6, 24(sp) # num_total
    sw   s7, 28(sp) # *return_mat

    mv   s1, a1
    mv   s2, a2
	
    mv   a1, a0
    li   a2, 0      # read permission
    jal  fopen
    li   t0, -1     # error code of fopen
    beq  a0, t0, throw_fopen
    mv   s3, a0     # file descripter

    # read row
    mv   a1, s3
    mv   a2, s1
    li   a3, 4
    jal  fread

    li   t0, 4
    bne  a0, t0, throw_fread
    lw   s4, 0(s1)

    # read col
    mv   a1, s3
    mv   a2, s2
    li   a3, 4
    jal  fread

    li   t0, 4
    bne  a0, t0, throw_fread
    lw   s5, 0(s2)

    mul  s6, s4, s5   # total number of int to read

    slli a0, s6, 2
    jal  malloc
    beq  a0, zero, throw_malloc
    mv   s7, a0       # memory address of return mat

    mv   a1, s3
    mv   a2, s7
    slli a3, s6, 2
    jal  fread
    slli t0, s6, 2
    bne  a0, t0, throw_fread

    mv   a1, s3
    jal  fclose
    bne  a0, zero, throw_fclose

    mv   a0, s7

    # Epilogue
    lw   ra, 0(sp)
    lw   s1, 4(sp)
    lw   s2, 8(sp)
    lw   s3, 12(sp)
    lw   s4, 16(sp)
    lw   s5, 20(sp)
    lw   s6, 24(sp)
    lw   s7, 28(sp)
    addi sp, sp, 32
    ret

throw_malloc:
    li   a1, 88
    jal  exit2

throw_fopen:
    li   a1, 90
    jal  exit2

throw_fread:
    li   a1, 91
    jal  exit2

throw_fclose:
    li   a1, 92
    jal  exit2
