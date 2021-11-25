#ifndef _TP_FORMULA_H
#define _TP_FORMULA_H



#include "attrib.h"
#include "tlist.h"
#include "termorder.h"
#include "hash.h"

enum class Ftype{
				ATOM_FORM=0,
				AND_FORM,
				OR_FORM,
				NOT_FORM,
				IFF_FORM,
				IMP_FORM,
				IMPBY_FORM, 
				ALL_FORM, 
				EXISTS_FORM
};

typedef struct formula * Formula;

struct formula {
  Ftype       type;
  int         arity;
  string      *qvar;
  Formula     *kids;         /* for non-atoms */
  Term        atom;          /* for atoms */
  Attribute   attributes;    /* */
  int excess_refs;           /* count of extra references */
};




enum class Fpref { 
					CONJUNCTION,
					DISJUNCTION
};

/* macros */

#define TRUE_FORMULA(f)  ((f)->type == Ftype::AND_FORM && (f)->arity == 0)
#define FALSE_FORMULA(f) ((f)->type == Ftype::OR_FORM  && (f)->arity == 0)



class GlobalFormula {
						private:
								 unsigned Formula_gets, Formula_frees;
								 unsigned Arg_mem; 
                                 
                        public:
                                   GlobalFormula();
                                   ~GlobalFormula();
                            
						friend class FormulaContainer;
                        friend class LadrVGlobais;
};


class FormulaContainer {

                            private:
                                    //Formula head;
                                
                                  
                                
                                    Formula get_formula(int);
                                    void free_formula(Formula);
                                    void gather_symbols_in_term(Term, I2list *, I2list *);
                                    
                            public:         
                                    FormulaContainer();
                                    ~FormulaContainer();
                                    
                                    
                                    void fprint_formula_mem(ostream &, bool);

                                    void p_formula_mem();
                                    unsigned formula_megs(void);
                                    Formula formula_get(int, Ftype);
                                    void zap_formula(Formula);
                                    bool logic_term(Term);
                                    void gather_symbols_in_formula_term(Term, I2list *, I2list *);
                                    void gather_symbols_in_formula(Formula, I2list *, I2list *);
                                    void gather_symbols_in_formulas(Plist, I2list *, I2list *);
                                    Ilist function_symbols_in_formula(Formula);
                                    Ilist relation_symbols_in_formula(Formula);
                                    bool  relation_symbol_in_formula(int, Formula);
                                    Formula term_to_formula(Term);
                                    Term formula_to_term(Formula);
                                    void fprint_formula(ostream &, Formula);
                                    void p_formula(Formula);
                                    unsigned hash_formula(Formula);
                                    bool formula_ident(Formula, Formula);
                                    Formula formula_copy(Formula);
                                    Ftype dual_type(Ftype);
                                    Formula dual(Formula);

                                    Formula  _and(Formula, Formula);
                                    Formula _or(Formula, Formula);
                                    Formula _not(Formula);
                                    
                                    
                                    Formula imp(Formula, Formula);
                                    Formula impby(Formula, Formula);

                                    Formula negate(Formula);
                                    bool quant_form(Formula);
                                    Formula flatten_top(Formula);
                                    Formula formula_flatten(Formula);
                                    Formula nnf2(Formula, Fpref);
                                    Formula nnf(Formula);
                                    Formula make_conjunction(Formula);
                                    Formula make_disjunction(Formula);	
                                    void formula_canon_eq(Formula);
                                    int formula_size(Formula); 
                                    int greatest_qvar(Formula);
                                    int greatest_symnum_in_formula(Formula);
                                    Formula elim_rebind(Formula, Ilist);
                                    Formula eliminate_rebinding(Formula);
                                    Plist free_vars(Formula, Plist);
                                    bool closed_formula(Formula);
                                    Formula get_quant_form(Ftype , string qvar, Formula); 
                                    Formula uni_close(Formula, Plist); 
                                    Formula universal_closure(Formula);
                                    bool free_var(string svar, Term, Formula);
                                    bool free_variable(string svar, Formula f);	
                                    Formula formulas_to_conjunction(Plist);
                                    Formula formulas_to_disjunction(Plist);
                                    void subst_free_var(Formula, Term, Term);
                                    Plist copy_plist_of_formulas(Plist);
                                    bool literal_formula(Formula);
                                    bool clausal_formula(Formula);
                                    void formula_set_vars_recurse(Formula, string vnames[], int);
                                    void formula_set_variables(Formula, int);
                                    bool positive_formula(Formula);
                                    bool formula_contains_attributes(Formula);
                                    bool subformula_contains_attributes(Formula);
                                    Ilist constants_in_formula(Formula); 
                                    bool relation_in_formula(Formula, int);
                                    void rename_all_bound_vars(Formula);
                                    void rename_these_bound_vars(Formula, Ilist);
    
                                    friend class Cnf;
};


#endif
