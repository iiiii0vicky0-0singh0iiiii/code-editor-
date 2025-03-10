#define USING_FLOAT_STUFF

#include "vim.h"

#if defined(FEAT_EVAL) || defined(PROTO)

#ifdef VMS
# include <float.h>
#endif

#define NAMESPACE_CHAR	(char_u *)"abglstvw"

static int eval2(char_u **arg, typval_T *rettv, evalarg_T *evalarg);
static int eval3(char_u **arg, typval_T *rettv, evalarg_T *evalarg);
static int eval4(char_u **arg, typval_T *rettv, evalarg_T *evalarg);
static int eval5(char_u **arg, typval_T *rettv, evalarg_T *evalarg);
static int eval6(char_u **arg, typval_T *rettv, evalarg_T *evalarg);
static int eval7(char_u **arg, typval_T *rettv, evalarg_T *evalarg, int want_string);
static int eval8(char_u **arg, typval_T *rettv, evalarg_T *evalarg, int want_string);
static int eval9(char_u **arg, typval_T *rettv, evalarg_T *evalarg, int want_string);
static int eval9_leader(typval_T *rettv, int numeric_only, char_u *start_leader, char_u **end_leaderp);

static char_u *make_expanded_name(char_u *in_start, char_u *expr_start, char_u *expr_end, char_u *in_end);

/*
 * Return "n1" divided by "n2", taking care of dividing by zero.
 * If "failed" is not NULL set it to TRUE when dividing by zero fails.
 */
	varnumber_T
num_divide(varnumber_T n1, varnumber_T n2, int *failed)
{
    varnumber_T	result;

    if (n2 == 0)
    {
	if (in_vim9script())
	{
	    emsg(_(e_divide_by_zero));
	    if (failed != NULL)
		*failed = TRUE;
	}
	if (n1 == 0)
	    result = VARNUM_MIN; // similar to NaN
	else if (n1 < 0)
	    result = -VARNUM_MAX;
	else
	    result = VARNUM_MAX;
    }
    else if (n1 == VARNUM_MIN && n2 == -1)
    {
	// specific case: trying to do VARNUM_MIN / -1 results in a positive
	// number that doesn't fit in varnumber_T and causes an FPE
	result = VARNUM_MAX;
    }
    else
	result = n1 / n2;

    return result;
}
