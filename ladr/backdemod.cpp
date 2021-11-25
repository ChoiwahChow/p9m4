#include "clauseid.h"
#include "backdemod.h"

void Backdemod::index_term_back_demod(Term t, Mindex idx, Indexop op) {
  MindexContainer M;
  if (!VARIABLE(t)) {
    int i;
    M.mindex_update(idx, t, op);
    for (i = 0; i < ARITY(t); i++)
      index_term_back_demod(ARG(t,i), idx, op);
  }
} 

void Backdemod::index_clause_back_demod(Topform c, Mindex idx, Indexop op) {
  Literals lit;
  for (lit = c->literals; lit != NULL; lit = lit->next) {
    Term atom = lit->atom;
    int i;
    for (i = 0; i < ARITY(atom); i++) {
      index_term_back_demod(ARG(atom,i), idx, op);
    }
  }
}


bool Backdemod::rewritable_term(Term alpha, Term t, Context subst) {
  UnifyContainer U;
  Trail tr = NULL;
  if (U.match(alpha, subst, t, &tr)) {
    U.undo_subst(tr);
    return true;
  }
  else {
    int i;
    bool ok = false;
    for (i = 0; i < ARITY(t) && !ok; i++)
      ok = rewritable_term(alpha, ARG(t,i), subst);
    return ok;
  }
}


bool Backdemod::rewritable_clause(Topform demod, Topform c) {
  UnifyContainer U;
  Term alpha = ARG(demod->literals->atom,0);
  Literals lit;
  bool ok = false;
  Context subst = U.get_context();
  for (lit = c->literals; lit != NULL && !ok; lit = lit->next) {
    Term atom = lit->atom;
    int i;
    for (i = 0; i < ARITY(atom) && !ok; i++)
      ok = rewritable_term(alpha, ARG(atom,i), subst);
  }
  U.free_context(subst);
  return ok;
} 

Plist Backdemod::back_demod_linear(Topform demod, Clist lst, Plist rewritables) {
  ClauseidContainer CI;
  Clist_pos p;

  for (p = lst->first; p != NULL; p = p->next) {
    Topform c = p->c;
    if (c != demod && rewritable_clause(demod, c)) {
      rewritables = CI.insert_clause_into_plist(rewritables, c, false);
    }
  }
  return(rewritables);
}  /* back_demod_linear */

bool Backdemod::lex_rewritable(Term subject, Context subst, Term gen,bool lex_order_vars) {
  /* Apply subst to gen, and check if it's less than subject. */
  UnifyContainer U;
  TermContainer  T;
  Term instance = U.apply(gen, subst);
  bool result = TermOrder::term_greater(subject, instance, lex_order_vars);
  T.zap_term(instance);
  return result;
}


Plist Backdemod::back_demod_indexed(Topform demod, int type, Mindex idx,bool lex_order_vars) {
  UnifyContainer U;
  ClauseidContainer CI;
  MindexContainer M;
  Term atom = demod->literals->atom;
  Term alpha = ARG(atom,0);
  Term beta  = ARG(atom,1);
  Plist rewritables = NULL;
  Context subst = U.get_context();

  if (type == (int) Demod_Type::ORIENTED) {
    Mindex_pos pos;
    Term t = M.mindex_retrieve_first(alpha,idx,Querytype::INSTANCE,subst,NULL,false,&pos);
    while (t != NULL) {
      Topform c =(Topform) t->container;
      if (c != demod) {          /* in case demod is already in idx */
		rewritables = CI.insert_clause_into_plist(rewritables, c, false);
      }
      t = M.mindex_retrieve_next(pos);
    }
  }
  else {
    if (type == (int) Demod_Type::LEX_DEP_LR || type == (int) Demod_Type::LEX_DEP_BOTH) {
      Mindex_pos pos;
      Term t;
      /* Find clauses that can be rewritten left-to-right. */
      t = M.mindex_retrieve_first(alpha,idx,Querytype::INSTANCE,subst,NULL,false,&pos);
      while (t != NULL) {
		Topform c = (Topform) t->container;
		if (c != demod && lex_rewritable(t, subst, beta, lex_order_vars)) {
		rewritables = CI.insert_clause_into_plist(rewritables, c, false);
	 }
	 t = M.mindex_retrieve_next(pos);
	}
   }

    if (type == (int) Demod_Type::LEX_DEP_RL || type == (int) Demod_Type::LEX_DEP_BOTH) {
      Mindex_pos pos;
      Term t;
      /* Find clauses that can be rewritten right-to-left. */
      t = M.mindex_retrieve_first(beta,idx,Querytype::INSTANCE,subst,NULL,false,&pos);
      while (t != NULL) {
		Topform c =(Topform) t->container;
		if (c != demod && lex_rewritable(t, subst, alpha, lex_order_vars)) {
			rewritables = CI.insert_clause_into_plist(rewritables, c, false);
	  }
	  t = M.mindex_retrieve_next(pos);
      }
    }
  }
  U.free_context(subst);
  return rewritables;
} 
