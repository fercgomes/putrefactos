#include <stdint.h>

typedef struct {
  /* Pointer to start */
  char* ptr;

  /* Buffer size */
  uint32_t size;

  uint32_t read_idx;
  uint32_t write_idx;

} ringbuffer_t;

ringbuffer_t* rb_alloc(uint32_t buffer_size);
int rb_free(ringbuffer_t* ptr);

int rb_read_buffer(ringbuffer_t* rb, uint8_t* buffer, uint32_t bytes_to_read);

int rb_read_byte(ringbuffer_t* rb, char* target);
int rb_write_byte(ringbuffer_t* rb, char data);
