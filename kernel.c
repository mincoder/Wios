/* Surely you will remove the processor conditionals and this comment
   appropriately depending on whether or not you use C++. */
#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif
#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "This kernel requires a cross-compiler"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "Kernel needs to be compiled with a ix86-elf compiler"
#endif

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
size_t cmdrow = 0;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

size_t get_end(size_t y) {
	for (size_t x = VGA_WIDTH; x > 0; x--) {
		const size_t index = y * VGA_WIDTH + x;
		if(!(terminal_buffer[index] == vga_entry(' ', terminal_color))) {
			return VGA_WIDTH-x;
		}
	}
	return 0;
}	

void terminal_putchar(char c,bool user) {
	if(c=='\n') {
		if(!user) {
			terminal_putentryat(' ',terminal_color,terminal_column, terminal_row);
			terminal_row++;
			terminal_putentryat(' ',terminal_color,terminal_column, terminal_row);
			terminal_column = 0;
			cmdrow=terminal_row;
		}
	} else if(c=='\b') {
		if(--terminal_column < 0) {
			/*if((terminal_row > 0) && (terminal_row>cmdrow)) {
				terminal_putentryat(' ',terminal_color,terminal_column, terminal_row);
				terminal_row--;
				terminal_putentryat(' ',terminal_color,terminal_column, terminal_row);
				terminal_column = get_end(--terminal_row);
			}*/		
		} else {
			terminal_putentryat(' ',terminal_color,terminal_column, terminal_row);
			terminal_putentryat(' ',terminal_color,terminal_column+1, terminal_row);

			
		}
	} else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			/*terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_row = 0;
			*/
		} else {
			terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		}
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i],false);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

/*
 * <Keyboard library>
 */

char keyCode[] =
{'1','2','3','4','5','6','7','8','9','0',
'-','=','\b','\t','q','w','e','r','t','y',
'u','i','o','p','å',']','\n','ĉ','a','s',
'd','f','g','h','j','k','l','ö','ä','ŝ','\\',
'z','z','x','c','v','b','n','m','.','/'};

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

char getScancode() {
    char c=0;
    do {
        if(inb(0x60)!=c) {
            c=inb(0x60);
            if(c>0)
                return c;
        }
    } while(1);
}
 
/*char getchar() {
    return scancode[getScancode()+1];
}*/

/*
 * </Keyboard library>
 */

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kernel_main(void) {
	/* Initialize terminal interface */
	terminal_initialize();

	/* Hello world and boot display */
	terminal_writestring("Booting WIOS v0.01\n");
	terminal_writestring("\n");
	terminal_writestring("Loading keyboard io\n");
	
	char c=0;
	int i = 0;
	do {
		if(i==0) {
			terminal_putentryat('_',terminal_color,terminal_column,terminal_row);
		} else if(i==1) {
			terminal_putentryat(' ',terminal_color,terminal_column,terminal_row);
		}
		if(inb(0x60)!=c) {
        	c=inb(0x60);
		if(c>0) {
			if(keyCode[c-2]=='\n') {
				
			}
			terminal_putchar(keyCode[c-2],true);
		}
        }
    } while(1);
}
