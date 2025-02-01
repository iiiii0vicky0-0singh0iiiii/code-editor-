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
/*
 * Call screen_fill() with the columns adjusted for 'rightleft' if needed.
 * Return the new offset.
 */
    static int
screen_fill_end(
	win_T *wp,
	int	c1,
	int	c2,
	int	off,
	int	width,
	int	row,
	int	endrow,
	int	attr)
{
    int	    nn = off + width;

    if (nn > wp->w_width)
	nn = wp->w_width;
#ifdef FEAT_RIGHTLEFT
    if (wp->w_p_rl)
    {
	screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
		W_ENDCOL(wp) - nn, (int)W_ENDCOL(wp) - off,
		c1, c2, attr);
    }
    else
#endif
	screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
		wp->w_wincol + off, (int)wp->w_wincol + nn,
		c1, c2, attr);
    return nn;
}
/*
 * Clear lines near the end the window and mark the unused lines with "c1".
 * use "c2" as the filler character.
 * When "draw_margin" is TRUE then draw the sign, fold and number columns.
 */
    void
win_draw_end(
    win_T	*wp,
    int		c1,
    int		c2,
    int		draw_margin,
    int		row,
    int		endrow,
    hlf_T	hl)
{
    int		n = 0;
    int		attr = HL_ATTR(hl);
    int		wcr_attr = get_wcr_attr(wp);

    attr = hl_combine_attr(wcr_attr, attr);

    if (draw_margin)
    {
#ifdef FEAT_FOLDING
	int	fdc = compute_foldcolumn(wp, 0);

	if (fdc > 0)
