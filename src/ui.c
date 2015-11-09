#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../include/ui.h"

pthread_mutex_t scr_mutex = PTHREAD_MUTEX_INITIALIZER;

void UI_init(WINDOW *top, WINDOW *bottom) {
	initscr();		
	getmaxyx(stdscr, UI_MAXY, UI_MAXX);
	UI_set_chat(top, bottom, UI_MAXY/2, UI_MAXX);

}

void UI_set_window(WINDOW *window, int x, int y, int posy, int posx,
		int scroll, char sides, char ceiling) {
	pthread_mutex_lock(&scr_mutex);
	window = newwin(y, x, posy, posx);
	scrollok(window, scroll);
	box(window, sides, ceiling); 
	if(scroll) 
		wsetscrreg(window, 1, y-2);

	wrefresh(window);
	pthread_mutex_unlock(&scr_mutex);
}

void UI_set_chat(WINDOW *bottom, WINDOW *top, int x, int y) {
	UI_set_window(top, y/2, x, 0, 0, 1, '|', '+'); 
	UI_set_window(bottom, y/2, x, y/2, 0, 1, '|', '+');
	pthread_mutex_lock(&scr_mutex);
	/* TODO SCROLL SET */
	wsetscrreg(top, 1, UI_MAXY/2-2);
	wsetscrreg(bottom, 1, UI_MAXY/2-2);
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
	mvwprintw(window, row, col, buffer); 
	pthread_mutex_unlock(&scr_mutex);
}

void UI_read_from(WINDOW *window, char *buffer, int row, int col, size_t size) {
	fflush(stdin);	
	pthread_mutex_lock(&scr_mutex);
	mvwgetnstr(window, row, col, buffer, size);
	pthread_mutex_unlock(&scr_mutex);
	fflush(stdin);
}

