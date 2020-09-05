#include "ring-buffer.h"
#include "../libc/mem.h"

ringbuffer_t* rb_alloc(uint32_t buffer_size) {
  // TODO check return values from kmalloc
  uint32_t phys_addr;
  ringbuffer_t* rb = (ringbuffer_t*) kmalloc(sizeof(ringbuffer_t), 0, &phys_addr);

  rb->ptr = (char*) kmalloc(buffer_size, 0, &phys_addr);

  rb->size = buffer_size;
  rb->read_idx = 0;
  rb->write_idx = 0;

  return rb;
}

int rb_free(ringbuffer_t* ptr) {
  // TODO free
  return 0;
}

/**
   Reads a byte from the buffer.
   @returns number of bytes read.
 */
int rb_read_byte(ringbuffer_t*  rb, char* target) {
  if(rb->read_idx == rb->write_idx) {
    return 0;
  }
  
  *target = (rb->ptr)[rb->read_idx];
  // TODO avoid using a div?
  rb->read_idx = (rb->read_idx + 1) % rb->size;
  return 1;
}

/**
   Writes a byte to the buffer.
   @returns number of bytes written.
 */
int rb_write_byte(ringbuffer_t*  rb, char data) {
  rb->ptr[rb->write_idx] = data;
  rb->write_idx = (rb->write_idx + 1) & rb->size;
  return 1;
}
