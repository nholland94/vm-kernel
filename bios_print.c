#include <stddef.h>

#include "bios_print.h"

#define BIOS_VIDEO_MEMORY_ADDRESS 0X000B8000

#define BIOS_VIDEO_MEMORY_WIDTH  80
#define BIOS_VIDEO_MEMORY_HEIGHT 25

void bios_print_char(int x, int y, uint16_t color, char character) {
  uint16_t cell = (color << 8) | (uint16_t)character;
  int index = (y * BIOS_VIDEO_MEMORY_WIDTH) + x;
  ptrdiff_t offset = index * sizeof(uint16_t);
  *(volatile uint16_t*)(BIOS_VIDEO_MEMORY_ADDRESS + offset) = cell;
}

void bios_print_fill(uint16_t color, char character) {
  for(int y = 0; y < BIOS_VIDEO_MEMORY_HEIGHT; y++) {
    for(int x = 0; x < BIOS_VIDEO_MEMORY_WIDTH; x++) {
      bios_print_char(x, y, color, character);
    }
  }
}

void bios_print_string(int x, int y, uint16_t color, const char *str) {
  while(*str != NULL) {
    bios_print_char(x, y, color, *str);

    x++;
    if(x == BIOS_VIDEO_MEMORY_WIDTH) {
      x = 0;
      y++;
      if(y == BIOS_VIDEO_MEMORY_HEIGHT) {
        bios_print_char(BIOS_VIDEO_MEMORY_WIDTH - 1, BIOS_VIDEO_MEMORY_HEIGHT - 1, BG_RED, '~');
        return;
      }
    }

    str += sizeof(char);
  }
}
