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

// Ugly global: overrule attribute used by screen_char()
static int screen_char_attr = 0;

#if defined(FEAT_CONCEAL) || defined(PROTO)
/*
* Return TRUE if the cursor line in window "wp" may be concealed, according
 * to the 'concealcursor' option.
 */
    int
conceal_cursor_line(win_T *wp)
{
    int		c;

    if (*wp->w_p_cocu == NUL)
	return FALSE;
    if (get_real_state() & MODE_VISUAL)
	c = 'v';
    else if (State & MODE_INSERT)
	c = 'i';
    else if (State & MODE_NORMAL)
	c = 'n';
    else if (State & MODE_CMDLINE)
	c = 'c';
    else
	return FALSE;
    return vim_strchr(wp->w_p_cocu, c) != NULL;
}
