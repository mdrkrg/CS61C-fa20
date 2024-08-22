#include "transpose.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define cache_line_size sysconf(_SC_LEVEL1_DCACHE_SIZE)

/* The naive transpose function as a reference. */
void transpose_naive(int n, int blocksize, int *dst, int *src) {
  for (int x = 0; x < n; x++) {
    for (int y = 0; y < n; y++) {
      dst[y + x * n] = src[x + y * n];
    }
  }
}

void print_matrix(int n, int *mat) {
  for (int y = 0; y < n; y++) {
    for (int x = 0; x < n; x++) {
      printf("%d\t", mat[x + y * n]);
    }
    printf("\n");
  }
  printf("\n");
}

void transcope_inner_main(int n, int blocksize, int *dst, int *src,
                          bool is_x_edge, bool is_y_edge)
    __attribute__((unused));

void transcope_inner_main(int n, int blocksize, int *dst, int *src,
                          bool is_x_edge, bool is_y_edge) {
  int x_range, y_range;
  x_range = is_x_edge ? n % blocksize : blocksize;
  y_range = is_y_edge ? n % blocksize : blocksize;

  for (int x = 0; x < x_range; x++) {
    for (int y = 0; y < y_range; y++) {
      dst[y + x * n] = src[x + y * n];
    }
  }
}

/* Implement cache blocking below. You should NOT assume that n is a
 * multiple of the block size. */
void transpose_blocking(int n, int blocksize, int *dst, int *src) {
  // YOUR CODE HERE
  if (n <= blocksize) {
    for (int x = 0; x < n; x++) {
      for (int y = 0; y < n; y++) {
        dst[y + x * n] = src[x + y * n];
      }
    }
    return;
  }
  int dst_shift_amount, src_shift_amount;
  dst_shift_amount = src_shift_amount = 0;
  int block_range = n / blocksize;
  int edge_range = n % blocksize;
  int remain = n - block_range * blocksize;
  for (int block_x = 0; block_x < block_range; block_x++) {
    for (int block_y = 0; block_y < block_range; block_y++) {
      dst_shift_amount = block_y * blocksize + block_x * n * blocksize;
      src_shift_amount = block_x * blocksize + block_y * n * blocksize;
      // transcope_inner_main(n, blocksize, dst + dst_shift_amount,
      //                      src + src_shift_amount, false, false);
      for (int x = 0; x < blocksize; x++) {
        for (int y = 0; y < blocksize; y++) {
          dst[y + x * n + dst_shift_amount] = src[x + y * n + src_shift_amount];
        }
      }
    }
    if (remain) {
      dst_shift_amount += blocksize;
      src_shift_amount += n * blocksize;
      // transcope_inner_main(n, blocksize, dst + dst_shift_amount,
      //                      src + src_shift_amount, false, true);
      for (int x = 0; x < blocksize; x++) {
        for (int y = 0; y < edge_range; y++) {
          dst[y + x * n + dst_shift_amount] = src[x + y * n + src_shift_amount];
        }
      }
    }
  }
  if (remain) {
    for (int block_y = 0; block_y < block_range; block_y++) {
      dst_shift_amount = block_y * blocksize + block_range * n * blocksize;
      src_shift_amount = block_range * blocksize + block_y * n * blocksize;
      // transcope_inner_main(n, blocksize, dst + dst_shift_amount,
      //                      src + src_shift_amount, true, false);
      for (int x = 0; x < edge_range; x++) {
        for (int y = 0; y < blocksize; y++) {
          dst[y + x * n + dst_shift_amount] = src[x + y * n + src_shift_amount];
        }
      }
    }

    dst_shift_amount += blocksize;
    src_shift_amount += n * blocksize;
    // transcope_inner_main(n, blocksize, dst + dst_shift_amount,
    //                      src + src_shift_amount, true, true);
    for (int x = 0; x < edge_range; x++) {
      for (int y = 0; y < edge_range; y++) {
        dst[y + x * n + dst_shift_amount] = src[x + y * n + src_shift_amount];
      }
    }
  }
}
