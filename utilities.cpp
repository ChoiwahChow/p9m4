#include "./ladr/fatal.h"
#include "./ladr/ladrvglobais.h"
#include "./ladr/di_tree.h"

#include "utilities.h"
#include <iomanip>

void Utilities::print_memory_stats(ostream &fp) {
  StrbufContainer SB;
  ParseContainer P;
  HashtabContainer H;
  TermContainer T;
  AttributeContainer A;
  PlistContainer PL;
  JustContainer J;
  FormulaContainer F;
  TopformContainer TF;
  ClistContainer CL;
  UnifyContainer U;
  BtmContainer BTM;
  BtuContainer BTU;
  FpaIndexContainer FPA;
  FpalistContainer FPAL;
  DiscrimContainer D;
  DiscrimBContainer DB;
  DiscrimWContainer DW;
  FlattermContainer FT;
  MindexContainer MI;
  LindexContainer LI;
  ClashContainer CLA;
  Di_treeContainer DIT;
  
  
  fp<<endl<<"Megs malloced by palloc(): "<< Memory::megs_malloced()<<endl;
  SB.fprint_strbuf_mem(fp, true);
  P.fprint_parse_mem(fp,false);
  H.fprint_hash_mem(fp, false);
  T.fprint_term_mem(fp, false);
  A.fprint_attrib_mem(fp, true);
  LADR_GLOBAL_G_LIST.fprint_glist_mem(fp, false);

  J.fprint_just_mem(fp, false);
  F.fprint_formula_mem(fp, false);
  TF.fprint_topform_mem(fp, false);
  CL.fprint_clist_mem(fp, false);

  U.fprint_unify_mem(fp, false);
  BTM.fprint_btm_mem(fp, false);
  BTU.fprint_btu_mem(fp, false);

  FPA.fprint_fpa_mem(fp, false);
  FPAL.fprint_fpalist_mem(fp, false);
  D.fprint_discrim_mem(fp, false);
  DB.fprint_discrimb_mem(fp, false);
  DW.fprint_discrimw_mem(fp, false);
  FT.fprint_flatterm_mem(fp, false);
  MI.fprint_mindex_mem(fp, false);
  LI.fprint_lindex_mem(fp, false);
  CLA.fprint_clash_mem(fp, false);
  DIT.fprint_di_tree_mem(fp, false);
  LADR_GLOBAL_AVL_TREE.fprint_avltree_mem(fp, false);
  Memory::memory_report(fp);
}




void Utilities::fsym_collect(Ilist *table, Term t, int depth) {
  IlistContainer I;
  if (VARIABLE(t))   return;
  else {
    int i;
	I.set_head(table[SYMNUM(t)]);
    table[SYMNUM(t)] = I.ilist_prepend(depth);
    for (i = 0; i < ARITY(t); i++)
      fsym_collect(table, ARG(t,i), depth+1);
  }
} 


bool  Utilities::inverse_axiom(Topform c, int *f2, int *f1, int *f0) {

  TermContainer T;
  if (LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals) != 1 || !LADRV_GLOBAIS_INST.Lit.pos_eq(c->literals))    return false;
  else {
    Term alpha = ARG(c->literals->atom,0);
    Term beta  = ARG(c->literals->atom,1);
    if (CONSTANT(alpha))
      { Term t = alpha; alpha = beta; beta = t; }
    if (!CONSTANT(beta) || ARITY(alpha) != 2) return false;
    else {
      Term a0 = ARG(alpha,0);
      Term a1 = ARG(alpha,1);
      if (VARIABLE(a0))
		{ Term t = a0; a0 = a1; a1 = t; }
      if (!VARIABLE(a1) || ARITY(a0) != 1)	return false;
      else if (!T.term_ident(ARG(a0,0),a1))	return false;
      else {
		*f2 = SYMNUM(alpha);
		*f1 = SYMNUM(a0);
		*f0 = SYMNUM(beta);
		return true;
      }
    }
  }
}  



void Utilities::fsym_report(Ilist fsyms, Plist clauses) {
  SymbolContainer S;
  Ilist *table = (Ilist *) calloc(S.greatest_symnum()+1  , sizeof(Ilist));
  Plist p;

  for (p = clauses; p; p = p->next) {
    Topform c = (Topform) p->v;
    Literals lit;
    int f0, f1, f2;
    for (lit = c->literals; lit; lit = lit->next)
      fsym_collect(table, lit->atom, 0);
    if (inverse_axiom(c, &f2, &f1, &f0))
      { cout<<"Inverse axiom: "; Ioutil::f_clause(c); }
  }

  {
    Ilist a;
    cout<<"Symbols:"<<endl;
	for (a = fsyms; a; a = a->next) {
      Ilist b;
      int n = 0;
      cout<<"  Symbol "<<setw(22)<<S.sn_to_str(a->i)<<"/"<<S.sn_to_arity(a->i)<<":";
	  for (b = table[a->i]; b; b = b->next) {
		cout<<" "<<b->i;
		n += b->i;
      }
	  cout << "    "<<n<<endl;
    }
  }
  free(table);
} 


bool Utilities::inverse_order(Clist clauses) {
  Clist_pos p;
  Ilist binary = NULL;
  Ilist unary = NULL;
  IlistContainer IB;
  IlistContainer IU;
  SymbolContainer S;
  bool change;
    
  for (p = clauses->first; p; p = p->next) {
    int f2, f1, f0;
    if (inverse_axiom(p->c, &f2, &f1, &f0)) {
      binary = IB.ilist_append(f2);
      unary = IU.ilist_append(f1);
    }
  }
  change = (unary != NULL || binary != NULL);
  S.lex_insert_after_initial_constants(unary);
  S.lex_insert_after_initial_constants(binary);
  IU.zap_ilist();
  IB.zap_ilist();
  return change;
} 

void Utilities::p_sym_list(Ilist syms) {
  Ilist p;
  SymbolContainer S;
  cout<<"[";
  for (p = syms; p; p = p->next) {
    cout << S.sn_to_str(p->i)<<"/"<<S.sn_to_arity(p->i)<< (p->next ? "," : "");
  }
  cout<<"]";
} 





void Utilities::symbol_order(Clist usable, Clist sos, Clist demods, bool echo) {
  Ilist fsyms, rsyms;
  I2list fsyms_multiset, rsyms_multiset;
  Plist nonneg;

  PlistContainer P;
  IlistContainer I;
  I2listContainer I2;
  TopformContainer TF;
  ClausesContainer C;
  SymbolContainer S;
  ClistContainer CL;
  
  
  Plist all = NULL;
  all = CL.prepend_clist_to_plist(all,usable);
  all = CL.prepend_clist_to_plist(all,sos);
  all = CL.prepend_clist_to_plist(all,demods);

  fsyms = TF.fsym_set_in_topforms(all);
  rsyms = TF.rsym_set_in_topforms(all);

  // fsym_report(fsyms, all);

  nonneg = C.nonneg_clauses(all);
  P.set_head(all);
  P.zap_plist();

  rsyms_multiset = NULL;
  fsyms_multiset = NULL;
  TF.gather_symbols_in_topforms(nonneg, &rsyms_multiset, &fsyms_multiset);

  S.lex_order(fsyms, rsyms, fsyms_multiset, rsyms_multiset,S.lex_compare_arity_0213);
              

  if (echo && S.exists_preliminary_precedence(Symbol_Type::FUNCTION_SYMBOL))  {
    // print any symbols missing from the lex command
    Ilist p, missing_fsyms;
    missing_fsyms = S.not_in_preliminary_precedence(fsyms, Symbol_Type::FUNCTION_SYMBOL);
    // p_sym_list(fsyms); printf(" (fsyms)\n");
    // p_sym_list(missing_fsyms); printf(" (missing_fsyms)\n");
    if (missing_fsyms) {
      cerr<<"WARNING, function symbols not in function_order (lex) command:";
	  cout<<"WARNING, function symbols not in function_order (lex) command:";
      for (p = missing_fsyms; p; p = p->next) {
		cerr<<" "<<S.sn_to_str(p->i)<< (p->next ? "," : "\n");
		cout<<" "<<S.sn_to_str(p->i)<< (p->next ? "," : "\n");
      }
    }
    I.set_head(missing_fsyms);
	I.zap_ilist();
  }
  if (echo && S.exists_preliminary_precedence(Symbol_Type::PREDICATE_SYMBOL))  {
    // print any symbols missing from the lex command
    Ilist p, missing_rsyms;
    missing_rsyms = S.not_in_preliminary_precedence(rsyms, Symbol_Type::PREDICATE_SYMBOL);
    // p_sym_list(rsyms); printf(" (rsyms)\n");
    // p_sym_list(missing_rsyms); printf(" (missing_rsyms)\n");
    if (missing_rsyms) {
      cerr<<"WARNING, predicate symbols not in predicate_order command:";
	  cout<<"WARNING, predicate symbols not in predicate_order command:";
	  for (p = missing_rsyms; p; p = p->next) {
		cerr<<" "<<S.sn_to_str(p->i)<< (p->next ? "," : "\n");
		cout<<" "<<S.sn_to_str(p->i)<< (p->next ? "," : "\n");
      }
    }
    I.set_head(missing_rsyms);
	I.zap_ilist();
  }

  LADR_GLOBAL_FEATURES.init_features(fsyms, rsyms);  // feature-vector subsumption for nonunits
  
  I.set_head(fsyms);
  I.zap_ilist();
  
  I.set_head(rsyms);
  I.zap_ilist();
  
  I2.set_head(fsyms_multiset);
  I2.zap_i2list();
  I2.set_head(rsyms_multiset);
  I2.zap_i2list();
}  /* symbol_order */



Ilist Utilities::unary_symbols(Ilist a) {
  SymbolContainer S;
  Ilist p=NULL;
  IlistContainer P;
   
  for (p = a; p; p = p->next) {
    if (S.sn_to_arity(p->i) == 1)
      p = P.ilist_append(p->i);
  }
  return p;
}


void Utilities::auto_kbo_weights(Clist usable, Clist sos) {
  Plist clauses = NULL;
  PlistContainer P;
  TopformContainer TF;
  IlistContainer I;
  SymbolContainer S;
  ClistContainer C;
  Ilist fsyms, unaries;

  clauses = C.prepend_clist_to_plist(clauses,usable);  /* shallow */
  clauses = C.prepend_clist_to_plist(clauses,sos);     /* shallow */

  fsyms = TF.fsym_set_in_topforms(clauses);
  unaries = unary_symbols(fsyms);
  I.set_head(unaries);	
  if (I.ilist_count() == 1) {
    int symnum = unaries->i;
    if (!S.exists_preliminary_precedence(Symbol_Type::FUNCTION_SYMBOL) || S.has_greatest_precedence(symnum)) {
      S.set_kb_weight(symnum, 0);
      if (!S.has_greatest_precedence(symnum)) S.assign_greatest_precedence(symnum);
		cout<<endl<<"%% Assigning unary symbol "<<S.sn_to_str(symnum)<<" kb_weight 0 and highest precedence ("<<S.sn_to_lex_val(symnum)<<")."<<endl;
    }
  }
  I.set_head(unaries);
  I.zap_ilist();
  I.set_head(fsyms);
  I.zap_ilist();
  P.zap_plist();  /* shallow */
} 



int Utilities::neg_pos_depth_diff(Topform c) {
 
  TermContainer T;
  Literals lit;
  int max_pos = 0;
  int max_neg = 0;
  for (lit = c->literals; lit; lit = lit->next) {
    int depth = T.term_depth(lit->atom);
    if (lit->sign)
      max_pos = IMAX(max_pos, depth);
    else
      max_neg = IMAX(max_neg, depth);
  }
  return max_neg - max_pos;
} 



int Utilities::neg_pos_wt_diff(Topform c) {
  TermContainer T;;
  Literals lit;
  int max_pos = 0;
  int max_neg = 0;
  for (lit = c->literals; lit; lit = lit->next) {
    int wt = T.symbol_count(lit->atom);
    if (lit->sign)
      max_pos = IMAX(max_pos, wt);
    else
      max_neg = IMAX(max_neg, wt);
  }
  return max_neg - max_pos;
} 


int Utilities::neg_pos_depth_difference(Plist sos) {

  TermContainer T;
  
  Plist p;
  int neg = 0;
  int pos = 0;
  for (p = sos; p; p = p->next) {
    Topform c = (Topform) p->v;
    if (LADRV_GLOBAIS_INST.Lit.mixed_clause(c->literals)) {
      Literals lit;
      int max_pos = 0;
      int max_neg = 0;
      for (lit = c->literals; lit; lit = lit->next) {
	int depth = T.term_depth(lit->atom);
	if (lit->sign)
	  max_pos = IMAX(max_pos, depth);
	else
	  max_neg = IMAX(max_neg, depth);
      }
      neg += max_neg;
      pos += max_pos;
      // printf("max_neg=%d, max_pos=%d, ", max_neg, max_pos); f_clause(c);
    }
  }
  // printf("neg=%d, pos=%d\n", neg, pos);
  return neg - pos;
}



void Utilities::structure_of_clauses(Clist clauses) {
 
  int num_pos = 0;
  int pos_wt = 0;
  int pos_depth = 0;

  int neg_wt = 0;
  int neg_depth = 0;
  int num_neg = 0;

  int mix_wt = 0;
  int mix_depth = 0;
  int num_mix = 0;

  int mix_wt_diff = 0;
  int mix_depth_diff = 0;

  int num_nonhorn = 0;
  int nonhorn = 0;

  int num_nonunit = 0;
  int nonunit = 0;

  Clist_pos p;
  for (p = clauses->first; p; p = p->next) {
    Topform c = p->c;
    int wt = LADRV_GLOBAIS_INST.Lit.clause_symbol_count(c->literals);
    int depth = LADRV_GLOBAIS_INST.Lit.clause_depth(c->literals);
    if (LADRV_GLOBAIS_INST.Lit.positive_clause(c->literals)) {
      num_pos++;
      pos_wt += wt;
      pos_depth += depth;
    }
    else if (LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals)) {
      num_neg++;
      neg_wt += wt;
      neg_depth += depth;
    }
    else {
      num_mix++;
      mix_wt += wt;
      mix_depth += depth;
      mix_wt_diff += neg_pos_wt_diff(c);
      mix_depth_diff += neg_pos_depth_diff(c);
    }

    if (!LADRV_GLOBAIS_INST.Lit.horn_clause(c->literals)) {
      num_nonhorn++;
      nonhorn += LADRV_GLOBAIS_INST.Lit.positive_literals(c->literals) - 1;
    }

    if (!LADRV_GLOBAIS_INST.Lit.unit_clause(c->literals)) {
      num_nonunit++;
      nonunit += LADRV_GLOBAIS_INST.Lit.number_of_literals(c->literals);
    }

  }
  {
    double pw = num_pos == 0 ? 0 : pos_wt / (double) num_pos;
    double pd = num_pos == 0 ? 0 : pos_depth / (double) num_pos;

    double nw = num_neg == 0 ? 0 : neg_wt / (double) num_neg;
    double nd = num_neg == 0 ? 0 : neg_depth / (double) num_neg;

    double mw = num_mix == 0 ? 0 : mix_wt / (double) num_mix;
    double md = num_mix == 0 ? 0 : mix_depth / (double) num_mix;

    double mwd = num_mix == 0 ? 0 : mix_wt_diff / (double) num_mix;
    double mdd = num_mix == 0 ? 0 : mix_depth_diff / (double) num_mix;

    double nh = num_nonhorn == 0 ? 0 : nonhorn / (double) num_nonhorn;
    double nu = num_nonunit == 0 ? 0 : nonunit / (double) num_nonunit;

	cout<<endl<<"%% Struc";
	cout<<"  (N  "<<setw(2)<<num_pos<<" "<<setw(2)<<num_neg<<" "<<setw(2)<<num_mix<<")";
	cout<<"  (WT "<<setw(5)<<pw<<" "<<setw(5)<<nw<<" "<<setw(5)<<mw<<setw(5)<<mwd<<")";
	cout<<"  (DP "<<setw(5)<<pd<<" "<<setw(5)<<nd<<" "<<setw(5)<<md<<setw(5)<<mdd<<")";
	cout<<"  (NU "<<setw(5)<<num_nonhorn<<" "<<setw(5)<<nh<<" "<<setw(5)<<num_nonunit<<setw(5)<<nu<<")";
	cout<<endl;
	
  }
}


void Utilities::p_term_list(Plist terms) {
   ParseContainer P;
   Plist p;
  
  cout<<"[";
  for (p = terms; p; p = p->next) {
    P.fwrite_term(cout, (Term)p->v);
    cout<<(p->next ? "," : "");
  }
  cout<<"]";
}


int Utilities::plist_size_of_diff(Plist a, Plist b) {
  PlistContainer P;
  
  if (a == NULL)    return 0;
  else {
    int n = plist_size_of_diff(a->next, b);
    P.set_head(b);
	return (!P.plist_member(a->v) ? n+1 : n);
  }
}  

void Utilities::structure_of_variables(Clist clauses) {

  TermContainer T;
  PlistContainer P;
  int n = 0;
  int np = 0;
  int nn = 0;

  Clist_pos p;
  for (p = clauses->first; p; p = p->next) {
    Topform c = (Topform)p->c;
    if (LADRV_GLOBAIS_INST.Lit.mixed_clause(c->literals)) {
      Plist pvars = NULL;
      Plist nvars = NULL;
      Literals lit;
      for (lit = c->literals; lit; lit = lit->next) {
		if (lit->sign)	  pvars = T.set_of_vars(lit->atom, pvars);
		else nvars = T.set_of_vars(lit->atom, nvars);
      }
      cout<<endl;
	  
      Ioutil::f_clause(c);
	  cout<<" nvars ("	<<plist_size_of_diff(nvars, pvars)<<"): ";
      p_term_list(nvars); 
      cout<<" pvars ("	<<plist_size_of_diff(pvars, nvars)<<"): ";
      p_term_list(pvars); 
      n++;
      P.set_head(pvars);
	  np += P.plist_count();
	  P.zap_plist();
	  
	  P.set_head(nvars);
      nn += P.plist_count();
      P.zap_plist();
    }
  }
  cout<<endl<<"nn="<<nn<<", np="<<np<<", n="<<n<<endl;
} 


OrderType Utilities::clause_compare_m4(Topform a, Topform b)
{
 
  /* pos < neg < mixed */
  bool a_pos = LADRV_GLOBAIS_INST.Lit.positive_clause(a->literals);
  bool b_pos = LADRV_GLOBAIS_INST.Lit.positive_clause(b->literals);
  bool a_neg = LADRV_GLOBAIS_INST.Lit.negative_clause(a->literals);
  bool b_neg = LADRV_GLOBAIS_INST.Lit.negative_clause(b->literals);
  
  if (a_pos && !b_pos)    		return OrderType::LESS_THAN;
  else if (!a_pos && b_pos)    	return OrderType::GREATER_THAN;
  else if (a_neg && !b_neg)    	return OrderType::LESS_THAN;
  else if (!a_neg && b_neg)	    return OrderType::GREATER_THAN;
  else {
    /* now both pos, both neg, or both mixed */
    /* fewer symbols < more symbols */
    int na = LADRV_GLOBAIS_INST.Lit.clause_symbol_count(a->literals);
    int nb = LADRV_GLOBAIS_INST.Lit.clause_symbol_count(b->literals);
    if (na > nb)		    return OrderType::GREATER_THAN;
    else if (na < nb)   	return OrderType::LESS_THAN;
    else {
      /* fewer literals < more literals */
      na = LADRV_GLOBAIS_INST.Lit.number_of_literals(a->literals);
      na = LADRV_GLOBAIS_INST.Lit.number_of_literals(b->literals);
      if (na > nb)		return OrderType::GREATER_THAN;
      else if (na < nb)	return OrderType::LESS_THAN;
      else {
		/* shallower < deeper */
		na = LADRV_GLOBAIS_INST.Lit.clause_depth(a->literals);
		na = LADRV_GLOBAIS_INST.Lit.clause_depth(b->literals);
		if (na > nb)	  return OrderType::GREATER_THAN;
		else if (na < nb) return OrderType::LESS_THAN;
		else return OrderType::SAME_AS;  /* should we go further? */
     }
    }
  }
} 

int Utilities::bogo_ticks(void) {
  return Memory::mega_mem_calls();
}  



Topform Utilities::next_negative_clause_3(Clist_pos *ap, Clist_pos *bp, Clist_pos *cp) {

  TopformContainer TF;
  Clist_pos a = *ap;
  Clist_pos b = *bp;
  Clist_pos c = *cp;

  Clist_pos x;
  Topform next_neg;
  
  while (a && LADRV_GLOBAIS_INST.Lit.negative_clause(a->c->literals)) a = a->next;
  while (b && !LADRV_GLOBAIS_INST.Lit.negative_clause(b->c->literals)) b = b->next;
  while (c && !TF.negative_clause_possibly_compressed(c->c)) c = c->next;

  if (!a) {
    if (!b)   x = c;  /* may be NULL */
    else if (!c)  x = b;
    else     x = (b->c->id < c->c->id ? b : c);
  }
  else if (!b) {
    if   (!c) x = a;
    else x = (a->c->id < c->c->id ? a : c);
  }
  else if (!c)    x = (a->c->id < b->c->id ? a : b);
  else if (a->c->id < b->c->id)   x = (a->c->id < c->c->id ? a : c);
  else x = (b->c->id < c->c->id ? b : c);

  /* Advance position of the winner. */

  if (x == NULL)  next_neg = NULL;
  else {
    next_neg = x->c;
    /* Advance position of the winner. */
    if (x == a)  a = a->next;
    else if (x == b)  b = b->next;
    else c = c->next;
  }

  /* Restore pointers for caller. */

  *ap = a;
  *bp = b;
  *cp = c;
  return next_neg;
}  /* next_negative_clause_3 */


Topform Utilities::first_negative_clause(Plist proof) {

  Plist p;
  for (p = proof; p; p = p->next) {
    Topform c =(Topform) p->v;
    if (!c->is_formula && LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals))
      return c;
  }
  return NULL;
} 


Plist Utilities::neg_clauses_and_descendants(Plist proof,Clist a_list, Clist b_list, Clist c_list) {
  PlistContainer P;
  JustContainer J;
  ClauseidContainer CI;

  Plist p;
  
  Topform first_neg = first_negative_clause(proof);
  Plist descendents = P.plist_prepend(first_neg);
  

  /* Get all descendents of first_neg that appear in a, b, or c. */
  
  Clist_pos a = a_list->first;
  Clist_pos b = b_list->first;
  Clist_pos c = c_list->first;

  Topform next = next_negative_clause_3(&a, &b, &c);
  while (next) {
    Topform neg_parent = J.first_negative_parent(next);
    if (neg_parent && CI.clause_plist_member(descendents, neg_parent, false))
      descendents = CI.insert_clause_into_plist(descendents, next, false);
    next = next_negative_clause_3(&a, &b, &c);
  }

  /* The last 2 negative clauses might not be in a, b, or c,
     so make sure they get put into the list of descendants.
  */

  for (p = proof; p; p = p->next) {
    Topform c = (Topform) p->v;
    if (!c->is_formula && LADRV_GLOBAIS_INST.Lit.negative_clause(c->literals))
      /* If already there, it will not be inserted. */
      descendents = CI.insert_clause_into_plist(descendents, c, false);
  }
  P.set_head(descendents);
  descendents = P.reverse_plist();  /* make it increasing */

#if 1
  {
    int n = 0;
    cout<<endl<<"%% *Preparing to disable descendents of clause "<<first_neg->id<<":";
	for (p = descendents; p; p = p->next) {
	  cout<<" "<< ((Topform) p->v)->id;
      if (++n % 20 == 0) cout<<endl;
    }
    cout<<endl;
	  }
#endif

  return descendents;
} 


Plist Utilities::neg_descendants(Topform top_neg, Clist a_list, Clist b_list, Clist c_list) {
  PlistContainer P;
  ClistContainer C;
  ClauseidContainer CI;
  JustContainer J;
  Plist descendants = P.plist_prepend(top_neg);
  Clist_pos a, b, c;
  Topform next;

  C.sort_clist_by_id(c_list);

  /* Get all descendants of top_neg that appear in a, b, or c. */
  
  a = a_list->first;
  b = b_list->first;
  c = c_list->first;

  next = next_negative_clause_3(&a, &b, &c);
  while (next) {
    Topform neg_parent = J.first_negative_parent(next);
    if (neg_parent && CI.clause_plist_member(descendants, neg_parent, false))
      descendants = CI.insert_clause_into_plist(descendants, next, false);
    next = next_negative_clause_3(&a, &b, &c);
  }
  PlistContainer D;
  D.set_head(descendants);
  descendants = D.reverse_plist();  /* make it increasing */
  return descendants;
} 



void Utilities::check_constant_sharing(Plist clauses) {
  
  SymbolContainer S;
  Plist a, b;
  IlistContainer I, Ir, Is;
  
  Ilist shared = NULL;
  for (a = clauses; a; a = a->next) {
    Topform aa =(Topform) a->v;
    Ilist ca = LADRV_GLOBAIS_INST.Lit.constants_in_clause(aa->literals);
    for (b = a->next; b; b = b->next) {
      Topform bb = (Topform) b->v;
      Ilist cb = LADRV_GLOBAIS_INST.Lit.constants_in_clause(bb->literals);
      Ilist sh2=I.ilist_intersect(ca,cb);
      Ilist sh3=I.ilist_cat(sh2,shared);
      shared=I.ilist_set(sh3);
      
      I.set_head(sh3);
      I.zap_ilist();
      I.set_head(cb);
      I.zap_ilist();
      
    }
    
    Ir.set_head(ca);
	Ir.zap_ilist();
  }
  
  if (shared) {
    Ilist p;
    fatal::bell(cerr);
    cerr<<endl<<"WARNING: denials share constants (see output)."<<endl<<endl;
	cout<<endl<<"WARNING, because some of the denials share constants,"<<endl;
    cout<<"some of the denials or their descendents may be subsumed,"<<endl;
	cout<<"preventin the target number of proofs form being found."<<endl;   
	cout<<"The shared constants are:";   
    Ir.set_head(shared);
	for (p = shared; p; p = p->next)
      cout<<" "<<S.sn_to_str(p->i)<<(p->next ? "," : ".\n")<<endl;
		
    Is.zap_ilist();
  }
} 
