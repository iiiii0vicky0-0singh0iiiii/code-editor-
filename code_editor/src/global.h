EXTERN long	Rows			// nr of rows in the screen
#ifdef DO_INIT
# if defined(MSWIN)
		    = 25L
# else
		    = 24L
# endif
#endif
		    ;
EXTERN long	Columns INIT(= 80);	// nr of columns in the screen

/*
 * The characters that are currently on the screen are kept in ScreenLines[].
 * It is a single block of characters, the size of the screen plus one line.
 * The attributes for those characters are kept in ScreenAttrs[].
 * The virtual column in the line is kept in ScreenCols[].
 *
 * "LineOffset[n]" is the offset from ScreenLines[] for the start of line 'n'.
 * The same value is used for ScreenLinesUC[], ScreenAttrs[] and ScreenCols[].
 *
 * Note: before the screen is initialized and when out of memory these can be
 * NULL.
 */
EXTERN schar_T	*ScreenLines INIT(= NULL);
EXTERN sattr_T	*ScreenAttrs INIT(= NULL);
EXTERN colnr_T  *ScreenCols INIT(= NULL);
EXTERN unsigned	*LineOffset INIT(= NULL);
EXTERN char_u	*LineWraps INIT(= NULL);	// line wraps to next line

/*
 * When using Unicode characters (in UTF-8 encoding) the character in
 * ScreenLinesUC[] contains the Unicode for the character at this position, or
 * NUL when the character in ScreenLines[] is to be used (ASCII char).
 * The composing characters are to be drawn on top of the original character.
 * ScreenLinesC[0][off] is only to be used when ScreenLinesUC[off] != 0.
 * Note: These three are only allocated when enc_utf8 is set!
 */
EXTERN u8char_T	*ScreenLinesUC INIT(= NULL);	// decoded UTF-8 characters
EXTERN u8char_T	*ScreenLinesC[MAX_MCO];		// composing characters
EXTERN int	Screen_mco INIT(= 0);		// value of p_mco used when
						// allocating ScreenLinesC[]

// Only used for euc-jp: Second byte of a character that starts with 0x8e.
// These are single-width.
EXTERN schar_T	*ScreenLines2 INIT(= NULL);

/*
 * One screen line to be displayed.  Points into ScreenLines.
 */
EXTERN schar_T	*current_ScreenLine INIT(= NULL);

/*
 * Last known cursor position.
 * Positioning the cursor is reduced by remembering the last position.
 * Mostly used by windgoto() and screen_char().
 */
EXTERN int	screen_cur_row INIT(= 0);
EXTERN int	screen_cur_col INIT(= 0);

#ifdef FEAT_SEARCH_EXTRA
// used for 'hlsearch' highlight matching
EXTERN match_T	screen_search_hl;

// last lnum where CurSearch was displayed
EXTERN linenr_T search_hl_has_cursor_lnum INIT(= 0);

// don't use 'hlsearch' temporarily
EXTERN int	no_hlsearch INIT(= FALSE);
#endif

#ifdef FEAT_FOLDING
EXTERN foldinfo_T win_foldinfo;	// info for 'foldcolumn'
#endif

// Flag that is set when drawing for a callback, not from the main command
// loop.
EXTERN int redrawing_for_callback INIT(= 0);

/*
 * Indexes for tab page line:
 *	N > 0 for label of tab page N
 *	N == 0 for no label
 *	N < 0 for closing tab page -N
 *	N == -999 for closing current tab page
 */
