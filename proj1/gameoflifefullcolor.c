/************************************************************************
**
** NAME:        gameoflife.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Justin Yokota - Starter Code
**				YOUR NAME HERE
**
**
** DATE:        2020-08-23
**
**************************************************************************/

#include "gameoflifefullcolor.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool isLiveNext(bool is_live, uint8_t neighbor_live, uint32_t rule) {
  uint32_t pos = 1; // 0b 0000 0000 0000 0000 0000 0000 0000 0001
  int shift = neighbor_live + is_live * 9;
  pos = pos << shift;
  return (pos & rule) >> shift;
}

Color *getNewCell(uint8_t r, uint8_t g, uint8_t b) {
  Color *new_cell = malloc(sizeof(Color));

  if (new_cell == NULL) {
    free(new_cell);
    return NULL;
  }

  new_cell->R = r;
  new_cell->G = g;
  new_cell->B = b;
  return new_cell;
}

// Determines what color the cell at the given row/col should be. This function
// allocates space for a new Color. Note that you will need to read the eight
// neighbors of the cell in question. The grid "wraps", so we treat the top row
// as adjacent to the bottom row and the left column as adjacent to the right
// column.
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule) {
  uint8_t cur_r = getRGBValue(image, row, col, R);
  uint8_t cur_g = getRGBValue(image, row, col, G);
  uint8_t cur_b = getRGBValue(image, row, col, B);

  uint8_t *r_neighbor_alive = getNeighborAliveArray(image, row, col, R);
  uint8_t *g_neighbor_alive = getNeighborAliveArray(image, row, col, G);
  uint8_t *b_neighbor_alive = getNeighborAliveArray(image, row, col, B);

  uint8_t next_r = getNextRGBValue(cur_r, r_neighbor_alive, rule);
  uint8_t next_g = getNextRGBValue(cur_g, g_neighbor_alive, rule);
  uint8_t next_b = getNextRGBValue(cur_b, b_neighbor_alive, rule);

  free(r_neighbor_alive);
  free(g_neighbor_alive);
  free(b_neighbor_alive);

  return getNewCell(next_r, next_g, next_b);
}

/* Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule) {
  // YOUR CODE HERE
  bool is_live = isLive(image, row, col);
  uint8_t neighbor_live = getNeighborLive(image, row, col);
  bool is_live_next = isLiveNext(is_live, neighbor_live, rule);
  return getNewCell(is_live_next);
} */

// The main body of Life; given an image and a rule, computes one iteration of
// the Game of Life. You should be able to copy most of this from
// steganography.c
Image *life(Image *image, uint32_t rule) {
  // YOUR CODE HERE
  Image *new_image = malloc(sizeof(Image));
  uint32_t index;
  if (new_image == NULL) {
    free(new_image);
    return NULL;
  }
  new_image->image = calloc(image->rows * image->cols, sizeof(Color *));
  if (new_image->image == NULL) {
    free(new_image->image);
    return NULL;
  }
  new_image->rows = image->rows;
  new_image->cols = image->cols;
  for (int i = 0; i < image->rows; i++) {
    for (int j = 0; j < image->cols; j++) {
      index = i * image->cols + j;
      new_image->image[index] = evaluateOneCell(image, i, j, rule);
      if (new_image->image[index] == NULL) {
        return NULL;
      }
    }
  }
  return new_image;
}

/*
Loads a .ppm from a file, computes the next iteration of the game of life, then
prints to stdout the new image.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a .ppm.
argv[2] should contain a hexadecimal number (such as 0x1808). Note that this
will be a string. You may find the function strtol useful for this conversion.
If the input is not correct, a malloc fails, or any other error occurs, you
should exit with code -1. Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!

You may find it useful to copy the code from steganography.c, to start.
*/
int main(int argc, char **argv) {
  // YOUR CODE HERE
  uint32_t rule;
  Image *read_image;
  Image *eval_image;

  if (argc != 3) {
    printf("Format: command path/to/file.ppm rule(SCNx32)");
    printf("Example: ./gameoflife ./foo.ppm 0x1808");
    exit(-1);
  }

  read_image = readData(argv[1]);
  if (read_image == NULL) {
    printf("Allocation failure occured, exit -1...");
    exit(-1);
  }

  rule = strtol(argv[2], NULL, 16);
  eval_image = life(read_image, rule);
  if (eval_image == NULL) {
    printf("Allocation failure occured, exit -1...");
    exit(-1);
  }

  writeData(eval_image);
  freeImage(read_image);
  freeImage(eval_image);
  return 0;
}

// Helper function to return the &Color of given rol, col
Color *getColor(Image *image, int row, int col) {
  int index = row * image->cols + col;
  return image->image[index];
}

uint8_t getNextRGBValue(uint8_t rgb_val, uint8_t neighbor_alive[8],
                        uint32_t rule) {
  bool this_bit_alive, new_this_bit_alive;
  uint8_t new_rgb_val;
  int shift;
  uint32_t this_bit_pos;
  for (int i = 0; i < 8; i++) {
    // take the MSB
    this_bit_alive = rgb_val & 0b10000000;
    this_bit_pos = 0b1; // 0b 0000 0000 0000 0000 0000 0000 0000 0001

    // push the bit to position
    shift = neighbor_alive[i] + this_bit_alive * 9;
    this_bit_pos = this_bit_pos << shift;

    // whether current bit is alive or not
    new_this_bit_alive = this_bit_pos & rule;

    // push back the previous bit
    new_rgb_val <<= 1;
    new_rgb_val += new_this_bit_alive;
    rgb_val <<= 1;
  }
  return new_rgb_val;
}

// Use neighbor rgb value directly (split into 8 bit depth)
uint8_t getRGBValue(Image *image, int row, int col, RGB color) {
  switch (color) {
  case R:
    return getColor(image, row, col)->R;
  case G:
    return getColor(image, row, col)->G;
  case B:
    return getColor(image, row, col)->B;
  default: // Never reach, suppress warning
    return 0;
  }
}

void updateNeighborAliveArray(uint8_t neighbor_rgb_val,
                              uint8_t neighbor_alive[8]) {
  const uint8_t compare = 0b10000000;
  for (int i = 0; i < 8; i++) {
    if (neighbor_rgb_val & compare) {
      neighbor_alive[i]++;
    }
    neighbor_rgb_val <<= 1;
  }
}

uint8_t *getNeighborAliveArray(Image *image, int row, int col, RGB color) {
  uint8_t *neighbor_alive = calloc(8, sizeof(uint8_t));
  int neighbor_row, neighbor_col;
  uint8_t neighbor_rgb_val;

  for (int i = -1; i < 2; i++) {
    for (int j = -1; j < 2; j++) {
      if (i || j) { // omit self
        neighbor_row = (row + i + image->rows) % image->rows;
        neighbor_col = (col + j + image->cols) % image->cols;
        neighbor_rgb_val =
            getRGBValue(image, neighbor_row, neighbor_col, color);
        updateNeighborAliveArray(neighbor_rgb_val, neighbor_alive);
      }
    }
  }

  return neighbor_alive;
}
