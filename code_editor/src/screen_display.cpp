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
 // draw the fold column
	    n = screen_fill_end(wp, ' ', ' ', n, fdc,
		      row, endrow, hl_combine_attr(wcr_attr, HL_ATTR(HLF_FC)));
#endif
#ifdef FEAT_SIGNS
	if (signcolumn_on(wp))
  // draw the sign column
	    n = screen_fill_end(wp, ' ', ' ', n, 2,
		      row, endrow, hl_combine_attr(wcr_attr, HL_ATTR(HLF_SC)));
#endif
	if ((wp->w_p_nu || wp->w_p_rnu)
				  && vim_strchr(p_cpo, CPO_NUMCOL) == NULL)
		  // draw the number column
	    n = screen_fill_end(wp, ' ', ' ', n, number_width(wp) + 1,
		       row, endrow, hl_combine_attr(wcr_attr, HL_ATTR(HLF_N)));
    }
#ifdef FEAT_RIGHTLEFT
    if (wp->w_p_rl)
    {
	screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
		wp->w_wincol, W_ENDCOL(wp) - 1 - n,
		c2, c2, attr);
	screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
		W_ENDCOL(wp) - 1 - n, W_ENDCOL(wp) - n,
		c1, c2, attr);
    }
    else
#endif
    {
	screen_fill(W_WINROW(wp) + row, W_WINROW(wp) + endrow,
		wp->w_wincol + n, (int)W_ENDCOL(wp),
		c1, c2, attr);
    }

    set_empty_rows(wp, row);
}

#if defined(FEAT_FOLDING) || defined(PROTO)
/*
 * Compute the width of the foldcolumn.  Based on 'foldcolumn' and how much
 * space is available for window "wp", minus "col".
 */
    int
compute_foldcolumn(win_T *wp, int col)
{
    int wmw = wp == curwin && p_wmw == 0 ? 1 : p_wmw;
    int n = wp->w_width - (col + wmw);

    return MIN(wp->w_p_fdc, n);
}

/*
 * Fill the foldcolumn at "p" for window "wp".
 * Only to be called when 'foldcolumn' > 0.
 * Returns the number of bytes stored in 'p'. When non-multibyte characters are
 * used for the fold column markers, this is equal to 'fdc' setting. Otherwise,
 * this will be greater than 'fdc'.
 */
    size_t
fill_foldcolumn(
    char_u	*p,
    win_T	*wp,
    int		closed,		// TRUE of FALSE
    linenr_T	lnum)		// current line number
{
    int		i = 0;
    int		level;
    int		first_level;
    int		empty;
    int		fdc = compute_foldcolumn(wp, 0);
    size_t	byte_counter = 0;
    int		symbol = 0;
    int		len = 0;
    int		n;

    // Init to all spaces.
    vim_memset(p, ' ', MAX_MCO * fdc + 1);

    level = win_foldinfo.fi_level;
    empty = (fdc == 1) ? 0 : 1;
 // If the column is too narrow, we start at the lowest level that
    // fits and use numbers to indicate the depth.
    first_level = level - fdc - closed + 1 + empty;
    if (first_level < 1)
	first_level = 1;

    n = MIN(fdc, level);		// evaluate this once
    for (i = 0; i < n; i++)
    {
	if (win_foldinfo.fi_lnum == lnum
		&& first_level + i >= win_foldinfo.fi_low_level)
	    symbol = wp->w_fill_chars.foldopen;
	else if (first_level == 1)
	    symbol = wp->w_fill_chars.foldsep;
	else if (first_level + i <= 9)
	    symbol = '0' + first_level + i;
	else
	    symbol = '>';

	len = utf_char2bytes(symbol, &p[byte_counter]);
	byte_counter += len;
	if (first_level + i >= level)
	{
	    i++;
	    break;
	}
    }

    if (closed)
    {
	if (symbol != 0)
	{
	    // rollback length and the character
	    byte_counter -= len;
	    if (len > 1)
		    // for a multibyte character, erase all the bytes
		vim_memset(p + byte_counter, ' ', len);
	}
	symbol = wp->w_fill_chars.foldclosed;
	len = utf_char2bytes(symbol, &p[byte_counter]);
	byte_counter += len;
    }

    return MAX(byte_counter + (fdc - i), (size_t)fdc);
}
#endif // FEAT_FOLDING

/*
 * Return if the composing characters at "off_from" and "off_to" differ.
 * Only to be used when ScreenLinesUC[off_from] != 0.
 */
    static int
comp_char_differs(int off_from, int off_to)
{
    int	    i;

    for (i = 0; i < Screen_mco; ++i)
    {
	if (ScreenLinesC[i][off_from] != ScreenLinesC[i][off_to])
	    return TRUE;
	if (ScreenLinesC[i][off_from] == 0)
	    break;
    }
    return FALSE;
}
/*
 * Check whether the given character needs redrawing:
 * - the (first byte of the) character is different
 * - the attributes are different
 * - the character is multi-byte and the next byte is different
 * - the character is two cells wide and the second cell differs.
 */
    static int
char_needs_redraw(int off_from, int off_to, int cols)
{
    if (cols > 0
	    && ((ScreenLines[off_from] != ScreenLines[off_to]
		    || ScreenAttrs[off_from] != ScreenAttrs[off_to])
		|| (enc_dbcs != 0
		    && MB_BYTE2LEN(ScreenLines[off_from]) > 1
		    && (enc_dbcs == DBCS_JPNU && ScreenLines[off_from] == 0x8e
			? ScreenLines2[off_from] != ScreenLines2[off_to]
			: (cols > 1 && ScreenLines[off_from + 1]
						 != ScreenLines[off_to + 1])))
		|| (enc_utf8
		    && (ScreenLinesUC[off_from] != ScreenLinesUC[off_to]
			|| (ScreenLinesUC[off_from] != 0
			    && comp_char_differs(off_from, off_to))
			|| ((*mb_off2cells)(off_from, off_from + cols) > 1
			    && ScreenLines[off_from + 1]
						!= ScreenLines[off_to + 1])))))
	return TRUE;
    return FALSE;
}
}

#if defined(FEAT_TERMINAL) || defined(PROTO)
/*
 * Return the index in ScreenLines[] for the current screen line.
 */
    int
screen_get_current_line_off(void)
{
    return (int)(current_ScreenLine - ScreenLines);
}
#endif

#ifdef FEAT_PROP_POPUP
/*
 * Return TRUE if this position has a higher level popup or this cell is
 * transparent in the current popup.
 */
    static int
blocked_by_popup(int row, int col)
{
    int off;

    if (!popup_visible)
	return FALSE;
    off = row * screen_Columns + col;
    return popup_mask[off] > screen_zindex || popup_transparent[off];
}
#endif

