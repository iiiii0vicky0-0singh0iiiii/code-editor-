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
/*
 * Reset the highlighting.  Used before clearing the screen.
 */
    void
reset_screen_attr(void)
{
#ifdef FEAT_GUI
    if (gui.in_use)
	// Use a code that will reset gui.highlight_mask in
	// gui_stop_highlight().
	screen_attr = HL_ALL + 1;
    else
#endif
	// Use attributes that is very unlikely to appear in text.
	screen_attr = HL_BOLD | HL_UNDERLINE | HL_INVERSE | HL_STRIKETHROUGH;
}

/*
 * Return TRUE if the character at "row" / "col" is under the popup menu and it
 * will be redrawn soon or it is under another popup.
 */
    static int
skip_for_popup(int row, int col)
{
    // Popup windows with zindex higher than POPUPMENU_ZINDEX go on top.
    if (pum_under_menu(row, col, TRUE)
#ifdef FEAT_PROP_POPUP
	    && screen_zindex <= POPUPMENU_ZINDEX
#endif
	    )
	return TRUE;
#ifdef FEAT_PROP_POPUP
    if (blocked_by_popup(row, col))
	return TRUE;
#endif
    return FALSE;
}

/*
 * Move one "cooked" screen line to the screen, but only the characters that
 * have actually changed.  Handle insert/delete character.
 * "coloff" gives the first column on the screen for this line.
 * "endcol" gives the columns where valid characters are.
 * "clear_width" is the width of the window.  It's > 0 if the rest of the line
 * needs to be cleared, negative otherwise.
 * "flags" can have bits:
 * SLF_POPUP	    popup window
 * SLF_RIGHTLEFT    rightleft window:
 *    When TRUE and "clear_width" > 0, clear columns 0 to "endcol"
 *    When FALSE and "clear_width" > 0, clear columns "endcol" to "clear_width"
 * SLF_INC_VCOL:
 *    When FALSE, use "last_vcol" for ScreenCols[] of the columns to clear.
 *    When TRUE, use an increasing sequence starting from "last_vcol + 1" for
 *    ScreenCols[] of the columns to clear.
 */
    void
screen_line(
	win_T	*wp,
	int	row,
	int	coloff,
	int	endcol,
	int	clear_width,
	colnr_T	last_vcol,
	int	flags UNUSED)
{
    unsigned	    off_from;
    unsigned	    off_to;
    unsigned	    max_off_from;
    unsigned	    max_off_to;
    int		    col = 0;
    int		    hl;
    int		    force = FALSE;	// force update rest of the line
    int		    redraw_this		// bool: does character need redraw?
#ifdef FEAT_GUI
				= TRUE	// For GUI when while-loop empty
#endif
				;
    int		    redraw_next;	// redraw_this for next character
#ifdef FEAT_GUI_MSWIN
    int		    changed_this;	// TRUE if character changed
    int		    changed_next;	// TRUE if next character changed
#endif
    int		    clear_next = FALSE;
    int		    char_cells;		// 1: normal char
					// 2: occupies two display cells

    // Check for illegal row and col, just in case.
    if (row >= Rows)
	row = Rows - 1;
    if (endcol > Columns)
	endcol = Columns;

# ifdef FEAT_CLIPBOARD
    clip_may_clear_selection(row, row);
# endif

    off_from = (unsigned)(current_ScreenLine - ScreenLines);
    off_to = LineOffset[row] + coloff;
    max_off_from = off_from + screen_Columns;
    max_off_to = LineOffset[row] + screen_Columns;

#ifdef FEAT_RIGHTLEFT
    if (flags & SLF_RIGHTLEFT)
    {
	// Clear rest first, because it's left of the text.
	if (clear_width > 0)
	{
	    int clear_start = col;

	    while (col <= endcol && ScreenLines[off_to] == ' '
		    && ScreenAttrs[off_to] == 0
				  && (!enc_utf8 || ScreenLinesUC[off_to] == 0))
	    {
		++off_to;
		++col;
	    }
	    if (col <= endcol)
		screen_fill(row, row + 1, col + coloff,
					    endcol + coloff + 1, ' ', ' ', 0);

	    for (int i = endcol; i >= clear_start; i--)
		ScreenCols[off_to + (i - col)] =
		    (flags & SLF_INC_VCOL) ? ++last_vcol : last_vcol;
	}
	col = endcol + 1;
	off_to = LineOffset[row] + col + coloff;
	off_from += col;
	endcol = (clear_width > 0 ? clear_width : -clear_width);
    }
#endif // FEAT_RIGHTLEFT

#ifdef FEAT_PROP_POPUP
    if (flags & SLF_POPUP)
    {
    // Clear the whole line, it's under a popup.
    screen_fill(row, row + 1, 0, endcol + coloff + 1, ' ', ' ', 0);
    for (int i = 0; i <= endcol; i++)
        ScreenCols[off_to + i] = (flags & SLF_INC_VCOL)
        ? ++last_vcol : last_vcol;
    col = endcol + 1;
    off_to = LineOffset[row] + col + coloff;
    off_from += col;
    endcol = (clear_width > 0 ? clear_width : -clear_width);
    }
    redraw_next = char_needs_redraw(off_from, off_to, endcol - col);
#ifdef FEAT_GUI_MSWIN
    changed_next = redraw_next;
#endif

    while (col < endcol)
    {
	if (has_mbyte && (col + 1 < endcol))
	    char_cells = (*mb_off2cells)(off_from, max_off_from);
	else
	    char_cells = 1;

	redraw_this = redraw_next;
	redraw_next = force || char_needs_redraw(off_from + char_cells,
			      off_to + char_cells, endcol - col - char_cells);

#ifdef FEAT_GUI
# ifdef FEAT_GUI_MSWIN
	changed_this = changed_next;
	changed_next = redraw_next;
# endif
	// If the next character was bold, then redraw the current character to
	// remove any pixels that might have spilt over into us.  This only
	// happens in the GUI.
	// With MS-Windows antialiasing may also cause pixels to spill over
	// from a previous character, no matter attributes, always redraw if a
	// character changed.
	if (redraw_next && gui.in_use)
	{
# ifndef FEAT_GUI_MSWIN
	    hl = ScreenAttrs[off_to + char_cells];
	    if (hl > HL_ALL)
		hl = syn_attr2attr(hl);
	    if (hl & HL_BOLD)
# endif
		redraw_this = TRUE;
	}
#endif
// Do not redraw if under the popup Menu
	if (redraw_this && skip_for_popup(row, col + coloff))
	    redraw_this = FALSE;

	if (redraw_this)
	{
	    /*
	     * Special handling when 'xs' termcap flag set (hpterm):
	     * Attributes for characters are stored at the position where the
	     * cursor is when writing the highlighting code.  The
	     * start-highlighting code must be written with the cursor on the
	     * first highlighted character.  The stop-highlighting code must
	     * be written with the cursor just after the last highlighted
	     * character.
	     * Overwriting a character doesn't remove its highlighting.  Need
	     * to clear the rest of the line, and force redrawing it
	     * completely.
	     */
	    if (       p_wiv
		    && !force
#ifdef FEAT_GUI
		    && !gui.in_use
#endif
		    && ScreenAttrs[off_to] != 0
		    && ScreenAttrs[off_from] != ScreenAttrs[off_to])
	    {
/*
		 * Need to remove highlighting attributes here.
		 */
		windgoto(row, col + coloff);
		out_str(T_CE);		// clear rest of this screen line
		screen_start();		// don't know where cursor is now
		force = TRUE;		// force redraw of rest of the line
		redraw_next = TRUE;	// or else next char would miss out

		/*
		 * If the previous character was highlighted, need to stop
		 * highlighting at this character.
		 */
		if (col + coloff > 0 && ScreenAttrs[off_to - 1] != 0)
		{
		    screen_attr = ScreenAttrs[off_to - 1];
		    term_windgoto(row, col + coloff);
		    screen_stop_highlight();
		}
		else
		    screen_attr = 0;	    // highlighting has stopped
	    }
	    if (enc_dbcs != 0)
	    {
		// Check if overwriting a double-byte with a single-byte or
		// the other way around requires another character to be
		// redrawn.  For UTF-8 this isn't needed, because comparing
		// ScreenLinesUC[] is sufficient.
		if (char_cells == 1
			&& col + 1 < endcol
			&& (*mb_off2cells)(off_to, max_off_to) > 1)
		{

