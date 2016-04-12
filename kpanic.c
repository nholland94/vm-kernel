#include "bios_print.h"

void kpanic(const char *message) {
  // TODO
  bios_print_fill(BG_RED, '@');
  bios_print_string(0, 0, FG_RED, message);
}
