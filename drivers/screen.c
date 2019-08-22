#include "screen.h"
#include "../cpu/ports.h"

/* private functions */
int get_cursor_offset();
void set_cursor_offset(int offset);
int print_char(char c, int col, int row, char attr);
int get_offset(int col, int row);
int get_offset_row(int offset);
int get_offset_col(int offset);

/* public kernel API functions */

/*
    Print a message on specified location.
    If col, row, are negative, will use current offset.
 */
void kprint_at(char *message, int col, int row) {
    /* set cursor if col/row are negative */
    int offset;

    if(col >= 0 && row >= 0)
        offset = get_offset(col, row);
    else {
        offset = get_cursor_offset();
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }

    /* loop through message and print */
    int i = 0;
    while (message[i] != 0) {
        offset = print_char(message[i++], col, row, WHITE_ON_BLACK);
        /* compute row/col for next iteration */
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
}

void kprint(char *message) {
    kprint_at(message, -1, -1);
}

void kprint_backspace() {
    int offset = get_cursor_offset() - 2;
    int row = get_offset_row(offset);
    int col = get_offset_col(offset);
    print_char(0x08, col, row, WHITE_ON_BLACK);
}

/* private kernel functions */

/*
    Innermost print function. Directly accesses video memory.

    If col and row are negative, print at current cursor location.
    If attr is zero, use white on black as default.
    Returns offset of next char.
    Sets video cursor to returned offset.
*/
int print_char(char c, int col, int row, char attr) {
    unsigned char *vidmem = (unsigned char*) VIDEO_ADDRESS;
    /* sets default color attr */
    if (!attr) attr = WHITE_ON_BLACK;  

    /* error control: print red 'E' if coordinates aren't right */
    if(col >= MAX_COLS || row >= MAX_ROWS) {
        vidmem[2 * (MAX_COLS) * (MAX_ROWS) - 2] = 'E';
        vidmem[2 * (MAX_COLS) * (MAX_ROWS) - 1] = RED_ON_WHTIE;
        return get_offset(col, row);
    }

    int offset;
    if(col >= 0 && row >= 0) offset = get_offset(col, row);
    else offset = get_cursor_offset();

    if(c == '\n') {
        row = get_offset_row(offset);
        offset = get_offset(0, row + 1);
    }
    else {
        vidmem[offset] = c;
        vidmem[offset + 1] = attr;
        offset += 2;
    }

    /* check if offset is over screen size and scroll */
    if(offset >= MAX_ROWS * MAX_COLS * 2) {
        int i;
        for(i = 1; i < MAX_ROWS; i++) {
            memory_copy(get_offset(0, i) + VIDEO_ADDRESS,
                        get_offset(0, i - 1) + VIDEO_ADDRESS,
                        MAX_COLS * 2);
        }

        /* blank last line */
        char* last_line = get_offset(0, MAX_ROWS - 1) + VIDEO_ADDRESS;
        for(i = 0; i < MAX_COLS * 2; i++) last_line[i] = 0;

        offset -= 2 * MAX_COLS;
    }

    set_cursor_offset(offset);
    return offset;
}

int get_cursor_offset() {
    /*
        use vga ports to get current cursor position
        1. ask for high byte for cursor offset (data 14)
        2. ask for low byte (data 15)
    */

   port_byte_out(REG_SCREEN_CONTROL, 14);
   int offset = port_byte_in(REG_SCREEN_DATA) << 8; // high byte << 8
   port_byte_out(REG_SCREEN_CONTROL, 15);
   offset += port_byte_in(REG_SCREEN_DATA);
   return offset * 2;
}

/* TODO: what are 14, 15 ? */
void set_cursor_offset(int offset) {
    offset /= 2;
    port_byte_out(REG_SCREEN_CONTROL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset >> 8));
    port_byte_out(REG_SCREEN_CONTROL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset & 0xff));
}

void clear_screen() {
    int screen_size = MAX_COLS * MAX_ROWS;
    int i;
    char *screen = VIDEO_ADDRESS;

    for(i = 0; i < screen_size; i++) {
        screen[i * 2] = ' ';
        screen[i * 2 + 1] = WHITE_ON_BLACK;
    }
    set_cursor_offset(get_offset(0, 0));
}

int get_offset(int col, int row) { return 2 * (row * MAX_COLS + col); }
int get_offset_row(int offset) { return offset / (2 * MAX_COLS); }
int get_offset_col(int offset) { return (offset - (get_offset_row(offset) * 2 * MAX_COLS)) / 2; }