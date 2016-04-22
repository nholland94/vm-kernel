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
  s->top -= sizeof(value_pair);
  return *(value_pair *)(s->top);
}

void stack_copy(stack *s, value count) {
  ptrdiff_t copy_size = sizeof(value) * count;

  for(value *origin = s->top - copy_size; origin < s->top; origin += sizeof(value)) {
    s->top += sizeof(value);
    *(s->top) = *origin;
  }
}

int stack_count(stack *s) {
  return (s->top - s->bottom) / sizeof(value);
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

  opcode *code_ptr = p->code + program_symbol_offset(p, program_symbol_id(p, "entry"));

  // TODO check bounds
  for(;;) {
    switch(*code_ptr) {
    case OP_CALL:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode));
      stack_push(&vm->call_stack, code_ptr - p->code);
      code_ptr = p->code + program_symbol_offset(p, arg);

    case OP_RET:
      code_ptr = p->code + stack_pop(&vm->call_stack);

    case OP_JMP:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      code_ptr = p->code + arg;
      // TODO check bounds

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
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      stack_push(&vm->right_stack, arg);
      code_ptr += sizeof(opcode) + sizeof(uint32_t);

    case OP_RCOPY:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      stack_copy(&vm->right_stack, arg);
      code_ptr += sizeof(opcode) + sizeof(uint32_t);

    case OP_RADD:
      value_pair values = stack_pop_pair(&vm->right_stack);
      stack_push(&vm->right_stack, values.a + values.b);
      code_ptr += sizeof(opcode);

    case OP_RSUB:
      value_pair values = stack_pop_pair(&vm->right_stack);
      stack_push(&vm->right_stack, values.a - values.b);
      code_ptr += sizeof(opcode);

    case OP_RJE:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->right_stack);

      if(values.a == values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_RJNE:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->right_stack);

      if(values.a != values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }


    case OP_RJL:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->right_stack);

      if(values.a < values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_RJG:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->right_stack);

      if(values.a > values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_RJLE:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->right_stack);

      if(values.a <= values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    case OP_RJGE:
      arg = *(uint32_t *)(code_ptr + sizeof(opcode)); 
      value_pair values = stack_pop_pair(&vm->right_stack);

      if(values.a >= values.b) {
        code_ptr = p->code + arg;
      } else {
        code_ptr += sizeof(opcode) + sizeof(uint32_t);
      }

    default:
      kpanic("Unknown opcode read!!!");
    }
  }
}
