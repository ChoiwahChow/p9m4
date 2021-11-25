#ifndef TP_LITERALS_H
#define TP_LITERALS_H

#include "termflag.h"
#include "tlist.h"


//Simpled Linked list of literals
struct literals {
  bool      sign;
  Term      atom;
  literals  *next;
};
typedef struct literals * Literals;


class GlobalLiterals {
						private:
									unsigned Literals_gets, Literals_frees;
                        

                        public:            
                                     GlobalLiterals();
                                     ~GlobalLiterals();

						friend class LiteralsContainer;
                        friend class LadrVGlobais;
};

class LiteralsContainer {
							private: 
										Literals head;
										Literals get_literals(void);
										void free_literals(Literals);
										
										
							
							public:	
										LiteralsContainer();
										~LiteralsContainer();
										void fprint_literals_mem(ostream &, bool);
										void p_literals_mem();
										void zap_literal(Literals);
										void zap_literals(Literals);
										Literals new_literal(int, Term);
										Literals copy_literal(Literals);
										Literals append_literal(Literals, Literals);
										Literals term_to_literals(Term, Literals);
										Term literal_to_term(Literals);
										Term literals_to_term(Literals);
										Term lits_to_term(Literals);
										void free_lits_to_term(Term);
										int positive_literals(Literals);
										int negative_literals(Literals); 
										bool positive_clause(Literals);
										bool any_clause(Literals);
										bool negative_clause(Literals); 
										bool mixed_clause(Literals);
										int  number_of_literals(Literals);
										bool unit_clause(Literals);
										bool horn_clause(Literals);
										bool definite_clause(Literals);
										int  greatest_variable_in_clause(Literals);
										Plist vars_in_clause(Literals);
										int   number_of_variables(Literals);
										bool  ground_clause(Literals);
										Literals copy_literals(Literals);
										Literals copy_literals_with_flags(Literals);
										Literals copy_literals_with_flag(Literals, int);
										int literal_number(Literals, Literals);
										int atom_number(Literals, Term);
										Literals ith_literal(Literals, int); 
										bool true_clause(Literals);
										bool complementary_scan(Literals, Literals);
										bool tautology(Literals);
										int  symbol_occurrences_in_clause(Literals, int); 
										Literals remove_null_literals(Literals);
										Literals first_literal_of_sign(Literals, bool);
										bool clause_ident(Literals, Literals);
										int clause_symbol_count(Literals);
										int clause_depth(Literals);
										bool pos_eq(Literals);
										bool neg_eq(Literals);
										bool pos_eq_unit(Literals);
										bool neg_eq_unit(Literals);
										bool contains_pos_eq(Literals);
										bool contains_eq(Literals);
										bool only_eq(Literals);
										int literals_depth(Literals);
										Term term_at_position(Literals, Ilist); 
										Ilist pos_predicates(Ilist, Literals); 
                                        Ilist varnums_in_clause(Literals); 
                                        Ilist constants_in_clause(Literals);
                                        
                                        friend class Parautil;
                                        friend class Ac_redun;
};


#endif
