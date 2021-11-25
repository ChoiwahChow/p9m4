#include "clausify.h"
#include "literals.h"
#include "mystring.h"
#include "fatal.h"


Literals Clausify::formula_to_literal(Formula f) {
  
  TermContainer T;
  if (f->type == Ftype::NOT_FORM && f->kids[0]->type == Ftype::ATOM_FORM)
    return LADRV_GLOBAIS_INST.Lit.new_literal(false, T.copy_term(f->kids[0]->atom));
  else if (f->type == Ftype::ATOM_FORM)
    return LADRV_GLOBAIS_INST.Lit.new_literal(true, T.copy_term(f->atom));
  else
    return NULL;
} 


Literals Clausify::formula_to_literals(Formula f) {

  Literals lits=NULL;
  if (f->type == Ftype::ATOM_FORM || f->type == Ftype::NOT_FORM)
    lits=LADRV_GLOBAIS_INST.Lit.append_literal(lits,formula_to_literal(f));
  else if (f->type == Ftype::OR_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
      lits = LADRV_GLOBAIS_INST.Lit.append_literal(lits,formula_to_literal(f->kids[i]));
  }
  else {
    fatal::fatal_error("formula_to_literals, formula not ATOM, NOT, or OR");
  }
  return lits;
}

Topform Clausify::formula_to_clause(Formula f) {
  TopformContainer T;
  Topform c = T.get_topform();
  c->literals = formula_to_literals(f);
  T.upward_clause_links(c);
  return c;
} 


Plist Clausify::formula_to_clauses(Formula f) {
  PlistContainer P;
  if (f->type == Ftype::AND_FORM) {
    int i;
    for (i = 0; i < f->arity; i++)
	   P.plist_append(formula_to_clause( f->kids[i]) );
  }
  else 
      if (f->type == Ftype::OR_FORM || f->type == Ftype::NOT_FORM || f->type == Ftype::ATOM_FORM)
        P.plist_append(formula_to_clause(f));
  return P.get_head();
}


Plist Clausify::clausify_formula(Formula f) {
  Formula g;
  Plist clauses, p;
  Cnf C;
  TopformContainer TF;
  FormulaContainer F;
  g = C.clausify_prepare(F.formula_copy(f));
  clauses = formula_to_clauses(g);
  for (p = clauses; p; p = p->next) {
    Topform c = Topform(p->v);
    TF.renumber_variables(c, MAX_VARS);
  }
  F.zap_formula(g);
  return clauses;
}

Term Clausify::vars_to_names(Term t) {
  TermContainer T;
  SymbolContainer S;
  if (VARIABLE(t)) {
    Term a;
    string s1;
	string s2;
	
    Variable_Style v = S.variable_style();
    s2 = myString::int_to_str(VARNUM(t), s2, 25);

    switch (v) {
    case Variable_Style::INTEGER_STYLE:   s1 = myString::null_string();  break;
    case Variable_Style::STANDARD_STYLE:  s1 = "var_";					 break;
    case Variable_Style::PROLOG_STYLE:    s1 = "VAR_"; 					 break;
    }
    s1 = s1+s2;

    a = T.get_rigid_term(s1, 0);
    T.free_term(t);
    return a;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = vars_to_names(ARG(t,i));
    return t;
  }
}

Formula Clausify::literal_to_formula(Literals lit) {
  FormulaContainer F;
  TermContainer T;
  Formula a = F.formula_get(0, Ftype::ATOM_FORM);
  a->atom = vars_to_names(T.copy_term(lit->atom));
  if (lit->sign) return a;
  else {
    Formula n = F.formula_get(1, Ftype::NOT_FORM);
    n->kids[0] = a;
    return n;
  }
}

Formula Clausify::clause_to_formula(Topform c) {
  FormulaContainer F;
  TermContainer T;
  SymbolContainer S;
 
  if (c->literals == NULL) {
    Formula f = F.formula_get(0, Ftype::ATOM_FORM);
    f->atom = T.get_rigid_term(S.false_sym(), 0);
    return f;
  }
  else {
    Literals lit;
    int i;
    Formula f = F.formula_get(LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals), Ftype::OR_FORM);
    for (lit = c->literals, i = 0; lit; lit = lit->next, i++) {
      f->kids[i] = literal_to_formula(lit);
    }
    return f;
  }
}
