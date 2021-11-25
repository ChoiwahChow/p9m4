#ifndef TP_TERM_H
#define TP_TERM_H

#include <climits>
#include <iostream>
#include "strbuf.h"
#include "glist.h"
#include "symbols.h"
using namespace std;

/* Public definitions */

#define MAX_VARS  100   /* max number of (distinct) variables per term */
#define MAX_VNUM  5000  /* maximum variable ID, for array of vars */

#define MAX_VAR   INT_MAX     /* max var ID that fits in sym field of term */
#define MAX_SYM   INT_MAX     /* max ID of any rigid symbol */
#define MAX_ARITY UCHAR_MAX   /* max arity of any term (don't make this big) */

#define FLAGS_TYPE unsigned char  /* for private_flags field of Term */



/* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
/* Global enabling of any const matching.  this should be changed
 * to user parameter */
#define MATCH_HINTS_ANYCONST true 

/* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
#define ANYCONST "_AnyConst" /* any constant prefix string */

/* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
#define MAX_ANYCONSTS  10 /* max number of any consts allowed */





typedef struct term * Term;     /* Term is a pointer to a term struct */

struct term {
  int            private_symbol; /* const/func/pred/var symbol ID */
  unsigned char  arity;          /* number of auguments */
  FLAGS_TYPE     private_flags;  /* for marking terms in various ways */
  Term           *args;          /* array (size arity) of pointers to args */
  void           *container;     /* containing object */
  union {
    unsigned     id;             /* unique ID, probably for FPA indexing */
    void         *vp;            /* auxiliary pointer */
  } u;
};



/* to check type of term */
#define VARIABLE(t) ((t)->private_symbol >= 0)      //variáveis o private_symbol é maior ou igual a 0
#define CONSTANT(t) ((t)->private_symbol < 0 && (t)->arity == 0) //constantes o private_symbol é negativo e tem ariedade de 0
#define COMPLEX(t)  ((t)->private_symbol < 0 && (t)->arity > 0) //complexo, o private_symbol é negativo e tem ariedade positiva

/* to get symbol ID from a CONSTANT or COMPLEX term */
#define SYMNUM(t)   (-((t)->private_symbol))

/* to get the variable number of a VARIABLE term */
#define VARNUM(t)   ((t)->private_symbol)

/* to get the arity of a term (VARIABLE terms have arity 0) */
#define ARITY(t)    ((t)->arity)

/* to get the i-th argument of a term (make sure i is in [0..arity-1]) */
#define ARG(t,i)    ((t)->args[i])

/* to get the array of arguments */
#define ARGS(t)    ((t)->args)

/* End of public definitions */



class GlobalTerm {
                    private:
                       unsigned Term_gets, Term_frees;
                       
                       unsigned Arg_mem;                    /* Memory:: (pointers) for arrays of args */
                       Term Shared_variables[MAX_VNUM];
                       
                       bool AnyConstsEnabled; 
                       
                       /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
                       bool AnyConstsInited; /* are AnyConsts values valid? */

                       /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
                       int AnyConsts[MAX_ANYCONSTS]; /* symbol numbers of "_AnyConsts_n" */
                     
                        
                     
                      
                      
                    public:  
                        GlobalTerm();
                        ~GlobalTerm();
                        void Free_Mem(void);
                       
                      friend class TermContainer;
                      friend class LadrVGlobais;
                      friend class FpaIndexContainer;
                      friend class Subsume;
                      friend class UnifyContainer;
                      
    
};


class TermContainer {
                        private:
                                    void init_any_consts();
                                  
                                    

                        public:
                                    TermContainer();
                                    ~TermContainer();
                                    Term get_term(int);
                                    void free_term(Term);
                                    
                                    void fprint_term_mem(ostream &, const bool) const;
                                    void p_term_mem(void) const;
                                    
                                    Term get_variable_term(int);
                                    Term get_rigid_term_like(Term);
                                    Term get_rigid_term(const string &, int);
                                    Term get_rigid_term_dangerously(int, int); 
                                    void zap_term(Term t);
                                    bool term_ident(Term, Term);
                                    Term copy_term(Term);
                                    bool ground_term(Term);
                                    int  biggest_variable(Term);
                                    int  term_depth(Term);
                                    int  symbol_count(Term);
                                    bool occurs_in(Term, Term);
                                    void fprint_term(ostream &, Term);
                                    void sprint_term(String_buf, Term);
                                    const string &term_to_string(Term);
                                    void p_term(Term);
                                    bool all_args_vars(Term);
                                    
                                    Term build_binary_term(int, Term , Term );
                                    Term build_binary_term_safe(const string &, Term , Term );
                                    Term build_unary_term(int , Term );
                                    Term build_unary_term_safe(const string &, Term);
                                    Term subst_term(Term , Term , Term);
                                    Term subst_var_term(Term , int,int );
                                    int  greatest_variable(Term);
                                    int greatest_symnum_in_term(Term);
                                    void upward_term_links(Term , void *);
                                    bool check_upward_term_links(Term , void *);
                                    int occurrences(Term, Term);
                                    Term trm_set_vars_recurse(Term, string[], int);
                                    void term_set_variables(Term , int );
                                    Term nat_to_term(int);
                                    Term int_to_term(int);
                                  
                                    
                                    Term bool_to_term(bool);
                                    Term double_to_term(double);
                                    int natural_constant_term(Term);
                                    int arg_position(Term, Term);
                                    bool is_term(Term, const string &, int);
                                    bool is_constant(Term, const string &);
                                    const string &term_symbol(Term);
                                    bool term_to_int(Term, int *);
                                    bool term_to_double(Term, double *);
                                    bool term_to_number(Term, double *);
                                    bool true_term(Term);
                                    bool false_term(Term);
                                    bool term_to_bool(Term , bool *);
                                    
                                    I2list symbols_in_term(Term , I2list );
                                    Ilist fsym_set_in_term(Term );
                                    Term renum_vars_recurse(Term , int[], int);
                                    Term set_vars_recurse(Term , string [], int);
                                    I2list multiset_of_vars(Term, I2list);
                                    I2list multiset_vars(Term);
                                    Plist set_of_vars(Term, Plist);
                                    Plist set_of_variables(Term);
                                    int number_of_vars_in_term(Term);
                                    Ilist set_of_ivars(Term, Ilist);
                                    Ilist set_of_ivariables(Term t);
                                    bool variables_subset(Term, Term);
                                    bool variables_multisubset(Term, Term);
                                    Term term_at_pos(Term, Ilist);
                                    Ilist pos_of_subterm(Term, Term);
                                    Ilist position_of_subterm(Term , Term);
                                    int symbol_occurrences(Term, int);
                                    bool args_distinct_vars(Term);
                                    unsigned hash_term(Term);
                                    bool skolem_term(Term);
                                    
                                    bool  contains_skolem_term(Term);
                                    Term  term0(const string &);
                                    Term  term1(const string &, Term);
                                    Term  term2(const string &, Term, Term);
                                    bool  symbol_in_term(int, Term);
                                    bool  same_structure(Term, Term);
                                    Plist copy_plist_of_terms(Plist);
                                    void  zap_plist_of_terms(Plist);
                                    Plist plist_of_subterms(Term);
                                    bool  tlist_member(Term, Plist);
                                    int   position_of_term_in_tlist(Term, Plist);
                                    bool tlist_subset(Plist,Plist);
                                    bool  tlist_set(Plist);
                                    bool  eq_term(Term);
                                    Plist free_vars_term(Term, Plist);
                                    
                                    /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
                                    int any_const(int sn);
                                    /* JJ(2016-nov-10): Match any constant (_AnyConst) in hints */
                                    int any_const_sn(int n);
    
    
};




#endif
