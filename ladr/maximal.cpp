#include "maximal.h"
#include "fatal.h"
#include "termflag.h"
#include "mystring.h"


int Maximal::Maximal_flag=-1;
int Maximal::Maximal_signed_flag=-1;  
int Maximal::Selected_flag=-1; 

TermflagContainer Maximal::TF;
SymbolContainer Maximal::S;



void Maximal::init_maximal(void) {
   
  if (Maximal_flag != -1)
    fatal::fatal_error("init_maximal, called more than once");
  Maximal_flag = TF.claim_term_flag();
  Maximal_signed_flag = TF.claim_term_flag();
  Selected_flag = TF.claim_term_flag();
} 


bool Maximal::greater_literals(Literals l1, Literals l2) {
  
  Term a1 = l1->atom;
  Term a2 = l2->atom;
  OrderType p = S.sym_precedence(SYMNUM(a1), SYMNUM(a2));
  if (p == OrderType::GREATER_THAN)  return true;
  else if (p == OrderType::LESS_THAN) return false;
  else if (SYMNUM(a1) != SYMNUM(a2))  return false;
  else if (S.is_eq_symbol(SYMNUM(a1)))  return TermOrder::greater_multiset_current_ordering(a1, a2);
  else   return TermOrder::term_greater(a1, a2,false);  /* LPO, RPO, KBO */
}

bool Maximal::max_lit_test(Literals lits, Literals lit) {
  /* If there is a greater literal of ANY sign, return FALSE. */
  Literals l2 = lits;
  bool max = true;
  while (l2 && max) {
    if (lit != l2 && greater_literals(l2, lit))    max = false;
    else l2 = l2->next;
  }
  return max;
}

bool Maximal::max_signed_lit_test(Literals lits, Literals lit) {
  /* If there is a greater literal of the same sign, return FALSE. */
  Literals l2 = lits;
  bool max = true;
  while (l2 && max) {
    if (lit != l2 && lit->sign == l2->sign && greater_literals(l2, lit)) max = false;
  else l2 = l2->next;
  }
  return max;
} 

void Maximal::mark_maximal_literals(Literals lits) {
  
  Literals lit;

  if (Maximal_flag == -1)
    fatal::fatal_error("mark_maximal_literals, init_maximal() was not called");
  
  /* Note: we mark the atom, not the literal. */

  for (lit = lits; lit; lit = lit->next) {
    if (max_lit_test(lits, lit))  TF.term_flag_set(lit->atom, Maximal_flag);
    if (max_signed_lit_test(lits, lit)) TF.term_flag_set(lit->atom,Maximal_signed_flag);
  }
}

bool Maximal::maximal_literal(Literals lits, Literals lit, int check) {
  
  if (check ==(int) CheckType::FLAG_CHECK)
    return TF.term_flag(lit->atom, Maximal_flag);
  else
    return max_lit_test(lits, lit);
} 

bool Maximal::maximal_signed_literal(Literals lits, Literals lit, int check) {
  
  if (check == (int) CheckType::FLAG_CHECK)
    return TF.term_flag(lit->atom, Maximal_signed_flag);
  else return max_signed_lit_test(lits, lit);
}

int Maximal::number_of_maximal_literals(Literals lits, int check) {
  int n = 0;
  Literals lit;
  for (lit = lits; lit; lit = lit->next) {
    if (maximal_literal(lits, lit, check))   n++;
  }
  return n;
}  /* number_of_maximal_literals */

void Maximal::mark_selected_literal(Literals lit){
  TF.term_flag_set(lit->atom, Selected_flag);
} 


void Maximal::mark_selected_literals(Literals lits, string selection){
  Literals lit;

  if (Selected_flag == -1)
    fatal::fatal_error("mark_selected_literals, init_maximal() was not called");
  
  /* Note: we mark the atom, not the literal. */

  for (lit = lits; lit; lit = lit->next) {
    if (!lit->sign) {
      if (myString::str_ident(selection, "all_negative"))
			mark_selected_literal(lit);
      else if (myString::str_ident(selection, "max_negative")) {
		if (maximal_signed_literal(lits, lit, (int) CheckType::FLAG_CHECK))
			mark_selected_literal(lit);
      }
    }
  }
}

bool Maximal::selected_literal(Literals lit) {
  return TF.term_flag(lit->atom, Selected_flag);
}

bool Maximal::exists_selected_literal(Literals lits) {
  Literals lit;
  for (lit = lits; lit; lit = lit->next)
    if (selected_literal(lit))   return true;
  return false;
}

void Maximal::copy_selected_literal_marks(Literals a, Literals b) {
  if (!a || !b) return;
  else {
		copy_selected_literal_marks(a->next, b->next);
		if (selected_literal(a))  mark_selected_literal(b);
  }
}
