#include "../module_lib.h"

value get_input(vm_interface vm) {
  // TODO
  return 'a';
}

extern const char *register_module_name() {
  return "keyboard";
}

extern const char **register_interface_names() {
  return { "get_input" };
}

extern const void **register_interface_pointers() {
  return { &get_input };
}
