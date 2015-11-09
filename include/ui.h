#ifndef __ui__
#define __ui__

#include <curses.h>

#define WELCOME_ROW 1
#define WELCOME_COLUMN 2
#define WELCOME_INPUT_ROW 2
#define WELCOME_INPUT_COLUMN 21
#define CHAT_OUTPUT_ROW 1
#define CHAT_OUTPUT_COLUMN 2
#define CHAT_INPUT_ROW 1
#define CHAT_INPUT_COLUMN 2

int UI_MAXX, UI_MAXY; //screen dimensions

void UI_init(WINDOW *top, WINDOW *bottom);

void UI_set_window(WINDOW *window, int x, int y, int posy, int posx,
		int scroll, char sides, char ceiling);

void UI_set_chat(WINDOW *bottom, WINDOW *top, int x, int y);

void UI_redraw_window(WINDOW *window, char sides, char ceiling);

void UI_clear_window(WINDOW *window);

void UI_write_on_window(WINDOW *window, char *buffer, int row, int col);

void UI_read_from(WINDOW *window, char *buffer, int row, int col, size_t size);

/*


*/
#endif
