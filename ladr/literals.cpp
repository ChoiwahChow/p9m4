#include "literals.h"
#include "memory.h"
#include "term.h"
#include "tlist.h"
#include "symbols.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>




GlobalLiterals::GlobalLiterals() {
    Literals_gets=0;
    Literals_frees=0;
}

GlobalLiterals::~GlobalLiterals() {
}



LiteralsContainer::LiteralsContainer() {head=NULL;}
LiteralsContainer::~LiteralsContainer() {head=NULL;}


Literals LiteralsContainer::get_literals(void) {
  Literals p= (Literals)Memory::memCNew(sizeof(struct literals));
  LADR_GLOBAL_LITERALS.Literals_gets++;
  return(p);
}  

void LiteralsContainer::free_literals(Literals p) {
  Memory::memFree((void *)p, sizeof(struct literals));
  LADR_GLOBAL_LITERALS.Literals_frees++;
} 

void LiteralsContainer::fprint_literals_mem(ostream &o, bool heading) {
  int n;
  if (heading)
    o<<"  type (bytes each)        gets      frees     in use      bytes"<<endl;

  n = sizeof(struct literals);
  o<<"literals ("<<setw(4)<<n<<")      ";
  o<<setw(11)<<LADR_GLOBAL_LITERALS.Literals_gets;
  o<<setw(11)<<LADR_GLOBAL_LITERALS.Literals_frees;
  o<<setw(11)<<LADR_GLOBAL_LITERALS.Literals_gets-LADR_GLOBAL_LITERALS.Literals_frees;
  o<<setw(9)<<((LADR_GLOBAL_LITERALS.Literals_gets-LADR_GLOBAL_LITERALS.Literals_frees) * n) /1024<<endl;
}



/* PUBLIC */
void LiteralsContainer::p_literals_mem() {
  fprint_literals_mem(cout, 1);
} 

void LiteralsContainer::zap_literal(Literals l) {
  TermContainer T;
  T.zap_term(l->atom);
  free_literals(l);
}  


void LiteralsContainer::zap_literals(Literals l) {
  if (l) {
    zap_literals(l->next);
    zap_literal(l);
  }
} 

Literals LiteralsContainer::new_literal(int sign, Term atom) {
  Literals lit = get_literals();
  lit->sign = sign;
  lit->atom = atom;
  return lit;
}

Literals LiteralsContainer::copy_literal(Literals lit) {
    TermContainer T;
    return new_literal(lit->sign, T.copy_term(lit->atom));
} 

Literals LiteralsContainer::append_literal(Literals lits, Literals lit) {
  if (lits == NULL)   return lit;
  else {
    lits->next = append_literal(lits->next, lit);
    return lits;
  }
}


Literals LiteralsContainer::term_to_literals(Term t, Literals lits) {
  TermContainer T;
  SymbolContainer S;
  Literals l;
  if (T.is_term(t, S.false_sym(), 0)) return lits;  /* translates to nothing */
  else if (T.is_term(t, S.or_sym(), 2)) {
    /* Traverse term right-to-left and add to the
     * front of the clause, so order is preserved.
     */
    l = term_to_literals(ARG(t,1), lits);
    l = term_to_literals(ARG(t,0), l);
  }
  else {
    l = get_literals();
    l->next = lits;
    l->sign = !(COMPLEX(t) && T.is_term(t, S.not_sym(), 1));
    if (l->sign) l->atom = T.copy_term(t);
    else
      l->atom = T.copy_term(ARG(t,0));
  }
  return(l);
}

Term LiteralsContainer::literal_to_term(Literals l) {
  TermContainer T;
  SymbolContainer S;
  Term t;
  if (l->sign)
    t = T.copy_term(l->atom);
  else {
    t = T.get_rigid_term(S.not_sym(), 1);
    ARG(t,0) = T.copy_term(l->atom);
  }
  return t;
} 

Term LiteralsContainer::literals_to_term(Literals l) {
  TermContainer T;
  SymbolContainer S;
  Term t = literal_to_term(l);
  if (l->next) {
    Term d = T.get_rigid_term(S.or_sym(), 2);
    ARG(d,0) = t;
    ARG(d,1) = literals_to_term(l->next);
    return d;
  }
  else
    return t;
}


Term LiteralsContainer::lits_to_term(Literals l) {
  TermContainer T;
  SymbolContainer S;
  Term t;

  if (l->sign)
    t = l->atom;
  else {
    t = T.get_rigid_term_dangerously(S.not_symnum(), 1);
    ARG(t,0) = l->atom;
  }
  if (l->next) {
    Term d = T.get_rigid_term_dangerously(S.or_symnum(), 2);
    ARG(d,0) = t;
    ARG(d,1) = lits_to_term(l->next);
    t = d;
  }
  return t;
} 


void LiteralsContainer::free_lits_to_term(Term t) {
  TermContainer T;
  SymbolContainer S;
  if (SYMNUM(t) == S.not_symnum())   T.free_term(t);
  else if (SYMNUM(t) == S.or_symnum()) {
    free_lits_to_term(ARG(t,0));
    free_lits_to_term(ARG(t,1));
    T.free_term(t);
  }
} 

int LiteralsContainer::positive_literals(Literals lits) {
  if (lits == NULL)   return 0;
  else if (lits->sign)  return 1 + positive_literals(lits->next);
  else  return positive_literals(lits->next);
} 

int LiteralsContainer::negative_literals(Literals lits) {
  if (lits == NULL)   return 0;
  else if (!lits->sign)  return 1 + negative_literals(lits->next);
  else   return negative_literals(lits->next);
} 

bool LiteralsContainer::positive_clause(Literals lits) {
  return negative_literals(lits) == 0;
}

bool LiteralsContainer::any_clause(Literals lits) {
  return true;
} 

bool LiteralsContainer::negative_clause(Literals lits) {
  return positive_literals(lits) == 0;
} 

bool LiteralsContainer:: mixed_clause(Literals lits) {
  return (positive_literals(lits) >= 1 && negative_literals(lits) >= 1);
} 

int LiteralsContainer::number_of_literals(Literals lits) {
  if (lits == NULL)    return 0;
  else   return 1 + number_of_literals(lits->next);
} 

bool LiteralsContainer::unit_clause(Literals lits) {
  return number_of_literals(lits) == 1;
} 

//Tem no m?ximo um literal positico
bool LiteralsContainer::horn_clause(Literals lits) {
  return positive_literals(lits) <= 1;
} 

bool LiteralsContainer::definite_clause(Literals lits) {
  return positive_literals(lits) == 1;
}

int LiteralsContainer::greatest_variable_in_clause(Literals lits) {
  TermContainer T;
  if (lits == NULL)   return -1;
  else {
    int max_this = T.greatest_variable(lits->atom);
    int max_rest = greatest_variable_in_clause(lits->next);
    return IMAX(max_this, max_rest);
  }
} 


Plist LiteralsContainer::vars_in_clause(Literals lits) {
  TermContainer T;
  if (lits == NULL)    return NULL;
  else  return T.set_of_vars(lits->atom, vars_in_clause(lits->next));
} 


Ilist LiteralsContainer::varnums_in_clause(Literals lits) {
  IlistContainer I;
  PlistContainer P;
  Plist p; 	

  P.set_head(vars_in_clause(lits));
  p=P.get_head();
  for (p = P.get_head(); p; p = p->next) {
    Term var = (Term) p->v;
    I.ilist_append(VARNUM(var));
  }
  P.zap_plist();
  return I.get_head();
} 


int LiteralsContainer::number_of_variables(Literals lits) {
  PlistContainer P;
  P.set_head(vars_in_clause(lits));
  int n=P.plist_count();
  P.zap_plist();
  return n;
}

bool LiteralsContainer::ground_clause(Literals lits) {
  return greatest_variable_in_clause(lits) == -1;
} 

Literals LiteralsContainer::copy_literals(Literals lits) {
  TermContainer T;
  if (lits == NULL) return NULL;
  else {
    Literals novo = get_literals();
    novo->sign = lits->sign;
    novo->atom = T.copy_term(lits->atom);
    novo->next = copy_literals(lits->next);
    return novo;
  }
}


Literals LiteralsContainer::copy_literals_with_flags(Literals lits) {
  TermflagContainer T;
  if (lits == NULL)   return NULL;
  else {
    Literals novo = get_literals();
    novo->sign = lits->sign;
    novo->atom = T.copy_term_with_flags(lits->atom);
    novo->next = copy_literals(lits->next);
    return novo;
  }
}

Literals LiteralsContainer::copy_literals_with_flag(Literals lits, int flag) {
  TermflagContainer T;
  if (lits == NULL)  return NULL;
  else {
    Literals novo = get_literals();
    novo->sign = lits->sign;
    novo->atom = T.copy_term_with_flag(lits->atom, flag);
    novo->next = copy_literals(lits->next);
    return novo;
  }
} 

int LiteralsContainer::literal_number(Literals lits, Literals lit) {
  if (lits == NULL)  return 0;
  else if (lits == lit) return 1;
  else {
    int n = literal_number(lits->next, lit);
    return n == 0 ? 0 : n+1;
  }
} 

int LiteralsContainer::atom_number(Literals lits, Term atom) {
  if (lits == NULL)    return 0;
  else if (lits->atom == atom)   return 1;
  else {
    int n = atom_number(lits->next, atom);
    return n == 0 ? 0 : n+1;
  }
} 

Literals LiteralsContainer::ith_literal(Literals lits, int i) {
  if (lits == NULL)   return NULL;
  else if (i == 1)   return lits;
  else
    return ith_literal(lits->next, i-1);
} 

bool LiteralsContainer::true_clause(Literals lits){
  TermContainer T; 
  if (lits == NULL)    return false;
  else if (lits->sign && T.true_term(lits->atom))  return true;
  else return true_clause(lits->next);
}

bool LiteralsContainer::complementary_scan(Literals lits, Literals lit) {
  TermContainer T;
  if (lits == NULL)   return false;
  else if (lits->sign != lit->sign && T.term_ident(lits->atom, lit->atom))   return true;
  else    return complementary_scan(lits->next, lit);
} 

bool LiteralsContainer::tautology(Literals lits) {
  TermContainer T;
  if (lits == NULL)    return false;
  else if (lits->sign && T.true_term(lits->atom))   return true;
  else if (!lits->sign && T.false_term(lits->atom)) return true;
  else if (complementary_scan(lits->next, lits))    return true;
  else    return tautology(lits->next);
}  /* tautology */


int LiteralsContainer::symbol_occurrences_in_clause(Literals lits, int symnum) {
  TermContainer T;
  if (lits == NULL)    return 0;
  else  return   T.symbol_occurrences(lits->atom, symnum) + symbol_occurrences_in_clause(lits->next, symnum);
} 


Literals LiteralsContainer::remove_null_literals(Literals l) {
  if (l == NULL)  return NULL;
  else {
    l->next = remove_null_literals(l->next);
    if (l->atom != NULL) return l;
    else {
      Literals m = l->next;
      free_literals(l);
      return m;
    }
  }
} 

Literals LiteralsContainer::first_literal_of_sign(Literals lits, bool sign) {
  if (lits == NULL)  return NULL;
  else if (lits->sign == sign) return lits;
  else return first_literal_of_sign(lits->next, sign);
}


Ilist LiteralsContainer::constants_in_clause(Literals lits) {
  
  if (lits == NULL) return NULL;
  else {
    Ilist p = constants_in_clause(lits->next);
    return Tlist::constants_in_term(lits->atom, p);
  }
} 

bool LiteralsContainer::clause_ident(Literals lits1, Literals lits2) {
  TermContainer T;
  if (lits1 == NULL)   return lits2 == NULL;
  else if (lits2 == NULL)    return false;
  else if (lits1->sign != lits2->sign)    return false;
  else if (!T.term_ident(lits1->atom, lits2->atom))    return false;
  else    return clause_ident(lits1->next, lits2->next);
} 

int LiteralsContainer::clause_symbol_count(Literals lits) {
  TermContainer T;
  if (lits == NULL)    return 0;
  else    return T.symbol_count(lits->atom) + clause_symbol_count(lits->next);
}  /* clause_symbol_count */

/*************
 *
 *   clause_depth()
 *
 *************/

/* DOCUMENTATION
Disjunction and negation signs are not included in the count.
That is, return the depth of the deepest atomic formula.
*/

/* PUBLIC */
int LiteralsContainer::clause_depth(Literals lits) {
  TermContainer T;
  if (lits == NULL)   return 0;
  else {
    int depth_this = T.term_depth(lits->atom);
    int depth_rest = clause_depth(lits->next);
    return IMAX(depth_this, depth_rest);
  }
} 


bool LiteralsContainer::pos_eq(Literals lit) {
  TermContainer T;
  return lit->sign && T.eq_term(lit->atom);
} 

bool LiteralsContainer::neg_eq(Literals lit) {
  TermContainer T;
  return lit->sign == false && T.eq_term(lit->atom);
} 

/* DOCUMENTATION
This function checks if a list of Literals is a positive equality unit
for the purposes of paramodulation and demodulation.
*/

/* PUBLIC */
bool LiteralsContainer::pos_eq_unit(Literals lits) {
  TermContainer T;
  return (unit_clause(lits) &&	  lits->sign &&	  T.eq_term(lits->atom));
}  /* pos_eq_unit */

/*************
 *
 *   neg_eq_unit()
 *
 *************/

/* DOCUMENTATION
This function checks if a list of Literals is a negative equality unit.
*/

/* PUBLIC */
bool LiteralsContainer::neg_eq_unit(Literals lits) {
  TermContainer T;
  return (unit_clause(lits) &&  !lits->sign && T.eq_term(lits->atom));
}  /* neg_eq_unit */


bool LiteralsContainer::contains_pos_eq(Literals lits) {
  if (lits == NULL)   return false;
  else if (pos_eq(lits))  return true;
  else   return contains_pos_eq(lits->next);
}

bool LiteralsContainer::contains_eq(Literals lits) {
  TermContainer T;
  if (lits == NULL)    return false;
  else if (T.eq_term(lits->atom))   return true;
  else   return contains_eq(lits->next);
}


bool LiteralsContainer::only_eq(Literals lits) {
  TermContainer T;
  if (lits == NULL)   return true;
  else if (!T.eq_term(lits->atom))  return false;
  else  return only_eq(lits->next);
}

int LiteralsContainer::literals_depth(Literals lits) {
  TermContainer T;
  if (lits == NULL)    return 0;
  else {
    int m = literals_depth(lits->next);
    int n = T.term_depth(lits->atom);
    return IMAX(m, n);
  }
} 


Term LiteralsContainer::term_at_position(Literals lits, Ilist pos) {
  TermContainer T;
  if (lits == NULL || pos == NULL)   return NULL;
  else {
    Literals lit = ith_literal(lits, pos->i);
    Term t = T.term_at_pos(lit->atom, pos->next);
    return t;
  }
}

Ilist LiteralsContainer::pos_predicates(Ilist p, Literals lits) {
  Literals l;
  IlistContainer I;
  I.set_head(p);
  for (l = lits; l; l = l->next) {
    if (l->sign && ! I.ilist_member(SYMNUM(l->atom)))
      I.ilist_prepend(SYMNUM(l->atom));
  }
  return I.get_head();
}
