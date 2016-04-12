#include <stdint.h>

#include "kpanic.h"

extern "C" void _K_HEAP_START_;
extern "C" void _K_HEAP_END_;

#define _K_HEAP_SIZE_ (_K_HEAP_END_ - _K_HEAP_START_)

struct free_zone {
  void *base_address;
  struct free_zone *next_free_zone;
  size_t zone_size;
};

struct free_zone *free_zone_top;
struct free_zone *free_zone_bottom;

void kmalloc_init() {
  *(size_t *)(_K_HEAP_START_) = sizeof(struct free_zone);
  struct free_zone *initial_free_zone = (struct free_zone *)(_K_HEAP_START_ + sizeof(size_t));

  initial_free_zone->base_address = initial_free_zone + sizeof(struct free_zone);
  initial_free_zone->next_free_zone = NULL;
  initial_free_zone->zone_size = _K_HEAP_SIZE_ - sizeof(struct free_zone);

  free_zone_top = initial_free_zone;
  free_zone_bottom = initial_free_zone;
}

void *kmalloc(size_t size) {
  void *k_heap_base;
  size_t required_size = size + sizeof(size_t);
  struct free_zone *zone = free_zone_top;
  struct free_zone *parent_zone = NULL;

  while(zone != NULL && !(zone->zone_size >= required_size)) {
    parent_zone = zone;
    zone = zone->next_free_zone;
  }

  if(zone == NULL) {
    return NULL;
  }

  if(zone->zone_size == required_size) {
    struct free_zone *old_free_zone_top = zone;
    k_heap_base = old_free_zone_top->base_address;

    if(parent_zone != NULL) {
      parent_zone->next_free_zone = zone->next_free_zone;
    } else {
      free_zone_top = zone->next_free_zone;
    }

    kfree(old_free_zone_top);
  } else {
    k_heap_base = zone->base_address;
    zone->base_address += required_size;
    zone->zone_size -= required_size;
  }

  *(size_t *)k_heap_base = size;
  return (void *)(k_heap_base + sizeof(size_t));
}

// Does not validate whether or not a pointer being free'd is valid... be careful
void kfree(void *ptr) {
  size_t *size_ptr = ptr - sizeof(size_t);
  size_t size = *size_ptr;
  *size_ptr = 0;

  // Does not attempt to combine contiguous free_zones
  struct free_zone *new_free_zone = (struct free_zone *)kmalloc(sizeof(struct free_zone));
  size_t zone_size = size + 1;
  if(new_free_zone == NULL) {
    if(size < sizeof(struct free_zone)) {
      kpanic("INTERNAL HEAP ERROR: cannot allocate free_zone in heap");
    } else {
      *size_ptr = sizeof(struct free_zone);
      new_free_zone = ptr;
      ptr += sizeof(struct free_zone);
      zone_size -= sizeof(struct free_zone) + sizeof(size_t);
    }
  }

  new_free_zone->base_address = ptr;
  new_free_zone->next_free_zone = NULL;
  new_free_zone->zone_size = zone_size;

  if(free_zone_bottom != NULL) {
    free_zone_bottom->next_free_zone = new_free_zone;
  }

  free_zone_bottom = new_free_zone;
}
