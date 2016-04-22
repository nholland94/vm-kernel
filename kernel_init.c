#include "bytecode.h"
#include "kmalloc.h"
#include "vm.h"

static uint16_t[] code = {
  OP_LPUSH,
  0, 2,
  OP_LPUSH,
  0, 4,
  OP_ADD
};

void kernel_init() {
  kmalloc_init();

  program p = {
    code_length = sizeof(code);
    code = &code;
  };

  run_program(&p);
}
