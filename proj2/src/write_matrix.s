.globl write_matrix

.text
# ==============================================================================
# FUNCTION: Writes a matrix of integers into a binary file
# FILE FORMAT:
#   The first 8 bytes of the file will be two 4 byte ints representing the
#   numbers of rows and columns respectively. Every 4 bytes thereafter is an
#   element of the matrix in row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is the pointer to the start of the matrix in memory
#   a2 (int)   is the number of rows in the matrix
#   a3 (int)   is the number of columns in the matrix
# Returns:
#   None
# Exceptions:
# - If you receive an fopen error or eof,
#   this function terminates the program with error code 93.
# - If you receive an fwrite error or eof,
#   this function terminates the program with error code 94.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 95.
# ==============================================================================
write_matrix:

    # Prologue
    addi sp, sp, -24
    sw   ra, 0(sp)
    sw   s1, 4(sp)  # *mat
    sw   s2, 8(sp)  # row
    sw   s3, 12(sp) # col
    sw   s0, 16(sp) # FILE
    sw   s4, 20(sp) # total size

    mv   s1, a1
    mv   s2, a2
    mv   s3, a3

    # test if string == "" or null
    beq  a0, zero, throw_fopen

    lw   t0, 0(a0)
    beq  t0, zero, throw_fopen

    mv   a1, a0
    li   a2, 1      # write permission
    jal  fopen
    li   t0, -1     # error code of fopen
    beq  a0, t0, throw_fopen
    mv   s0, a0     # file descripter


    # write row and col
    mv   a1, s0
    addi sp, sp, -8
    sw   s2, 0(sp)
    sw   s3, 4(sp)
    mv   a2, sp
    li   a3, 2      # write 2 ints
    li   a4, 4
    jal  fwrite
    li   a3, 2
    blt  a0, a3, throw_fwrite  # a0 < a3
    addi sp, sp, 8

    mv   a1, s0     # file
    mv   a2, s1     # buffer
    mul  s4, s2, s3 # row * col
    mv   a3, s4
    li   a4, 4
    jal  fwrite
    mv   t0, s4
    blt  a0, t0, throw_fwrite

    mv   a1, s0
    jal  fclose
    bne  a0, zero, throw_fclose

    # Epilogue
    lw   ra, 0(sp)
    lw   s1, 4(sp)  # *mat
    lw   s2, 8(sp)  # row
    lw   s3, 12(sp) # col
    lw   s0, 16(sp) # FILE
    lw   s4, 20(sp) # total size
    addi sp, sp, 24

    ret

throw_fopen:
    li   a1, 93
    jal  exit2

throw_fwrite:
    # Flush the fd first
    mv   a1, s0
    jal  fflush

    li   a1, 94
    jal  exit2

throw_fclose:
    li   a1, 95
    jal  exit2
