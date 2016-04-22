#ifndef __VM_H__
#define __VM_H__

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

extern void run_program(*program);

#endif
