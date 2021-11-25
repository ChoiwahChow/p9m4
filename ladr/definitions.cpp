#include "definitions.h"
#include "fatal.h"


bool DefinitionsContainer::is_definition(Formula f) {
  FormulaContainer F;
  TermContainer T;
  if (!F.closed_formula(f)) return false;
  else {
    PlistContainer vars;
	bool ok = true;
    Formula g;
    for (g = f; g->type == Ftype::ALL_FORM; g = g->kids[0]) {
      Term t = T.get_rigid_term(*(g->qvar), 0);
      vars.plist_append(t);
    }
    if (g->type != Ftype::IFF_FORM || g->kids[0]->type != Ftype::ATOM_FORM) ok = false;
    else {
      Term atom = g->kids[0]->atom;
      PlistContainer args;
	  args.set_head(T.plist_of_subterms(atom));
      ok = (ARITY(atom) == vars.plist_count() &&
	    T.tlist_set(vars.get_head()) &&
	    T.tlist_subset(vars.get_head(), args.get_head()) &&
	    T.tlist_subset(args.get_head(), vars.get_head()));
      args.zap_plist();  /* shallow */
    }
    T.zap_plist_of_terms(vars.get_head());  /* deep */

    if (ok) {
      /* check that the defined symbol does not occur in the right side */
      ok = !F.relation_symbol_in_formula(SYMNUM(g->kids[0]->atom), g->kids[1]);
    }
    return ok;
  }
}


Term DefinitionsContainer::subst_terms(Term t, Plist vars, Plist terms, Plist qvars) {
  /* In Term t, replace members of vars (really constants) with
     (copies of) the corresponding members of terms.
     EXCEPT if the var (constant) is a member of qvars.
     Note that "vars" and "terms" are lists of terms, and "qvars"
     is a list of strings.
   */
  SymbolContainer S;
  PlistContainer P;
  TermContainer T;
  if (CONSTANT(t)) {
	P.set_head(qvars);  
    if (!P.string_member_plist(S.sn_to_str(SYMNUM(t)))) {
      int i = T.position_of_term_in_tlist(t, vars);
      if (i != -1) {
				P.set_head(terms);
				Term corresponding_term = (Term) P.ith_in_plist(i);
				T.zap_term(t);
				t = T.copy_term(corresponding_term);
      }
    }
    return t;
  }
  else {
    int i;
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = subst_terms(ARG(t,i), vars, terms, qvars);
    return t;
  }
}

void DefinitionsContainer::subst_free_vars(Formula f, Plist vars, Plist terms, Plist qvars) {
  FormulaContainer F;
  if (f->type == Ftype::ATOM_FORM)  f->atom = subst_terms(f->atom, vars, terms, qvars);
  else if (F.quant_form(f)) {
    PlistContainer QVARS;
	QVARS.set_head(qvars);
	Plist qvars2 = QVARS.plist_prepend((void *) & (f->qvar));
    subst_free_vars(f->kids[0], vars, terms, qvars2);
    PlistContainer QVARS2;
	QVARS2.free_plist(qvars2);  /* first_node only, leaves qvars as it was */
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      subst_free_vars(f->kids[i], vars, terms, qvars);
  }
}

Formula DefinitionsContainer::subst_atomic_formula(Formula f, Formula target, Formula replacement) {
  /* We have to do *simultaneous* replacement, instead of making
     a sequence of calls to subst_free_var.  For example, consider
     f=p(y,z), target=p(x,y), replacement=r(x,y).  Sequential
     replacement gives r(z,z) instead of r(y,z).
   */
 
  if (f->type == Ftype::ATOM_FORM) {
    if (SYMNUM(f->atom) == SYMNUM(target->atom)) {
      FormulaContainer F; TermContainer T;
	  Formula novo = F.formula_copy(replacement);
      Plist vars  = T.plist_of_subterms(target->atom);
      Plist terms = T.plist_of_subterms(f->atom);
      /* We have to prevent capture of constants in f by quantifiers
	 in the repalcement. */
      Ilist constants_in_f = F.constants_in_formula(f);
      F.rename_these_bound_vars(novo, constants_in_f);
      subst_free_vars(novo, vars, terms, NULL);
      PlistContainer P;
	  P.set_head(vars);
	  P.zap_plist();
	  P.set_head(terms);
      P.zap_plist();
	 
	  
      IlistContainer I(constants_in_f);
      I.zap_ilist();
      T.zap_term(f->atom);
      return novo;
    }
  }
  else {
    int i;
    for (i = 0; i < f->arity; i++)
      f->kids[i] = subst_atomic_formula(f->kids[i], target, replacement);
  }
  return f;
}

Formula DefinitionsContainer::strip_quantifiers(Formula f) {
  FormulaContainer F;
  while (F.quant_form(f))
    f = f->kids[0];
  return f;
} 

bool DefinitionsContainer::definition_applies(Formula f, Formula def) {
  int symnum;
  FormulaContainer F;
  if (!is_definition(def)) {
    F.p_formula(def);
    fatal::fatal_error("definition_applies received non-definition");
  }
  def = strip_quantifiers(def);
  symnum = SYMNUM(def->kids[0]->atom);
  return F.relation_in_formula(f, symnum);
}

Formula DefinitionsContainer::expand_with_definition(Formula f, Formula def) {
  FormulaContainer F;
  Formula g = F.formula_copy(f);
  Formula result;
  Ilist constants_in_def;
  if (!is_definition(def)) {
    F.p_formula(def);
    fatal::fatal_error("expand_with_definition received non-definition");
  }
  /* We have to prevent capture of constants in the definition
     by quantifiers in the formula being expanded. */
  constants_in_def = F.constants_in_formula(def);
  F.rename_these_bound_vars(g, constants_in_def);
  IlistContainer I(constants_in_def);
  I.zap_ilist();
  def = strip_quantifiers(def);
  result = subst_atomic_formula(g, def->kids[0], def->kids[1]);
  return result;
}


Topform DefinitionsContainer::first_definition(Plist p) {
  if (p == NULL)  return NULL;
  else {
    Topform tf = (Topform) p->v;
    if (is_definition(tf->formula))
      return tf;
    else
      return first_definition(p->next);
  }
}

void DefinitionsContainer::process_definitions(Plist formulas,Plist *results,Plist *defs,Plist *rewritten) {
  Plist work = formulas;
  Topform def;
  PlistContainer P;
  TopformContainer TP;
  ClauseidContainer C;
  JustContainer J;
  *defs = NULL;
  *rewritten = NULL;

  def = first_definition(work);
  while (def) {
    Plist p;
	P.set_head(work);
    work = P.plist_remove(def);
	P.set_head(*defs);
    P.plist_append(def);
    for (p = work; p; p = p->next) {
      Topform old =(Topform) p->v;
      if (definition_applies(old->formula, def->formula)) {
		Topform novo = TP.get_topform();
		novo->is_formula = true;
		novo->formula = expand_with_definition(old->formula, def->formula);
		C.assign_clause_id(novo);
		novo->justification = J.expand_def_just(old, def);
		P.set_head(*rewritten);
		P.plist_append(old);
		p->v = (void *) novo;
      }
    }
    def = first_definition(work);
  }
  *results = work;
}  

void DefinitionsContainer::expand_with_definitions(Plist formulas,Plist defs,Plist *results,Plist *rewritten) {
  Plist p1;
  TopformContainer TP;
  PlistContainer P;
  ClauseidContainer C;
  JustContainer J;

  for (p1 = formulas; p1; p1 = p1->next) {
    Topform work = (Topform) p1->v;
    Plist p2;
    for (p2 = defs; p2; p2 = p2->next) {
      Topform def = Topform (p2->v);
      if (definition_applies(work->formula, def->formula)) {
		Topform novo = TP.get_topform();
		novo->is_formula = true;
		novo->formula = expand_with_definition(work->formula, def->formula);
		C.assign_clause_id(work);
		novo->justification = J.expand_def_just(work, def);
		P.set_head(*rewritten);
        *rewritten = P.plist_prepend(work);
		work = novo;
      }
    }
    P.set_head(*results);
    *results = P.plist_prepend(work);
  }
  P.set_head(formulas);
  P.zap_plist();  /* shallow */
  P.set_head(*results);
  P.reverse_plist();
  *results=P.get_head();
} 

void DefinitionsContainer::separate_definitions(Plist formulas,Plist *defs,Plist *nondefs) {
  Plist p;
  PlistContainer P;
  *defs = NULL;
  *nondefs = NULL;
  for (p = formulas; p; p = p->next) {
    Topform tf =(Topform) p->v;
    
	if (is_definition(tf->formula)) {
		P.set_head(*defs);
        *defs = P.plist_prepend(tf);
	}	
    else {
       P.set_head(*nondefs);
	   *nondefs = P.plist_prepend(tf);
	}
  }
  P.set_head(formulas); 	
  P.zap_plist();  /* shallow */
  P.set_head(*defs);
  *defs = P.reverse_plist();
  P.set_head(*nondefs);
  *nondefs = P.reverse_plist();
} 
