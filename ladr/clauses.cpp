#include "clauses.h"
#include "literals.h"
#include <cfloat>

Topform ClausesContainer::clause_member_plist(Plist p, Topform c) {

  while (p) {
    Topform d = (Topform) p->v;
    if (LADRV_GLOBAIS_INST.Lit.clause_ident(c->literals, d->literals))
      return (Topform)p->v;
    p = p->next;
  }
  return NULL;
}

Plist ClausesContainer::intersect_clauses(Plist a, Plist b) {
  if (a == NULL) return NULL;
  else {
    Plist c = intersect_clauses(a->next, b);
    if (clause_member_plist(b, (Topform) a->v))  {
		PlistContainer P;
		P.set_head(c);
		P.plist_prepend(a->v);
		return P.get_head();
	}
    else
      return c;
  }
}

double ClausesContainer::max_clause_weight(Plist p){
  if (p == NULL) return DBL_MIN;
  else {
    double max_rest = max_clause_weight(p->next);
    Topform c = (Topform) p->v;
    return (c->weight > max_rest ? c->weight : max_rest);
  }
} 

int ClausesContainer::max_clause_symbol_count(Plist p) {
  
  if (p == NULL) return INT_MIN;
  else {
    int max_rest = max_clause_symbol_count(p->next);
    Topform c = (Topform) p->v;
    return IMAX(LADRV_GLOBAIS_INST.Lit.clause_symbol_count(c->literals), max_rest);
  }
}  /* max_clause_symbol_count */


Plist ClausesContainer::nonneg_clauses(Plist clauses) {
  PlistContainer nonneg;
  Plist p;

  for (p = clauses; p; p = p->next) {
    Topform c =(Topform) p->v;
    if (!LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals)) 
      nonneg.plist_append(c);
  }
  return nonneg.get_head();
} 

bool ClausesContainer::all_clauses_horn(Plist l) {
  Plist p;

  for (p = l; p; p = p->next) {
    Topform c = (Topform) p->v;
    if (!LADRV_GLOBAIS_INST.Lit.horn_clause(c->literals))
      return false;
  }
  return true;
} 

bool ClausesContainer::all_clauses_unit(Plist l) {

  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = (Topform) p->v;
    if (!LADRV_GLOBAIS_INST.Lit.unit_clause(c->literals))
      return false;
  }
  return true;
}


bool ClausesContainer::all_clauses_positive(Plist l) {
  Plist p;

  for (p = l; p; p = p->next) {
    Topform c = (Topform) p->v;
    if (!LADRV_GLOBAIS_INST.Lit.positive_clause(c->literals))
      return false;
  }
  return true;
}

int ClausesContainer::neg_nonunit_clauses(Plist l) {

  int n = 0;
  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = (Topform) p->v;
    if (LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals) && LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) > 1) n++;
  }
  return n;
}

int ClausesContainer::negative_clauses(Plist l) {
  
  int n = 0;
  Plist p;
  for (p = l; p; p = p->next) {
    Topform c = (Topform) p->v;
    if (LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals))
      n++;
  }
  return n;
}


int ClausesContainer::most_literals(Plist clauses) {

  int max = -1;
  Plist p;
  for (p = clauses; p; p = p->next) {
    Topform c = (Topform) p->v;
    int n = LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals);
    max = IMAX(max,n);
  }
  return max;
} 


bool ClausesContainer::pos_equality_in_clauses(Plist clauses) {
 
  if (clauses == NULL) return false;
  else {
    Topform c = (Topform) clauses->v;
    if (LADRV_GLOBAIS_INST.Lit.contains_pos_eq(c->literals)) return true;
    else  return pos_equality_in_clauses(clauses->next);
  }
} 


bool ClausesContainer::equality_in_clauses(Plist clauses) {
 
  if (clauses == NULL)    return false;
  else {
    Topform c = (Topform) clauses->v;
    if (LADRV_GLOBAIS_INST.Lit.contains_eq(c->literals))    return true;
    else
      return equality_in_clauses(clauses->next);
  }
} 

int ClausesContainer::imax_clause_weight(Plist p) {
  int real_weight;
  if (p == NULL)
    return INT_MIN;
  else {
    int max_rest = imax_clause_weight(p->next);
    Topform c = (Topform) p->v;
    real_weight = (int) c->weight % 500;
    return (real_weight > max_rest ? real_weight : max_rest);
  }
    
    
}

