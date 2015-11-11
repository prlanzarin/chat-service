#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/ui.h"

pthread_mutex_t scr_mutex = PTHREAD_MUTEX_INITIALIZER;


void clear_command_input(WINDOW *window, int row, int col)
{
	pthread_mutex_lock(&scr_mutex);
	mvwprintw(window, row, col, ">>                         ");
	//mvwprintw(window, chatInRow, 45, "                           ");
	wrefresh(window);
	pthread_mutex_unlock(&scr_mutex);
}

void UI_init(WINDOW *top, WINDOW *bottom) {
	initscr();		
	getmaxyx(stdscr, UI_MAXY, UI_MAXX);
	curs_set(2);
	UI_set_chat(top, bottom, UI_MAXY/2, UI_MAXX);

}

void UI_set_window(WINDOW *window, int x, int y, int posy, int posx,
		int scroll, char sides, char ceiling) {
	pthread_mutex_lock(&scr_mutex);
	window = newwin(y, x, posy, posx);
	box(window, sides, ceiling); 
	if(scroll) {
		scrollok(window, TRUE);
		wsetscrreg(window, 1, y-2);
	}
	wrefresh(window);
	pthread_mutex_unlock(&scr_mutex);
}

void UI_set_chat(WINDOW *top, WINDOW *bottom, int y, int x) {
	//UI_set_window(top, y, x, 0, 0, 1, '|', '+'); 
	//UI_set_window(bottom, y, x, y/2, 0, 1, '|', '+');
	pthread_mutex_lock(&scr_mutex);
	/* TODO SCROLL SET */

	top = newwin(y, x, 0, 0);
	bottom = newwin(y, x, y, 0);
	box(top, '|', '+'); 
	box(bottom, '|', '+'); 
	scrollok(top, TRUE);
	scrollok(bottom, TRUE);
	wsetscrreg(top, 1, y-2);
	wsetscrreg(bottom, 1, y-2);
	mvwprintw(bottom, CHAT_INPUT_ROW, CHAT_INPUT_COLUMN, "> ");
	wrefresh(top);
	wrefresh(bottom);
	pthread_mutex_unlock(&scr_mutex);
}

void UI_redraw_window(WINDOW *window, char sides, char ceiling) {
	pthread_mutex_lock(&scr_mutex);
	wclear(window);
	box(window, sides, ceiling);
	wrefresh(window);
	pthread_mutex_unlock(&scr_mutex);

}	

void UI_clear_window(WINDOW *window) {
	pthread_mutex_lock(&scr_mutex);
	wclear(window);
	wrefresh(window);
	pthread_mutex_unlock(&scr_mutex);

}

void UI_write_on_window(WINDOW *window, char *buffer, int row, int col) {
	pthread_mutex_lock(&scr_mutex);
	printf("WRITING %s on %d %d", buffer, row, col);
	mvwprintw(window, row, col, buffer); 
	wrefresh(window);
	pthread_mutex_unlock(&scr_mutex);
}

void UI_read_from(WINDOW *window, char *buffer, int row, int col, size_t size) {
	pthread_mutex_lock(&scr_mutex);
	mvwprintw(window, CHAT_INPUT_ROW, CHAT_INPUT_COLUMN, "> ");
	fflush(stdin);
	wgetnstr(window, buffer, size-2);
	fflush(stdin);
	UI_clear_window(window);
	pthread_mutex_unlock(&scr_mutex);
}
