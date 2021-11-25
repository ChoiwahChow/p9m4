#ifndef TP_BTU_H
#define TP_BTU_H

#include "dioph.h"
#include "unify.h"
#include <iostream>


#define MAX_ACU_ARGS 500

#define GO        1
#define EXHAUSTED 2
#define POP       3
#define SUCCESS   4
#define FAILURE   5
#define BACKTRACK 6

/******** bind a variable, record binding in a bt_node ********/

#define BIND_BT(i, c1, t2, c2, bt) {  \
    c1->terms[i] = t2; c1->contexts[i] = c2; \
    bt->varnum = i; bt->cb = c1; }

struct btu_state ;
    
struct ac_position {
                    int m, n, num_basis;             /* # of coefficients and size of basis */
                    int basis[MAX_BASIS][MAX_COEF];
                    int constraints[MAX_COEF];       /* 0 for vars, else symbol number */
                    Term args[MAX_COEF];
                    Context arg_contexts[MAX_COEF];
                    Term new_terms[MAX_COEF]; /* substitution terms */
                    int combo[MAX_BASIS];            /* current subset of basis solutions */
                    int sum[MAX_COEF];               /* solution corresponding to combo */
                    Term basis_terms[MAX_BASIS][MAX_COEF];
                    Context c3;               /* table for new variables */
                    btu_state *sub_position;    /* position in sub-unification problem */
                    int superset_limit;              /* for superset-restricted AC unif. */
                    int combos[MAX_COMBOS][MAX_BASIS];/* for superset-restricted AC unif. */
                    int combos_remaining;            /* for superset-restricted AC unif. */
                    ac_position *next;         /* for avail list only */
};

typedef struct ac_position * Ac_position;

struct btu_state {

                    btu_state *parent, *next, *prev, *first_child, *last_child;
                    Term t1, t2;         /* terms being unified or matched */
                    Context c1, c2;      /* respective contexts for variables */
                    int varnum;          /* for unbinding when backtracking */
                    Context cb;          /* for unbinding when backtracking */
                    Unif_alternative alternative;   /* type of alternative (position) */

                    /* for commutative unification */
                    int flipped;
                    btu_state *position_bt;      /* in sequence of alternatives */

                    /* for AC unification */
                    Ac_position ac;    /* in sequence of AC unifiers */
};

typedef struct btu_state * Btu_state;

/* #define DEBUG */


class GlobalBtu {

                private:
                        unsigned Btu_state_gets, Btu_state_frees;
                        unsigned Ac_position_gets, Ac_position_frees;

                public:
                        GlobalBtu();
                        ~GlobalBtu();
                        
                        friend class BtuContainer;
                        friend class LadrVGlobais;
};


class BtuContainer {
                private:
                            Btu_state root;
                            Btu_state get_btu_state(void);
                            void free_btu_state(Btu_state);
                            Ac_position get_ac_position(void);
                            void free_ac_position(Ac_position);
                            #if 0
                            void p_binding(int, Context, Term, Context);
                            #endif
                            void flatten_deref(Term,  Context , Term *, Context *, int *);
                            OrderType compare_ncv_context(Term , Term ,Context , Context );
                            void sort_ac(Term *, Context *, int);
                            
                public:
                            BtuContainer();
                            ~BtuContainer();
                            void fprint_btu_mem(ostream &, bool);
                            void p_btu_mem();
                            Btu_state unify_bt_first(Term, Context ,Term , Context );
                            Btu_state unify_bt_guts(Btu_state);
                            Btu_state unify_bt_next(Btu_state);
                            
                            
                            void elim_con_context(Term *, Term *, Context *, Context *, int, int);
                            void ac_mult_context(Term *, Context *, int *, int *);
                            void ac_prepare(Term *, Term *, Context *, Context *,  int *, int *, int , int , int *,    int *,   Term *, Context *);
                            void set_up_basis_terms(int sn, int (*basis) [MAX_COEF], int num_basis, int length, Term (*basis_terms)[MAX_COEF]);
                            int unify_ac(Term, Context , Term , Context, struct btu_state *);
                            
                            void unify_ac_cancel(Ac_position);
                            int unify_commute(Term, Context, Term, Context, Btu_state);
                            Btu_state unify_bt_backup(Btu_state);

                            
                            void unify_bt_cancel(Btu_state);
                            void p_bt_tree(Btu_state, int);
                            void p_ac_position(Ac_position, int);
                           
};








#endif
