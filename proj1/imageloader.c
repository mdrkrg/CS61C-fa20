/************************************************************************
**
** NAME:        imageloader.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**              Justin Yokota - Starter Code
**				YOUR NAME HERE
**
**
** DATE:        2020-08-15
**
**************************************************************************/

#include "imageloader.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Opens a .ppm P3 image file, and constructs an Image object.
// You may find the function fscanf useful.
// Make sure that you close the file with fclose before returning.
Image *readData(char *filename) {
  // YOUR CODE HERE
  FILE *fp = fopen(filename, "r");
  Image *image;

  if (fp == NULL) {
    fclose(fp);
    return NULL;
  }

  image = malloc(sizeof(Image));

  if (image == NULL) {
    free(image);
    return NULL;
  }

  fscanf(fp, "%*[^\n]\n"); // Skip a line
  fscanf(fp, "%d %d\n", &(image->cols), &(image->rows));
  fscanf(fp, "%*[^\n]\n"); // Skip a line

  image->image = calloc(image->rows, sizeof(Color **));
  if (image->image == NULL) {
    return NULL;
  }
  for (int i = 0; i < image->rows; i++) {
    image->image[i] = calloc(image->cols, sizeof(Color *));
    if (image->image[i] == NULL) {
      return NULL;
    }
    for (int j = 0; j < image->cols; j++) {
      image->image[i][j] = malloc(sizeof(Color));
      fscanf(fp, "%" SCNd8, &(image->image[i][j]->R));
      fscanf(fp, "%" SCNd8, &(image->image[i][j]->G));
      fscanf(fp, "%" SCNd8, &(image->image[i][j]->B));
    }
  }
  fclose(fp);
  return image;
}

// Given an image, prints to stdout (e.g. with printf) a .ppm P3 file with the
// image's data.
void writeData(Image *image) {
  // YOUR CODE HERE
  printf("P3\n");
  printf("%d %d\n", image->cols, image->rows);
  printf("255\n");
  for (int i = 0; i < image->rows; i++) {
    assert(image->cols > 0);
    for (int j = 0; j < image->cols - 1; j++) {
      printf("%3d ", image->image[i][j]->R);
      printf("%3d ", image->image[i][j]->G);
      printf("%3d ", image->image[i][j]->B);
      printf("  ");
    }
    printf("%3d ", image->image[i][image->cols - 1]->R);
    printf("%3d ", image->image[i][image->cols - 1]->G);
    printf("%3d", image->image[i][image->cols - 1]->B);
    printf("\n");
  }
}

// Frees an image
void freeImage(Image *image) {
  // YOUR CODE HERE
  for (int i = 0; i < image->rows; i++) {
    for (int j = 0; j < image->cols; j++) {
      free(image->image[i][j]);
    }
    free(image->image[i]);
  }
  free(image->image);
  free(image);
}
