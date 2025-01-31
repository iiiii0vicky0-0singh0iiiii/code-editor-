#include "code_editor.h"

/*
 * The attributes that are actually active for writing to the screen.
 */
static int screen_attr = 0;

// Function declarations
static void screen_char_2(unsigned int off, int row, int col);
static int screenclear2(int doclear);
static void lineclear(unsigned int off, int width, int attr);
static void lineinvalid(unsigned int off, int width);
static int win_do_lines(win_T* wp, int row, int line_count, int mayclear, int del, int clear_attr);
static void win_rest_invalid(win_T* wp);
static void msg_pos_mode();
static void recording_mode(int attr);
