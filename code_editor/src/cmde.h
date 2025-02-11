#define EX_RANGE	0x001	// allow a linespecs
#define EX_BANG		0x002	// allow a ! after the command name
#define EX_EXTRA	0x004	// allow extra args after command name
#define EX_XFILE	0x008	// expand wildcards in extra part
#define EX_NOSPC	0x010	// no spaces allowed in the extra part
#define	EX_DFLALL	0x020	// default file range is 1,$
#define EX_WHOLEFOLD	0x040	// extend range to include whole fold also
				// when less than two numbers given
#define EX_NEEDARG	0x080	// argument required
#define EX_TRLBAR	0x100	// check for trailing vertical bar
#define EX_REGSTR	0x200	// allow "x for register designation
#define EX_COUNT	0x400	// allow count in argument, after command
#define EX_NOTRLCOM	0x800	// no trailing comment allowed
#define EX_ZEROR       0x1000	// zero line number allowed
#define EX_CTRLV       0x2000	// do not remove CTRL-V from argument
#define EX_CMDARG      0x4000	// allow "+command" argument
#define EX_BUFNAME     0x8000	// accepts buffer name
#define EX_BUFUNL     0x10000	// accepts unlisted buffer too
#define EX_ARGOPT     0x20000	// allow "++opt=val" argument
#define EX_SBOXOK     0x40000	// allowed in the sandbox
#define EX_CMDWIN     0x80000	// allowed in cmdline window
#define EX_MODIFY    0x100000	// forbidden in non-'modifiable' buffer
#define EX_FLAGS     0x200000	// allow flags after count in argument
#define EX_RESTRICT  0x400000	// forbidden in restricted mode
#define EX_EXPAND    0x800000	// expands wildcards later
#define EX_LOCK_OK  0x1000000	// command can be executed when textlock is
				// set; when missing disallows editing another
				// buffer when curbuf_lock is set
#define EX_NONWHITE_OK 0x2000000  // command can be followed by non-white
#define EX_KEEPSCRIPT  0x4000000  // keep sctx of where command was invoked
#define EX_EXPR_ARG    0x8000000  // argument is an expression
#define EX_WHOLE      0x10000000  // command name cannot be shortened in Vim9
#define EX_EXPORT     0x20000000  // command can be used after :export

#define EX_FILES (EX_XFILE | EX_EXTRA)	// multiple extra files allowed
#define EX_FILE1 (EX_FILES | EX_NOSPC)	// 1 file, defaults to current file
#define EX_WORD1 (EX_EXTRA | EX_NOSPC)	// one extra word allowed

#ifndef DO_DECLARE_EXCMD
/*
 * values for cmd_addr_type
 */
typedef enum {
    ADDR_LINES,		 // buffer line numbers
    ADDR_WINDOWS,	 // window number
    ADDR_ARGUMENTS,	 // argument number
    ADDR_LOADED_BUFFERS, // buffer number of loaded buffer
    ADDR_BUFFERS,	 // buffer number
    ADDR_TABS,		 // tab page number
    ADDR_TABS_RELATIVE,	 // Tab page that only relative
    ADDR_QUICKFIX_VALID, // quickfix list valid entry number
    ADDR_QUICKFIX,	 // quickfix list entry number
    ADDR_UNSIGNED,	 // positive count or zero, defaults to 1
    ADDR_OTHER,		 // something else, use line number for '$', '%', etc.
    ADDR_NONE		 // no range used
} cmd_addr_T;
#endif

#ifndef DO_DECLARE_EXCMD
typedef struct exarg exarg_T;
#endif

/*
 * This array maps ex command names to command codes.
 * The order in which command names are listed below is significant --
 * ambiguous abbreviations are always resolved to be the first possible match
 * (e.g. "r" is taken to mean "read", not "rewind", because "read" comes
 * before "rewind").
 * Not supported commands are included to avoid ambiguities.
 */
#ifdef DO_DECLARE_EXCMD
