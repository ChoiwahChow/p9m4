#include "ladrvglobais.h"
#include "parautil.h"
#include "fatal.h"
#include "termflag.h"
#include "clauses.h"
#include <iostream>




GlobalParautil::GlobalParautil() {
    Oriented_flag=-1;       /* termflag to mark oriented equality atoms */
    Renamable_flip_flag=-1;
}

GlobalParautil::~GlobalParautil() {

    
}








void Parautil::init_paramod(void)
{
  TermflagContainer TF;
  if (LADR_GLOBAL_PARAUTIL.Oriented_flag != -1 || LADR_GLOBAL_PARAUTIL.Renamable_flip_flag != -1)
    fatal::fatal_error("init_paramod, called more than once");
  LADR_GLOBAL_PARAUTIL.Renamable_flip_flag = TF.claim_term_flag();
  LADR_GLOBAL_PARAUTIL.Oriented_flag = TF.claim_term_flag();
}


void Parautil::mark_renamable_flip(Term atom) {
  TermflagContainer TF;
  if (LADR_GLOBAL_PARAUTIL.Renamable_flip_flag == -1)
    fatal::fatal_error("mark_renamable_flip, init_paramod() was not called");
  TF.term_flag_set(atom, LADR_GLOBAL_PARAUTIL.Renamable_flip_flag);
}

bool Parautil::renamable_flip_eq(Term atom) {
  TermflagContainer TF;
   if (LADR_GLOBAL_PARAUTIL.Renamable_flip_flag == -1)
    /* Nothing has ever been marked renamable_flip. */
    return false;
  else
    return TF.term_flag(atom, LADR_GLOBAL_PARAUTIL.Renamable_flip_flag);
}


bool Parautil::renamable_flip_eq_test(Term atom) {
  TermContainer T;
  TopformContainer TF;
  if (T.eq_term(atom))
    return false;
  else {
    bool result;
    Term a1 = T.copy_term(atom);
    Term a2 = T.copy_term(atom);
    Term tmp = ARG(a2,0);
    ARG(a2,0) = ARG(a2,1);
    ARG(a2,1) = tmp;
    TF.term_renumber_variables(a1, MAX_VARS);
    TF.term_renumber_variables(a2, MAX_VARS);
    result = T.term_ident(a1,a2);
    T.zap_term(a1);
    T.zap_term(a2);
    return result;
  }
}


void Parautil::mark_oriented_eq(Term atom)
{
  TermflagContainer TF;
  if (LADR_GLOBAL_PARAUTIL.Oriented_flag == -1)
    fatal::fatal_error("mark_oriented_eq, init_paramod() was not called");
  TF.term_flag_set(atom, LADR_GLOBAL_PARAUTIL.Oriented_flag);
}


bool Parautil:: oriented_eq(Term atom) {
  TermContainer T;
  TermflagContainer TF;
  if (!T.eq_term(atom))
    return false;
  else if (LADR_GLOBAL_PARAUTIL.Oriented_flag == -1)
    /* Nothing has ever been oriented. */
    return false;
  else
    return TF.term_flag(atom, LADR_GLOBAL_PARAUTIL.Oriented_flag);
} 

bool Parautil::same_term_structure(Term t1, Term t2) {
  if (VARIABLE(t1) || VARIABLE(t2))
    return VARIABLE(t1) && VARIABLE(t2);
  else if (SYMNUM(t1) != SYMNUM(t2))
    return false;
  else {
    int i;
    for (i = 0; i < ARITY(t1); i++)
      if (!same_term_structure(ARG(t1,i), ARG(t2,i)))
	return false;
    return true;
  }
} 


void Parautil::flip_eq(Term atom, int n) {
  JustContainer J;
  Topform c =(Topform) atom->container;
  Term t = ARG(atom,0);
  ARG(atom,0) = ARG(atom,1);
  ARG(atom,1) = t;
  c->justification = J.append_just(c->justification, J.flip_just(n));
}

void Parautil::orient_equalities(Topform c, bool allow_flips) {
  TermContainer T;
  ParseContainer P;
  TopformContainer TF;
  

  Literals lit;
  int i;
  for (lit = c->literals, i = 1; lit != NULL; lit = lit->next, i++) {
    Term atom = lit->atom;
    if (T.eq_term(atom)) {
      Term alpha = ARG(atom,0);
      Term beta  = ARG(atom,1);
      if (!T.term_ident(alpha, beta)) {
	OrderType ord = OrderType::NOT_COMPARABLE;
	/* Check if it is a pos_eq_unit that should be unfolded. */
	if (i == 1 && lit->next == NULL && lit->sign)
	  ord = unfold_order(alpha, beta);
	/* If not oriented, check the primary ordering (LPO, RPO, KBO). */
	if (ord == OrderType::NOT_COMPARABLE)
	  ord = TermOrder::term_order(alpha, beta);

	if (ord == OrderType::GREATER_THAN)
	  mark_oriented_eq(atom);
	else if (ord == OrderType::LESS_THAN) {
	  if (allow_flips) {
	    flip_eq(atom, i);
	    mark_oriented_eq(atom);
	  }
	  else {
#if 0  /* disable, because this happens regularly during uncompression */
	    cerr<< "WARNING: orient_equalities, backward eq1:";
	    P.fwrite_term_nl(cerr, atom);
#endif
	  }
	}
	else {
	  /* Not orientable by the primary ordering. */
	  /* We could call renamable_flip_eq_test, but we might need
	     a1 and a2 for the secondary test. */
	  Term a1 = T.copy_term(atom);
	  Term a2 = T.copy_term(atom);
	  Term tmp = ARG(a2,0);
	  ARG(a2,0) = ARG(a2,1);
	  ARG(a2,1) = tmp;
	  TF.term_renumber_variables(a1, MAX_VARS);
	  TF.term_renumber_variables(a2, MAX_VARS);
	  if (T.term_ident(a1,a2))
	    /* The renumbered flip is identical to the renumbered original. */
	    mark_renamable_flip(atom);  /* We won't para from both sides. */
	  else {
	    /* Flip if the right side is greater in the secondary ordering. */
	    ord = TermOrder::term_compare_vcp(a1, a2);
	    if (ord == OrderType::LESS_THAN) {
	      if (allow_flips)
		flip_eq(atom, i);
	      else {
				cerr<<"WARNING: orient_equalities, backward eq2:";
				P.fwrite_term_nl(cerr, atom);
	      }
	    }
	  }
	  T.zap_term(a1);
	  T.zap_term(a2);
	}
      }  /* not identical */
    }  /* eq_atom */
  }  /* foreach literal */
}  /* orient_equalities */


bool Parautil::eq_tautology(Topform c) {
  TermContainer T;
  Literals l1;
  for (l1 = c->literals; l1; l1 = l1->next) {
    Term a = l1->atom;
    if (l1->sign && T.eq_term(a) && T.term_ident(ARG(a,0), ARG(a,1)))
      return true;
  }
  return false;
}

Term Parautil::top_flip(Term a) {
  TermContainer T;
  Term b;
  if (ARITY(a) != 2)
    fatal::fatal_error("top_flip, arity != 2");
  b = T.get_rigid_term_like(a);
  ARG(b,0) = ARG(a,1);
  ARG(b,1) = ARG(a,0);
  b->private_flags = a->private_flags;
  return b;
} 

void Parautil::zap_top_flip(Term a) {
  TermContainer T;
  T.free_term(a);
} 

Literals Parautil::literal_flip(Literals a) {
  
  Literals b = LADRV_GLOBAIS_INST.Lit.get_literals();
  b->sign = a->sign;
  b->atom = top_flip(a->atom);
  return b;
} 

void Parautil::zap_literal_flip(Literals a) {
  TermContainer T;
  
  T.free_term(a->atom);
  LADRV_GLOBAIS_INST.Lit.free_literals(a);
} 

Topform Parautil::clause_with_new_constant(Topform c, Term arg, int new_constant_sn) {
  TermContainer T;
  
  TopformContainer TF;
  JustContainer J;
  ClausesContainer C;
  SymbolContainer S;
  Term atom = T.get_rigid_term(S.eq_sym(), 2);
  Literals lit = LADRV_GLOBAIS_INST.Lit.get_literals();
  Topform novo = TF.get_topform();
  lit->sign = true;
  lit->atom = atom;
  ARG(atom,0) = T.copy_term(arg);
  ARG(atom,1) = T.get_rigid_term_dangerously(new_constant_sn, 0);
  novo->literals = LADRV_GLOBAIS_INST.Lit.append_literal(novo->literals, lit);
  TF.upward_clause_links(novo);
  novo->justification = J.new_symbol_just(c);
  return novo;
}  

Topform Parautil::new_constant(Topform c, int new_sn) {
  TermContainer T;
  SymbolContainer S;
  
  if (!LADRV_GLOBAIS_INST.Lit.pos_eq_unit(c->literals))
    return NULL;
  else {
    Term a = ARG(c->literals->atom,0);
    Term b = ARG(c->literals->atom,1);
    Plist va = T.set_of_variables(a);
    Plist vb = T.set_of_variables(b);
    if (va && !va->next && vb && !vb->next && !T.term_ident((Term)va->v, (Term) vb->v)) {
      if (new_sn == INT_MAX) {
			new_sn = S.fresh_symbol("c_", 0);
			S.new_constant_properties(new_sn);  /* type, weights, prec */
      }
      return clause_with_new_constant(c, a, new_sn);
    }
    else
      return NULL;
  }
} 

Topform Parautil::fold_denial(Topform c, int alpha_max) {

  TermContainer T;
  SymbolContainer S;
  static bool done = false;
  if (done || !LADRV_GLOBAIS_INST.Lit.neg_eq_unit(c->literals) || !LADRV_GLOBAIS_INST.Lit.ground_clause(c->literals))    return NULL;
  else {
    /* assume it's already oriented */
    Term alpha = ARG(c->literals->atom,0);
    Term beta =  ARG(c->literals->atom,1);

    if (!CONSTANT(beta) && (alpha_max == -1 || T.symbol_count(alpha) <= alpha_max)) {
      int new_constant_sn = S.fresh_symbol("c_", 0);
      Topform ca = clause_with_new_constant(c, alpha, new_constant_sn);
      S.new_constant_properties(new_constant_sn);  /* type, weights, prec */
      done = true;
      return ca;
    }
    else
      return NULL;
  }
}  

bool Parautil::equational_def_2(Term alpha, Term beta) {
  TermContainer T;
  return (!VARIABLE(alpha) && 
	  T.args_distinct_vars(alpha) &&
	  T.variables_subset(beta, alpha) &&
	  T.symbol_occurrences(beta, SYMNUM(alpha)) == 0);
} 

int Parautil::equational_def(Topform c) {

  if (!LADRV_GLOBAIS_INST.Lit.pos_eq_unit(c->literals))
    return 0;
  else {
    Term left = ARG(c->literals->atom,0);
    Term right  = ARG(c->literals->atom,1);
    if (equational_def_2(left, right))
      return 1;
    else if (equational_def_2(right, left))
      return 2;
    else
      return 0;
  }
}

OrderType Parautil::unfold_order(Term alpha, Term beta) {
  SymbolContainer S;
   if (!VARIABLE(alpha) &&
      S.is_unfold_symbol(SYMNUM(alpha)) &&
      equational_def_2(alpha, beta))
    return OrderType::GREATER_THAN;

  else if (!VARIABLE(beta) &&
	   S.is_unfold_symbol(SYMNUM(beta)) &&
	   equational_def_2(beta, alpha))
    return OrderType::LESS_THAN;
  else
    return OrderType::NOT_COMPARABLE;
} 

Topform Parautil::build_reflex_eq(void) {
  TermContainer T;

  TopformContainer TF;
  ClausesContainer C;
  SymbolContainer S;
  Term alpha = T.get_variable_term(0);
  Term beta  = T.get_variable_term(0);
  Term atom  = T.build_binary_term(S.str_to_sn(S.eq_sym(), 2), alpha, beta);
  Literals lit = LADRV_GLOBAIS_INST.Lit.get_literals();
  Topform c = TF.get_topform();
  lit->sign = true;
  lit->atom = atom;
  c->literals = LADRV_GLOBAIS_INST.Lit.append_literal(c->literals, lit);
  TF.upward_clause_links(c);
  return c;
}
