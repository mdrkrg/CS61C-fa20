#include "imageloader.h"
#include <inttypes.h>
#include <stdbool.h>

// TODO: Update all uint8_t of color bit depth to new type
const uint8_t BIT_DEPTH = 8;
typedef uint8_t bitdepth_t;
typedef enum { R, G, B } RGB;

Color *getColor(Image *image, int row, int col);

uint8_t getRGBValue(Image *image, int row, int col, RGB color);

// Returns next RGB value
uint8_t getNextRGBValue(uint8_t rgb_val, uint8_t neighbor_alive[8],
                        uint32_t rule);

// Helper function for getNextRGBValue
uint8_t *getNeighborAliveArray(Image *image, int row, int col, RGB color);

// Helper function for getNeighborAliveArray
void updateNeighborAliveArray(uint8_t neighbor_rgb_val,
                              uint8_t neighbor_alive[8]);
