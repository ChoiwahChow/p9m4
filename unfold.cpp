#include "unfold.h"


Tsymdata Unfold::symdata;
						
				
				
void Unfold::trace_dependents(int i, Tsymdata *table) {
  if (table[i].status == (int)UnfoldCheckType::CHECKED)
    return;
  else if (table[i].status == (int)UnfoldCheckType::CHECKING) {
    table[i].status =(int)UnfoldCheckType::CYCLE;
    return;
  }
  else if (table[i].status == (int) UnfoldCheckType::CYCLE)
    return;
  else {
    Ilist p;
    bool ok = true;
    table[i].status = (int) UnfoldCheckType::CHECKING;
    for (p = table[i].dependents; p; p = p->next) {
      trace_dependents(p->i, table);
      if (table[p->i].status == (int)UnfoldCheckType::CYCLE)
	ok = false;
    }
    table[i].status = (ok ? (int)UnfoldCheckType::CHECKED : (int)UnfoldCheckType::CYCLE);
    return;
  }
} 	

Ilist Unfold::eliminate_cycles(Ilist symbols, Tsymdata *table) {
  IlistContainer IL;
  if (symbols == NULL)   return NULL;
  else {
    int i = symbols->i;
    symbols->next = eliminate_cycles(symbols->next, table);
    if (table[i].status == (int) UnfoldCheckType::CYCLE) {
      Ilist p = symbols->next;
      IL.free_ilist(symbols);
      IL.set_head(table[i].dependents);
	  IL.zap_ilist();
      table[i].dependents = NULL;
      return p;
    }
    else
      return symbols;
  }
} 	


bool Unfold::sym_less_or_equal(int s1, int s2, Tsymdata *table){
  if (s1 == s2)   return true;
  else {
    Ilist p;
    for (p = table[s2].dependents; p; p = p->next)
      if (sym_less_or_equal(s1, p->i, table))	return true;
      return false;
  }
}	


OrderType Unfold::compare_symbols(int s1, int s2, Tsymdata *table) {
  string str1, str2;
  SymbolContainer S;
  if (s1 == s2)
    return OrderType::SAME_AS;
  else if (sym_less_or_equal(s1, s2, table))
    return OrderType::LESS_THAN;
  else if (sym_less_or_equal(s2, s1, table))
    return OrderType::GREATER_THAN;
  else if (S.sn_to_arity(s1) < S.sn_to_arity(s2))
    return OrderType::LESS_THAN;
  else if (S.sn_to_arity(s2) < S.sn_to_arity(s1))
    return OrderType::GREATER_THAN;
  else {
    str1=S.sn_to_str(s1);
	str2=S.sn_to_str(s2);
	int i=str1.compare(str2);
	if (i<0)   return OrderType::LESS_THAN;
    else if (i>0)   return OrderType::GREATER_THAN;
    else   return OrderType::SAME_AS;
  }
}	

Ilist Unfold::insert_symbol(Ilist syms, int sym, Tsymdata *table) { 
 IlistContainer IL,IL1;
 if (syms == NULL)
    return IL.ilist_append(sym);
  else if (compare_symbols(sym, syms->i, table) == OrderType::GREATER_THAN) {
    syms->next = insert_symbol(syms->next, sym, table);
    return syms;
  }
  else  {
			 IL1.set_head(syms);
			 return IL1.ilist_prepend(sym);
		}
} 


Ilist Unfold::order_symbols(Ilist syms, Tsymdata *table) {
  Ilist novo = NULL;
 
  
  Ilist p;
  for (p = syms; p; p = p->next)
    novo = insert_symbol(novo, p->i, table);
  IlistContainer IL(syms);
  IL.zap_ilist();
  return novo;
} 


Ilist Unfold::eq_defs(Clist clauses, int arity_limit) {
  SymbolContainer S;
  Parautil Pu;
  TermContainer T;
  Ilist symbols = NULL;
  Ilist p;
  IlistContainer IL_DEPENDENTS,IL_FSYMS_IN_SET,IL_SYMBOLS;
  int size = S.greatest_symnum() + 1;
  int i;
  Tsymdata *table =(Tsymdata *) calloc(size, sizeof(Tsymdata));
  Clist_pos cp;
  for (cp = clauses->first; cp; cp = cp->next) {
    Topform c = (Topform) cp->c;
    int rc = Pu.equational_def(c);  /* 0=no; 1=LR, 2=RL */
    if (rc != 0) {
      Term alpha = ARG(c->literals->atom, (rc == 1 ? 0 : 1));
      Term beta  = ARG(c->literals->atom, (rc == 1 ? 1 : 0));
      if (ARITY(alpha) <= arity_limit && ARITY(beta) > 0) {
		int symbol = SYMNUM(alpha);
		IL_DEPENDENTS.set_head(table[symbol].dependents);
		IL_FSYMS_IN_SET.set_head(T.fsym_set_in_term(beta));
		table[symbol].dependents = IL_DEPENDENTS.ilist_cat(IL_FSYMS_IN_SET);
		table[symbol].alpha = alpha;
		table[symbol].beta = beta;
		IL_SYMBOLS.set_head(symbols);
		if (!IL_SYMBOLS.ilist_member(symbol)) symbols = IL_SYMBOLS.ilist_append(symbol);
      }
    }
  }

  // trace dependencies (in table)
  for (p = symbols; p; p = p->next)
    trace_dependents(p->i, table);

  // eliminate symbols involved in cycles
  symbols = eliminate_cycles(symbols, table);

  // partial-order -> total-order (by partial-order, arity, strcmp)
  symbols = order_symbols(symbols, table);

  for (i = 0; i < size; i++) {
    IL_DEPENDENTS.set_head(table[i].dependents);
	IL_DEPENDENTS.zap_ilist();
  }	
  free(table);
  return symbols;
} 


int Unfold::num_constant_symbols(Ilist p) {
  SymbolContainer S;
  if (p == NULL)  return 0;
  else   return (S.sn_to_arity(p->i) == 0 ? 1 : 0) + num_constant_symbols(p->next);
}



bool Unfold::constant_check(int symnum, Ilist symbols, Clist clauses, int constant_limit) {
  SymbolContainer S;
 
  TermContainer T;
  if (S.sn_to_arity(symnum) > 0)    return true;
  else if (num_constant_symbols(symbols) > constant_limit)   return false;
  else {
    /* ok if the constant occurs in a negative clause */
    Clist_pos cp;
    for (cp = clauses->first; cp; cp = cp->next) {
      Topform c = (Topform)cp->c;
      if (LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals)) {
	  Literals lit;
	  for (lit = cp->c->literals; lit; lit = lit->next) {
	     if (T.symbol_in_term(symnum, lit->atom))  return true;
	  }
     }
    }
    return false;
  }
} 


void Unfold::unfold_eq_defs(Clist clauses, int arity_limit, int constant_limit, bool print) {
  SymbolContainer S;
 
  Ilist symbols = eq_defs(clauses, arity_limit);
  Ilist p;
  int n = 0;

  // Now we have a list of symbols that can be unfolded.  There can
  // be dependencies, but there are no cycles.  Dependents are earlier.
  // If we were always using LPO or RPO, we could simply give these
  // symbols highest precedence (in the same order).  However,
  // for KBO we want to be able to unfold when there are repeated
  // variables in the right side, e.g., g(x) = f(x,x), which does not
  // satisfy KBO.  Therefore, we do not control unfolding by setting
  // precedences and KBO weights.  Instead, we flag the symbol as "unfold"
  // and the routine that orients equalities checks for that special case.

  if (print) cout<<"Unfolding symbols:";
  for (p = symbols; p; p = p->next) {
    int i = p->i;
    if (constant_check(i, symbols, clauses, constant_limit)) {
      n++;
      // assign_greatest_precedence(i);   /* for LRPO */
      // set_kb_weight(SYMNUM(table[i].alpha), kbo_weight(table[i].beta) + 1);
      S.set_unfold_symbol(i);
      if (print) cout<<" "<<S.sn_to_str(i)<<"/"<<S.sn_to_arity(i);	
    }
  }
  if (print) cout<<(n > 0 ? "." : " (none).")<<endl;
  
  IlistContainer IL_SYMBOLS(symbols);
  IL_SYMBOLS.zap_ilist();
}


Ilist Unfold::remove_kb_wt_zero(Ilist syms) {
  SymbolContainer S;
  IlistContainer IL;
  if (syms == NULL)   return NULL;
  else {
    syms->next = remove_kb_wt_zero(syms->next);
    if (S.sn_to_kb_wt(syms->i) == 0) {
      Ilist next = syms->next;
      IL.free_ilist(syms);
      return next;
    }
    else return syms;
  }
} 


bool Unfold::fold_eq_defs(Clist clauses, bool kbo) {
  SymbolContainer S;
  
  Ilist symbols = eq_defs(clauses, INT_MAX);
  Ilist p;
  bool change;

  if (kbo)
    /* required for termination */
    symbols = remove_kb_wt_zero(symbols);

  
  cout<<"Folding symbols:";
  for (p = symbols; p; p = p->next)
    cout<<" "<<S.sn_to_str(p->i)<<"/"<<S.sn_to_arity(p->i);
  cout<<(symbols ? "." : " (none).")<<endl;	
  

  S.lex_insert_after_initial_constants(symbols);
  change = (symbols != NULL);
  IlistContainer SYMBOLS(symbols);
  SYMBOLS.zap_ilist();
  return change;
}


bool Unfold::one_unary_def(Clist a, Clist b) {
  IlistContainer IL_D1, IL_D2;
  Ilist d1 = eq_defs(a, 1);
  Ilist d2 = eq_defs(b, 1);
  IL_D1.set_head(d1);
  IL_D2.set_head(d2);
  bool rc = (IL_D1.ilist_count() + IL_D2.ilist_count() == 1);
  IL_D1.zap_ilist();
  IL_D2.zap_ilist();
  return rc;
} 
