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




/*
 * Check if the cursor line needs to be redrawn because of 'concealcursor'.
 * To be called after changing the state, "was_concealed" is the value of
 * "conceal_cursor_line()" before the change.
 * "
 */
    void
conceal_check_cursor_line(int was_concealed)
{
    if (curwin->w_p_cole <= 0 || conceal_cursor_line(curwin) == was_concealed)
	return;

    int wcol = curwin->w_wcol;

    need_cursor_line_redraw = TRUE;
    // Need to recompute cursor column, e.g., when starting Visual mode
    // without concealing.
    curs_columns(TRUE);

    // When concealing now w_wcol will be computed wrong, keep the previous
    // value, it will be updated in win_line().
    if (!was_concealed)
	curwin->w_wcol = wcol;
}
#endif

/*
 * Get 'wincolor' attribute for window "wp".  If not set and "wp" is a popup
 * window then get the "Pmenu" highlight attribute.
 */
    int
get_wcr_attr(win_T *wp)
{
    int wcr_attr = 0;

    if (*wp->w_p_wcr != NUL)
	wcr_attr = syn_name2attr(wp->w_p_wcr);
#ifdef FEAT_PROP_POPUP
    else if (WIN_IS_POPUP(wp))
    {
	if (wp->w_popup_flags & POPF_INFO)
	    wcr_attr = HL_ATTR(HLF_PSI);    // PmenuSel
	else
	    wcr_attr = HL_ATTR(HLF_PNI);    // Pmenu
    }
#endif
    return wcr_attr;
}
