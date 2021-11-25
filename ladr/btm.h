#ifndef TP_BTM_H
#define TP_BTM_H

#include "accanon.h"
#include "unify.h"

#include "memory.h"
#include "fatal.h"
#include "term.h"
#include "accanon.h"
#include "termorder.h"
#include "symbols.h"

//#include "ladrvglobais.h"



/* INTRODUCTION
This package handles "backtrack matching", that is, matching
that allows more than one unifier for a pair of terms, and computes
the unifiers incrementally by backtracking.  As I write this,
we support associative commutative (AC) operations and 
commutative/symmetric (C) operations.  Symbols are declared to be
AC with set_assoc_comm() and C with set_commutative().
The use of Terms and Contexts is similar to ordinary matching,
except that the means for undoing substitutions is different.
*/


struct ac_match_pos; //forward declaration
struct ac_match_free_vars_pos; //forward declaration

struct btm_state {

    btm_state *parent, *next, *prev, *first_child, *last_child;

    Term t1, t2;         /* terms being matched */
    Context c1;          /* context for variables of t1*/

    int varnum;          /* for unbinding when backtracking */
    Context cb;          /* for unbinding when backtracking */

    Unif_alternative alternative;  /* type of alternative (position) */

    /* for commutative unification */
    int flipped;
    btm_state *position_bt;      /* in sequence of alternatives */

    /* for AC matching */
    ac_match_pos *acm;  /* in sequence of AC matchers */
    int partial;               /* partial match for this pair */
    };

typedef struct btm_state * Btm_state;
	

	
struct ac_match_pos {
    Term t1, t2;         /* t1 is pattern, t2 is subject */
    Context c1;          /* context for variables in t1  */
    int n1;              /* number of arguments in t1 */ 
    int n2;              /* size of set of set of args in t2 */
    Term args1[MAX_ACM_ARGS], args2[MAX_ACM_ARGS];  /* the arguments */
           /* position in sequence of matches for complex args of args2 */
    Btm_state bt1[MAX_ACM_ARGS];
           /* flags indicating which of args1 have been matched */
    int match1[MAX_ACM_ARGS];
           /* integer indicating how many of each of args2 have been matched */
    int match2[MAX_ACM_ARGS];
    int mults2[MAX_ACM_ARGS];  /* multiplicities for args2 */
           /* indicates which of args2 are matched by bound vars in args1 */
    int bound_matches[MAX_ACM_ARGS], bound_count;
    int last_a1_symbol;   /* position of last non-variable arg in args1 */
           /* list of backtrack positions for free variables of args1 */
    ac_match_free_vars_pos *free_first, *free_last;
           /* # args of unmatched term---used for partial match */
    int partial_term_size;
    ac_match_pos *next;  /* for avail list only */
    };

	
typedef struct ac_match_pos * Ac_match_pos;

	
struct ac_match_free_vars_pos {
    int varnum;                 /* the index of the free variable */
    int coef;                   /* # of occurrences of the var in args1 */
    int targets[MAX_ACM_ARGS];   /* terms in args2 that can go with variable */
    int n;                      /* number of tragets*/
    int combo[MAX_ACM_ARGS];     /* current subset of the targets */
    ac_match_free_vars_pos *prev, *next;
    };

typedef struct ac_match_free_vars_pos * Ac_match_free_vars_pos;	

#define GO        1
#define EXHAUSTED 2
#define POP       3
#define SUCCESS   4
#define FAILURE   5
#define BACKTRACK 6

#define GO_FUNCTORS   1
#define GO_BOUND_VARS 2
#define GO_FREE_VARS  3


/* Bind a variable, record binding in a bt_node. */
#define BIND_BT(i, c1, t2, c2, bt) {  \
    c1->terms[i] = t2; c1->contexts[i] = c2; \
    bt->varnum = i; bt->cb = c1; }


	
	
class GlobalBtm {
                private:
                        unsigned Ac_match_pos_gets, Ac_match_pos_frees;
                        unsigned Ac_match_free_vars_pos_gets, Ac_match_free_vars_pos_frees;
                        unsigned Btm_state_gets, Btm_state_frees;
                public:        
                        GlobalBtm();
                        ~GlobalBtm();


                    friend class BtmContainer;
                    friend class LadrVGlobais;
};
	

class BtmContainer {
						private: 
									
                                    Btm_state root;
									Ac_match_pos get_ac_match_pos(void);
									void free_ac_match_pos(Ac_match_pos);
									Ac_match_free_vars_pos get_ac_match_free_vars_pos(void);
									void free_ac_match_free_vars_pos(Ac_match_free_vars_pos);
									Btm_state get_btm_state(void);
									void free_btm_state(Btm_state);
									
									
						
						public:	
								BtmContainer();
                                ~BtmContainer();
                                void fprint_btm_mem(ostream &, bool);
								void p_btm_mem();
								
						
								void flatten_mult(Term t, Term *a, int *m, int *ip, int *totp, int (*comp_proc) (void *, void *));
								int macbv_rec(int, Term , Term *, int *,   int *, int, int, int *);
								int match_ac_bound_vars(int , Term *, int ,Term *, int *, int *,int , int,int *, int *, Context);
                                int macbv_rec(int, Term , Term *, int *,int *, int, int *, int *);
								void set_up_free_vars(Ac_match_pos ac, Context c1);
								void unbind_free_var(Ac_match_free_vars_pos, Context);
								int  free_var_match(Ac_match_free_vars_pos, Term *, int *, int *, int, Context, int, int);
								Term build_partial_term(Ac_match_pos);	
								void clear_partial_term(Ac_match_pos);
								int  match_ac(Term, Context, Term,  Btm_state);
								void match_ac_cancel(Ac_match_pos);
								Btm_state match_bt_backup(Btm_state bt1);
								Btm_state match_bt_guts(Btm_state);
								int match_commute(Term, Context, Term, Btm_state);
								void p_acm(Ac_match_pos);
								void p_btm_state(Btm_state);
								Btm_state match_bt_first(Term , Context , Term , int );
								Btm_state match_bt_next(Btm_state );
								void match_bt_cancel(Btm_state);
									
						
};
	



#endif  /* conditional compilation of whole file */
