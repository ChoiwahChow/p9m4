#ifndef TP_UNIFY_H
#define TP_UNIFY_H

#include "listterm.h"
#include "termflag.h"

/* Dereference a variable. */

#define DEREFERENCE(t, c) { int i; \
    while (c!=NULL && VARIABLE(t) && c->terms[i=VARNUM(t)]) \
    { t = c->terms[i]; c = c->contexts[i]; } }
    
#define BIND_TR(i, c1, t2, c2, trp) { struct trail *tr; \
    c1->terms[i] = t2; c1->contexts[i] = c2; \
    tr = get_trail(); tr->varnum = i; tr->context = c1; \
    tr->next = *trp; *trp = tr; }

#define MAX_MULTIPLIERS 500
#define MAX_ANYVARS     10

enum class Unif_alternative {
                                NO_ALT = 0,
                                AC_ALT,
                                COMM_ALT
};

struct context {
  Term    terms[MAX_VARS];    /* terms substituted for variables */
  context *contexts[MAX_VARS]; /* Contexts corresponding to terms */
  int     multiplier;         /* for getting separate vars in apply */
  Term    partial_term;       /* for AC matching */
};



typedef struct context *Context;

struct trail {
  int     varnum;   /* index of instantiated variable */
  Context context;  /* context of instanteated variable */
  trail *   next;     /* next (earlier) member of trail */
};


typedef struct trail *Trail;

class GlobalUnify {
                    private:
                            unsigned Context_gets, Context_frees;
                            unsigned Trail_gets, Trail_frees;
                            bool AnyVarsInited;
                            int AnyVars[MAX_ANYVARS];
                            
                            bool Multipliers[MAX_MULTIPLIERS];
                            void initMultipliers(void); //isto tem de ser chamado no initvGlobais
                    
                    public:       
                            GlobalUnify();
                            ~GlobalUnify();
                            
                            friend class UnifyContainer;
                            friend class LadrVGlobais;
                            
                            

};



class UnifyContainer {
                        private:
                                    int next_available_multiplier(void)const;
                                    bool match_anyvar(int, Term, int *);
                                    int any_var(int);
                                    bool match_anyconst(Term , Term, int *, Ilist *);
                                    
                        public:
                                    Context get_context(void) const;
                                    void    free_context(Context) const;
                                    Trail   get_trail(void) const;
                                    void    free_trail(Trail) const;
                                    void    fprint_unify_mem(ostream &, const bool) const;
                                    void    p_unify_mem(void) const ;
                                    bool    unify(Term , Context , Term , Context , Trail *);
                                    bool    occur_check(int, Context , Term , Context);
                                    bool    match(Term , Context , Term , Trail *);
                                    bool    match2(Term , Context , Term , Trail *);
                                    Term    apply(Term , Context );
                                    Term    apply_substitute(Term , Term , Context ,Term , Context);
                                    Term    apply_substitute2(Term , Term , Context ,Ilist , Context);
                                    Term    apply_demod(Term, Context, int);
                                    void    undo_subst(Trail);
                                    void    undo_subst_2(Trail, Trail);
                                    void    fprint_context(ostream &, const Context) const;
                                    void    p_context(const Context) const ;
                                    void    fprint_trail(ostream &, const Trail) const;
                                    void    p_trail(const Trail)const ;
                                    bool    match_weight(Term , Context , Term , Trail *, int *);
                                    Ilist   vars_in_trail(Trail );
                                    Plist context_to_pairs(Ilist, Context);
                                    bool empty_substitution(Context);
                                    bool variable_substitution(Context);
                                    bool subst_changes_term(Term, Context);
                                    bool variant(Term , Context , Term , Trail *);
                                    bool match_anyctx(Term, Context, Term, Trail *, int *, Ilist*);

};

#endif
//-----------------------------------------------------------------------------------------------------
