/************************************************************************
**
** NAME:        steganography.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**				Justin Yokota - Starter Code
**				YOUR NAME HERE
**
** DATE:        2020-08-23
**
**************************************************************************/

#include "imageloader.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// Determines what color the cell at the given row/col should be. This should
// not affect Image, and should allocate space for a new Color.
Color *evaluateOnePixel(Image *image, int row, int col) {
  // YOUR CODE HERE
  int index = row * image->cols + col;
  uint8_t b_value = image->image[index]->B;
  const uint8_t decoder = 0b00000001;
  uint8_t is_white = b_value & decoder;
  Color *one_pixel = malloc(sizeof(Color));
  if (one_pixel == NULL) {
    free(one_pixel);
    return NULL;
  }
  if (is_white) {
    one_pixel->R = 255;
    one_pixel->G = 255;
    one_pixel->B = 255;
  } else {
    one_pixel->R = 0;
    one_pixel->G = 0;
    one_pixel->B = 0;
  }
  return one_pixel;
}

// Given an image, creates a new image extracting the LSB of the B channel.
Image *steganography(Image *image) {
  // YOUR CODE HERE
  Image *new_image = malloc(sizeof(Image));
  uint32_t index;
  if (image == NULL) {
    free(image);
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
      new_image->image[index] = evaluateOnePixel(image, i, j);
      if (new_image->image[index] == NULL) {
        return NULL;
      }
    }
  }
  return new_image;
}

/*
Loads a file of ppm P3 format from a file, and prints to stdout (e.g. with
printf) a new image, where each pixel is black if the LSB of the B channel is 0,
and white if the LSB of the B channel is 1.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a file of ppm P3 format (not
necessarily with .ppm file extension). If the input is not correct, a malloc
fails, or any other error occurs, you should exit with code -1. Otherwise, you
should return from main with code 0. Make sure to free all memory before
returning!
*/
int main(int argc, char **argv) {
  // YOUR CODE HERE
  Image *read_image;
  Image *decode_image;
  if (argc != 2) {
    printf("Format: command path/to/file.ppm");
    exit(-1);
  }
  read_image = readData(argv[1]);
  if (read_image == NULL) {
    printf("Allocation failure occured, exit -1...");
    exit(-1);
  }
  decode_image = steganography(read_image);
  if (decode_image == NULL) {
    printf("Allocation failure occured, exit -1...");
    exit(-1);
  }
  writeData(decode_image);
  freeImage(read_image);
  freeImage(decode_image);
  return 0;
}
