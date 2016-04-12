#include <stddef.h>
#include <setjmp.h>

#include "bytecode.h"
#include "kmalloc.h"
#include "kpanic.h"

#include "vm.h"

#define STACK_SIZE 0x0000FF00

static jmp_buf buf;

typedef virtual_machine_error int;

#define ERROR_UNKOWN 0xFF

typedef struct stack {
  void *top;
  void *bottom;
  size_t size;
} stack;

typedef struct virtual_machine {
  stack call_stack;
  stack left_stack;
  stack right_stack;
} virtual_machine;

// typedef struct program_header {
//   uint8_t system_module_dependency_count;
//   char **system_module_dependencies;
//   uint8_t library_dependency_count;
//   char **library_dependencies[2];
// } program_header;

typedef struct program {
//   program_header *header;
  size_t code_length;
  void *code;
} program;

const char *virtual_machine_error_message(virtual_machine_error error) {
  return "TODO (YOU LAZY BASTARD)";
}

void stack_push(stack *s, value val) {
  s->top += sizeof(value);
  *(s->top) = val;
}

value stack_pop(stack *s) {
  value *val = s->top;
  s->top -= sizeof(value);
  return *val;
}

value_pair stack_pop_pair(stack *s) {
  value_pair values;
  values.a = *(s->top);
  s->top -= sizeof(value);
  values.b = *(s->top);
  s->top -= sizeof(value);
  return values;
}

void stack_copy(stack *s, value count) {
  ptrdiff_t copy_size = sizeof(value) * count;

  for(value *origin = s->top - copy_size; origin < s->top; origin += sizeof(value)) {
    s->top += sizeof(value);
    *(s->top) = *origin;
  }
}

virtual_machine *create_virtual_machine() {
  virtual_machine *vm = kmalloc(sizeof(virtual_machine));
  if(vm == NULL) {
    return NULL;
  }

  // allocate and initialize the stacks
  for(stack *s = vm; s < vm + sizeof(virtual_machine); s += sizeof(stack)) {
    void *temp_ptr = kmalloc(STACK_SIZE);

    if(temp_ptr == NULL) {
      // cleanup previously malloc'd stacks
      for(s -= sizeof(stack); s >= vm; s -= sizeof(stack)) {
        kfree(s->top);
      }

      kfree(vm);
      return NULL;
    }

    s->top = temp_ptr;
    s->bottom = temp_ptr;
    s->size = STACK_SIZE;
  }

  return vm;
}

void cleanup_virtual_machine(virtual_machine *vm) {
  kfree(vm->call_stack->bottom);
  kfree(vm->left_stack->bottom);
  kfree(vm->right_stack->bottom);
  kfree(vm);
}


ptrdiff_t program_symbol_offset(program *p, char *symbol) {
}

void run_program(program *p) {
  virtual_machine *vm = create_virtual_machine();
  if(vm == NULL) {
  }

  virtual_machine_error error = setjmp(buf);
  if(!error) {
    run_virtual_machine(vm, p);
  } else {
    char *error_message = virtual_machine_error_message(error);
    kpanic("Encountered fatal virtual machine error: %s", error_message);
  }

  cleanup_virtual_machine(vm);
}

void run_virtual_machine(virtual_machine *vm, program *p) {
  value arg;
  value_pair values;

  opcode *code_ptr = p->code + program_symbol_offset(p, "entry");

  for(;;) {
    switch(*code_ptr) {
    case OP_CALL:
    case OP_RET:
    case OP_JMP:
    case OP_LPUSH:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      stack_push(&vm->left_stack, arg);
      code_ptr += sizeof(opcode) + sizeof(uint32_t);

    case OP_LCOPY:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      stack_copy(&vm->left_stack, arg);
      code_ptr += sizeof(opcode) + sizeof(uint32_t);

    case OP_LADD:
      value_pair values = stack_pop_pair(&vm->left_stack);
      stack_push(&vm->left_stack, values.a + values.b);
      code_ptr += sizeof(opcode);

    case OP_LSUB:
      value_pair values = stack_pop_pair(&vm->left_stack);
      stack_push(&vm->left_stack, values.a - values.b);
      code_ptr += sizeof(opcode);

    case OP_LJE:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->left_stack);

      if(values.a == values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_LJNE:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->left_stack);

      if(values.a != values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_LJL:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->left_stack);

      if(values.a < values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_LJG:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->left_stack);

      if(values.a > values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_LJLE:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->left_stack);

      if(values.a <= values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_LJGE:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->left_stack);

      if(values.a >= values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_RPUSH:
    case OP_RCOPY:
    case OP_RADD:
    case OP_RSUB:
    case OP_RJE:
    case OP_RJNE:
    case OP_RJL:
    case OP_RJG:
    case OP_RJLE:
    case OP_RJGE:
    default:
      kpanic("Unknown opcode read!!!");
    }
  }
}
