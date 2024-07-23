.globl classify

.text
classify:
    # =====================================
    # COMMAND LINE ARGUMENTS
    # =====================================
    # Args:
    #   a0 (int)    argc
    #   a1 (char**) argv
    #   a2 (int)    print_classification, if this is zero, 
    #               you should print the classification. Otherwise,
    #               this function should not print ANYTHING.
    # Returns:
    #   a0 (int)    Classification
    # Exceptions:
    # - If there are an incorrect number of command line args,
    #   this function terminates the program with exit code 89.
    # - If malloc fails, this function terminats the program with exit code 88.
    #
    # Usage:
    #   main.s <M0_PATH> <M1_PATH> <INPUT_PATH> <OUTPUT_PATH>

    li   t0, 5
    bne  a0, t0, throw_incorrect_argument

    addi sp, sp, -52
    sw   ra, 0(sp)
    sw   s0, 4(sp)
    sw   s1, 8(sp)
    sw   s2, 12(sp)
    sw   s3, 16(sp) # m0_ptr
    sw   s4, 20(sp) # m0_row, later be temp_row
    sw   s5, 24(sp) # m0_col, later be temp_col
    sw   s6, 28(sp) # m1_ptr, later be result of classify
    sw   s7, 32(sp) # m1_row
    sw   s8, 36(sp) # m1_col
    sw   s9, 40(sp) # input_ptr
    sw   s10, 44(sp) # input_row
    sw   s11, 48(sp) # input_col

    lw   t0, 4(a1)  # M0_PATH
    lw   t1, 8(a1)  # M1_PATH
    lw   s0, 12(a1) # INPUT_PATH, later be temp_ptr
    lw   s1, 16(a1) # OUTPUT_PATH
    mv   s2, a2     # print_classification

	  # =====================================
    # LOAD MATRICES
    # =====================================

    addi sp, sp, -4
    sw   t1, 0(sp)

    # Load pretrained m0
    mv   a0, t0
    addi sp, sp, -8
    mv   a1, sp
    addi a2, sp, 4
    jal  read_matrix
    lw   s4, 0(sp)
    lw   s5, 4(sp)
    addi sp, sp, 8
    mv   s3, a0
    ebreak

    # Load pretrained m1
    lw   t1, 0(sp)
    addi sp, sp, 4
    mv   a0, t1
    addi sp, sp, -8
    mv   a1, sp
    addi a2, sp, 4
    jal  read_matrix
    lw   s7, 0(sp)
    lw   s8, 4(sp)
    addi sp, sp, 8
    mv   s6, a0
    ebreak

    # Load input matrix
    mv   a0, s0
    addi sp, sp, -8
    mv   a1, sp
    addi a2, sp, 4
    jal  read_matrix
    lw   s10, 0(sp)
    lw   s11, 4(sp)
    addi sp, sp, 8
    mv   s9, a0
    ebreak

    # =====================================
    # RUN LAYERS
    # =====================================
    # 1. LINEAR LAYER:    m0 * input
    # 2. NONLINEAR LAYER: ReLU(m0 * input)
    # 3. LINEAR LAYER:    m1 * ReLU(m0 * input)

    # let s0 be *temp_ptr

    # m0 * input
    mul  t0, s4, s11    # m * n, n * k, count = m * k
    slli a0, t0, 2
    jal  malloc
    beq  a0, zero, throw_malloc # null pointer exception
    mv   s0, a0

    mv   a0, s3
    mv   a1, s4
    mv   a2, s5
    mv   a3, s9
    mv   a4, s10
    mv   a5, s11
    mv   a6, s0
    jal  matmul
    mv   s0, a6
    ebreak

    # free m0
    mv   a0, s3
    jal  free

    # set temp_row, temp_col
    mv   s4, s4
    mv   s5, s11

    # ReLU(m0 * input)
    mv   a0, s0
    mul  a1, s4, s5
    jal  relu
    ebreak

    # m1 * ReLU(m0 * input)
    mul  t0, s7, s5    # r * m, m * k, count = r * k
    slli a0, t0, 2
    jal  malloc
    beq  a0, zero, throw_malloc # null pointer exception
    mv   t0, a0        # t0 is now temp2_ptr
    addi sp, sp, -4
    sw   t0, 0(sp)

    mv   a0, s6
    mv   a1, s7
    mv   a2, s8
    mv   a3, s0
    mv   a4, s4
    mv   a5, s5
    mv   a6, t0
    jal  matmul
    ebreak

    lw   t0, 0(sp)
    addi sp, sp, 4

    # free temp_ptr
    addi sp, sp, -4
    sw   t0, 0(sp)

    mv   a0, s0
    jal  free

    # set output_ptr, output_row, output_col
    mv   s4, s7
    lw   s0, 0(sp)      # now s0 = temp2_ptr = output
    addi sp, sp, 4

    # free m1
    mv   a0, s6
    jal  free
    ebreak

    # =====================================
    # WRITE OUTPUT
    # =====================================
    # Write output matrix

    mv   a0, s1
    mv   a1, s0
    mv   a2, s4
    mv   a3, s5
    jal  write_matrix
    ebreak

    # =====================================
    # CALCULATE CLASSIFICATION/LABEL
    # =====================================
    # Call argmax

    mul  a1, s4, s5
    mv   a0, s0
    jal  argmax
    mv   s6, a0
    ebreak

    # Print classification
    bne  s2, zero, not_print_classification

    mv   a1, s6
    jal  print_int
    ebreak

    # Print newline afterwards for clarity
    li   a1, '\n'
    jal  print_char
    ebreak

not_print_classification:
    mv   a0, s6

    lw   ra, 0(sp)
    lw   s0, 4(sp)
    lw   s1, 8(sp)
    lw   s2, 12(sp)
    lw   s3, 16(sp) # m0_ptr
    lw   s4, 20(sp) # m0_row, later be temp_row
    lw   s5, 24(sp) # m0_col, later be temp_col
    lw   s6, 28(sp) # m1_ptr, later be result of classify
    lw   s7, 32(sp) # m1_row
    lw   s8, 36(sp) # m1_col
    lw   s9, 40(sp) # input_ptr
    lw   s10, 44(sp) # input_row
    lw   s11, 48(sp) # input_col
    addi sp, sp, 52

    ret

throw_incorrect_argument:
    li   a1, 89
    jal  exit2

throw_malloc:
    li   a1, 88
    jal  exit2
