#include "./ladr/clauseid.h"
#include "pred_elim.h"


int PredElim::rsym_occurrences(int symbol, Topform c){
  int n = 0;
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next)
    if (SYMNUM(lit->atom) == symbol)
      n++;
  return n;
}

bool PredElim::arg_check(int symbol, Topform c){
  TermContainer T;
  Literals lit;
  for (lit = c->literals; lit; lit = lit->next) {
    if (!lit->sign &&	SYMNUM(lit->atom) == symbol &&	!T.args_distinct_vars(lit->atom))  return false;
 }
  return true;
}

bool PredElim::eliminable_relation(int symbol, Plist clauses, bool equality) {
  /* Eliminable if no clause has more than one occurrence.
     Additional constraint for equality: all negative occurrences
     have unique vars as args.
  */
  Plist p;
  for (p = clauses; p; p = p->next) {
    int n = rsym_occurrences(symbol, (Topform) p->v);
    if (n > 1)  return false;
    else if (equality && !arg_check(symbol, (Topform) p->v))  return false;
  }
  return true;
} 

Ilist PredElim::eliminable_relations(Plist clauses, bool equality) {
  SymbolContainer S;
  IlistContainer IRSYMS, ELIMINATE;
  TopformContainer TF;
  Ilist rsyms = TF.rsym_set_in_topforms(clauses);
  Ilist eliminable = NULL;
  Ilist p;
  for (p = rsyms; p; p = p->next) {
    if (!S.is_eq_symbol(p->i) &&          /* don't allow equalities */
		S.sn_to_arity(p->i) != 0 &&      /* don't allow prop atoms */
		eliminable_relation(p->i, clauses, equality))
      eliminable = ELIMINATE.ilist_append(p->i); 
  }
  IRSYMS.set_head(rsyms);
  IRSYMS.zap_ilist();
  return eliminable;
}


Plist PredElim::resolve_on_symbol(int symbol, Topform c1, Topform c2) {
  PlistContainer RESOLVENTS;
  ClauseidContainer CI;

  Plist resolvents = NULL;
  Literals l1, l2;
  for (l1 = c1->literals; l1; l1 = l1->next)
    for (l2 = c2->literals; l2; l2 = l2->next) {
      if (l1->sign != l2->sign &&
	  SYMNUM(l1->atom) == symbol &&
	  SYMNUM(l2->atom) == symbol) {
	Topform res;
	if (c1->id == 0) {
	  CI.assign_clause_id(c1);
	  Ioutil::fwrite_clause(cout, c1, (int) Clause_print_format::CL_FORM_STD);
	}
	if (c2->id == 0) {
	  CI.assign_clause_id(c2);
	  Ioutil::fwrite_clause(cout, c2, (int) Clause_print_format::CL_FORM_STD);
	}
	res = Resolve::resolve3(c1, l1, c2, l2, true);
	if (res) {
	  if (LADRV_GLOBAIS_INST.Lit.tautology(res->literals))  ClauseMisc::delete_clause(res);
	  else {
	    resolvents = RESOLVENTS.plist_append(res);
	  }
	}
      }
    }
  return resolvents;
} 



Plist PredElim::gen_given(int symbol, Topform given, Plist usable) {
  Plist novo = NULL;
  PlistContainer NOVO;
  Plist p;
  for (p = usable; p; p = p->next) {
    Topform c =(Topform) p->v;
    Plist resolvents = resolve_on_symbol(symbol, given, c);
	novo = NOVO.plist_cat(resolvents);
  }
  return novo;
}


bool PredElim::subsumed_by_member(Topform c, Plist p) {
  if (p == NULL)
    return false;
  else if (Subsume::subsumes((Topform)p->v, c))
    return true;
  else
    return subsumed_by_member(c, p->next);
}


Plist PredElim::incorporate_new_clauses(Plist sos, Plist novo, bool echo)
{
  Plist p;
  PlistContainer NOVO;
  PlistContainer SOS;
  SOS.set_head(sos);
  for (p = novo; p; p = p->next) {
    if (!subsumed_by_member(Topform(p->v), sos)) {
      sos = SOS.plist_append(p->v);
      printf("Derived: ");
      cout<<"Derived: ";
      Ioutil::fwrite_clause(cout, (Topform)p->v, (int) Clause_print_format::CL_FORM_STD);
    }
  }
  NOVO.set_head(novo);
  NOVO.zap_plist();
  return sos;
} 


Plist PredElim::elim_relation(int symbol, Plist sos, Clist disabled, bool echo) {
  /* this does a naive given-clause loop */
  Plist usable = NULL;
  PlistContainer USABLE,WITHOUT, SOS;
  ClistContainer CLIST;
  while (sos) {
    Plist novo;
    Topform given =(Topform) sos->v;
    SOS.set_head(sos);
    sos = SOS.plist_pop();
    usable = USABLE.plist_append(given);
#ifdef GEN_DEBUG
	cout<<endl<<"given: "; f_clause(given);
#endif
    novo = gen_given(symbol, given, usable);
    sos = incorporate_new_clauses(sos, novo, echo);
  }
  /* partition usable into clauses with and without symbol */
  {
    Plist without = NULL;
    Plist p;
	
    for (p = usable; p; p = p->next) {
      if (rsym_occurrences(symbol, (Topform)p->v) > 0) {
		CLIST.clist_append((Topform)p->v, disabled);
	  }
      else
		without = WITHOUT.plist_append(p->v);
    }
    USABLE.zap_plist();
    return without;
  }
} 


void PredElim::predicate_elimination(Clist clauses, Clist disabled, bool echo)
{
  PlistContainer PAUX;
  
  SymbolContainer S;
  ClistContainer CL;
  ClausesContainer CC;
  Plist clauses2 = CL.prepend_clist_to_plist(NULL, clauses);
  bool equality = CC.equality_in_clauses(clauses2);  /* eq => different method */
  Ilist syms = eliminable_relations(clauses2, equality);

  if (syms == NULL) {
	PAUX.set_head(clauses2);  
    PAUX.zap_plist();
    if (echo)
		cout<<endl<<"No predicates eliminated."<<endl;
  }
  else {
    CL.clist_remove_all_clauses(clauses);
    
    while (syms) {
      /* use first symbol, discard rest, get new list */
      if (echo)
		cout<<endl<<"Eliminating "<<S.sn_to_str(syms->i)<<"/"<<S.sn_to_arity(syms->i)<<endl;
  
      clauses2 = elim_relation(syms->i, clauses2, disabled, echo);
      IlistContainer IAUX(syms);
	  IAUX.zap_ilist();
      syms = eliminable_relations(clauses2, equality);
    }
    CL.clist_append_plist(clauses, clauses2);
    PAUX.set_head(clauses2);
	PAUX.zap_plist();
  }
} 
