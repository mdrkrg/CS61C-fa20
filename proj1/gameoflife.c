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

#include "imageloader.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Helper function to return the &Color of given rol, col
Color *getColor(Image *image, int row, int col) {
  int index = row * image->cols + col;
  return image->image[index];
}

uint8_t getRValue(Image *image, int row, int col) {
  return getColor(image, row, col)->R;
}

bool isLive(Image *image, int row, int col) {
  return (getRValue(image, row, col)) ? 1 : 0;
}

uint8_t getNeighborLive(Image *image, int row, int col) {
  uint8_t neighbor_live = 0;
  int neighbor_row, neighbor_col;
  for (int i = -1; i < 2; i++) {
    for (int j = -1; j < 2; j++) {
      neighbor_row = (row + i + image->rows) % image->rows;
      neighbor_col = (col + j + image->cols) % image->cols;
      if ((i || j) && isLive(image, neighbor_row, neighbor_col)) {
        neighbor_live++;
      }
    }
  }
  return neighbor_live;
}

bool isLiveNext(bool is_live, uint8_t neighbor_live, uint32_t rule) {
  uint32_t pos = 1; // 0b 0000 0000 0000 0000 0000 0000 0000 0001
  int shift = neighbor_live + is_live * 9;
  pos = pos << shift;
  return (pos & rule) >> shift;
}

Color *getNewCell(bool is_live_next) {
  Color *new_cell = malloc(sizeof(Color));
  if (new_cell == NULL) {
    free(new_cell);
    return NULL;
  }
  if (is_live_next) {
    // White
    new_cell->R = 255;
    new_cell->G = 255;
    new_cell->B = 255;
  } else {
    // Black
    new_cell->R = 0;
    new_cell->G = 0;
    new_cell->B = 0;
  }
  return new_cell;
}

// Determines what color the cell at the given row/col should be. This function
// allocates space for a new Color. Note that you will need to read the eight
// neighbors of the cell in question. The grid "wraps", so we treat the top row
// as adjacent to the bottom row and the left column as adjacent to the right
// column.
//
//
//
// Bitwise operation: uint16t 0b0000000000000001 leftshift by n + is_live * 9
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule) {
  // YOUR CODE HERE
  bool is_live = isLive(image, row, col);
  uint8_t neighbor_live = getNeighborLive(image, row, col);
  bool is_live_next = isLiveNext(is_live, neighbor_live, rule);
  return getNewCell(is_live_next);
}

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
