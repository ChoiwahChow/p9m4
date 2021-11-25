#include "demod.h"
#include "termflag.h"
#include "unify.h"
#include "symbols.h"
#include "fatal.h"
#include "flatdemod.h"


int Demod::Demod_attempts=0;
int Demod::Demod_rewrites=0;


int Demod::demodulator_type(Topform c, int lex_dep_demod_lim, bool sane) {
  
  Parautil Pu;
  TermContainer T;
  if (!LADRV_GLOBAIS_INST.Lit.pos_eq_unit(c->literals)) return (int) Demod_Type::NOT_DEMODULATOR;
  else {
   
	Term atom = c->literals->atom;
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    int n_alpha = T.symbol_count(alpha);
    int n_beta = T.symbol_count(beta);
  
    if (Pu.oriented_eq(atom))      return (int) Demod_Type::ORIENTED;
    else if (sane && n_alpha != n_beta) return (int) Demod_Type::NOT_DEMODULATOR;
    else if (lex_dep_demod_lim != -1 &&  n_alpha + n_beta + 1 > lex_dep_demod_lim)  return (int) Demod_Type::NOT_DEMODULATOR;
    else {
      Plist alpha_vars = T.set_of_variables(alpha);
      Plist beta_vars =  T.set_of_variables(beta);
      PlistContainer P1,P2;
	  P1.set_head(beta_vars);
	  P2.set_head(alpha_vars);
	  bool lr = P2.plist_subset(beta_vars) && !VARIABLE(alpha);
      bool rl = !Pu.renamable_flip_eq(atom) &&	P1.plist_subset(alpha_vars) && !VARIABLE(beta);
	
      P1.zap_plist();
      P2.zap_plist();

      if (lr && rl)	return (int) Demod_Type::LEX_DEP_BOTH;
      else if (lr)	return (int) Demod_Type::LEX_DEP_LR;
      else if (rl)	return (int) Demod_Type::LEX_DEP_RL;
      else	return (int) Demod_Type::NOT_DEMODULATOR;
    }
  }
}


void Demod::idx_demodulator(Topform c, int type, Indexop operation, Mindex idx) {
  MindexContainer M;
  Term atom = c->literals->atom;
  Term alpha = ARG(atom, 0);
  Term beta  = ARG(atom, 1);

  if (type == (int) Demod_Type::ORIENTED ||
      type == (int) Demod_Type::LEX_DEP_LR ||
      type == (int) Demod_Type::LEX_DEP_BOTH)
    M.mindex_update(idx, alpha, operation);  /* index for left->right */

  if (type == (int) Demod_Type::LEX_DEP_RL ||
      type == (int) Demod_Type::LEX_DEP_BOTH)
    M.mindex_update(idx, beta, operation);   /* index for right->left */
} 

int Demod::demod_attempts() {
  return Demod_attempts;
} 

int Demod::demod_rewrites() {
  return Demod_rewrites;
}

Term Demod::demod(Term t, Mindex demods, int flag, Ilist *just_head,bool lex_order_vars) {
  TermflagContainer TF;
  UnifyContainer U;
  DiscrimBContainer Db;
  TermContainer T;
  TermOrder To;
  Parautil Pu;
  IlistContainer I;
  
  if (TF.term_flag(t, flag) || VARIABLE(t))
    ;  /* leave it alone */
  else {
    Discrim_pos dpos;
    Term found;
    Context c;  /* allocate after we demoulate the subterms */
    int i;

    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = demod(ARG(t,i), demods, flag, just_head, lex_order_vars);

    c = U.get_context();
    Demod_attempts++;
    found = (Term) Db.discrim_bind_retrieve_first(t, demods->discrim_tree, c, &dpos);
				  
    while (found != NULL) {
      Topform demodulator = (Topform) found->container;
      Term atom = demodulator->literals->atom;
      Term alpha = ARG(atom, 0);
      Term beta = ARG(atom, 1);
      bool match_left = (found == alpha);
      Term other = (match_left ? beta : alpha);
      Term contractum = U.apply_demod(other, c, flag);
      bool ok;

      if (Pu.oriented_eq(atom))	  ok = true;
      else	ok = To.term_greater(t, contractum, lex_order_vars);  /* LPO, RPO, KBO */
      
      if (ok) {
		Db.discrim_bind_cancel(dpos);
		Demod_rewrites++;
		T.zap_term(t);
		
		I.set_head(*just_head);
		*just_head = I.ilist_prepend(demodulator->id);
		*just_head = I.ilist_prepend(match_left ? 1 : 2);
		t = demod(contractum, demods, flag, just_head, lex_order_vars);
		found = NULL;
      }
      else {
		T.zap_term(contractum);
		found = (Term) Db.discrim_bind_retrieve_next(dpos);
      }
    }
    U.free_context(c);
    TF.term_flag_set(t, flag);  /* Mark as fully demodulated. */
  }
  return(t);
} 

Term Demod::contract_bt(Term t, Mindex demods, int flag, Topform *demodulator_ptr)
{
  Mindex_pos pos;
  MindexContainer M;
  TermContainer T;
  UnifyContainer U;
  Term contractum, alpha;
  Topform demodulator = NULL;
  Context c = U.get_context();

  alpha = M.mindex_retrieve_first(t,demods,Querytype::GENERALIZATION,NULL,c,true,&pos);
				  
  if (alpha == NULL)    contractum = NULL;
  else {
    Term atom;
    demodulator =(Topform) alpha->container;
    atom = demodulator->literals->atom;
    contractum = U.apply_demod(ARG(atom,1), c, flag);
    if (c->partial_term) {
      /* Get copy, including marks that indicate normal terms. */
      Term partial = U.apply_demod(c->partial_term, NULL, flag);
      contractum = T.build_binary_term(SYMNUM(t), contractum, partial);
    }
    M.mindex_retrieve_cancel(pos);
  }
  U.free_context(c);
  *demodulator_ptr = demodulator;
  return(contractum);
} 

Term Demod::demod_bt(Term t, Mindex demods, int psn, int flag, Ilist *just_head) {
  TermflagContainer TF;
  SymbolContainer S;
  TermContainer T;
  if (TF.term_flag(t, flag) || VARIABLE(t))
    ;  /* leave it alone */
  else {
    int i;
    int sn = SYMNUM(t);
    for (i = 0; i < ARITY(t); i++)
      ARG(t,i) = demod_bt(ARG(t,i), demods, sn, flag, just_head);

    if (sn == psn && S.is_assoc_comm(sn))
      ;  /* t is part of an AC term, so leave it alone. */
    else {
      Term contractum;
      Topform demodulator;
      Accnon::ac_canonical(t, flag);
      Demod_attempts++;
      contractum = contract_bt(t, demods, flag, &demodulator);
      if (contractum) {
        Demod_rewrites++;
        T.zap_term(t);
        Accnon::ac_canonical(contractum, flag);
        IlistContainer I;
        I.set_head(*just_head);
        *just_head = I.ilist_prepend(demodulator->id);
        t = demod_bt(contractum, demods, psn, flag, just_head);
      }
    }
  }
  /* Mark as fully demodulated.  This also means ac_canonical. */
  TF.term_flag_set(t, flag);
  return(t);
} 

Term Demod::demodulate(Term t, Mindex demods, Ilist *just_head, bool lex_order_vars) {
  TermflagContainer TF;
  int flag = TF.claim_term_flag();
  Term result;

  if (demods->unif_type == Uniftype::ORDINARY_UNIF)
    result = demod(t, demods, flag, just_head, lex_order_vars);
  else
    result = demod_bt(t, demods, -1, flag, just_head);
  TF.term_flag_clear_recursively(result, flag);
  TF.release_term_flag(flag);
  return result;
}

Term Demod::demod1_recurse(Term top, Term t, Topform demodulator, int direction,  Ilist *ipos, bool lex_order_vars)
{
  UnifyContainer U;
  Parautil Pu;
  TermContainer T;
  TermOrder To;
  if (VARIABLE(t))
    ;  /* leave it alone */
  else {
    int i;

    for (i = 0; i < ARITY(t) && *ipos == NULL; i++)
      ARG(t,i) = demod1_recurse(top,ARG(t,i),demodulator,direction,ipos,
				lex_order_vars);

    if (*ipos == NULL) {
      Context c1 = U.get_context();
      Trail tr = NULL;
      Term atom = demodulator->literals->atom;
      bool match_left = (direction == 1);
      Term t1 = ARG(atom, match_left ? 0 : 1);
      Term t2 = ARG(atom, match_left ? 1 : 0);

      if (U.match(t1, c1, t, &tr)) {
		Term contractum = U.apply_demod(t2, c1, -1);
		
		UnifyContainer U;
		bool ok;
		if (Pu.oriented_eq(atom))ok = true;
		else ok = To.term_greater(t, contractum, lex_order_vars); /* LPO, RPO, KBO */
	
		if (ok) {
                    U.undo_subst(tr);
                    *ipos = T.position_of_subterm(top, t);
                    T.zap_term(t);
                    t = contractum;
		}
		else T.zap_term(contractum);
	}
     U.free_context(c1);
    }
  }
  return(t);
} 



void Demod::demod1(Topform c, Topform demodulator, int direction,   Ilist *fpos, Ilist *ipos,bool lex_order_vars) {
  Term result;
  Literals lit;
  int n = 0;

  for (lit = c->literals, *ipos = NULL; lit && *ipos == NULL; lit=lit->next) {
    n++;
    result = demod1_recurse(lit->atom, lit->atom, demodulator,direction,ipos,
			    lex_order_vars);
  }

  if (*ipos == NULL)
    fatal::fatal_error("demod1, clause not rewritable");
  else {
	IlistContainer I1,I2;
    
	I1.set_head(*fpos);	
    *fpos = I1.ilist_prepend(direction);  /* side of demodulator */
    *fpos = I1.ilist_prepend(1);  /* literal number */
    I2.set_head(*ipos);
    *ipos = I2.ilist_prepend(n);  /* literal number */
  }

}

Term Demod::part_recurse(Term top, Term t, Topform demod, int target, int direction, int *sequence, Ilist *ipos) {
  UnifyContainer U;
  TermContainer T;
  if (VARIABLE(t))
    return t;
  else {
    int i;
    for (i = 0; i < ARITY(t) && *ipos == NULL; i++) {
      ARG(t,i) = part_recurse(top, ARG(t,i), demod, target,
			      direction, sequence, ipos);
    }
    
    if (*ipos != NULL || *sequence >= target)
      return t;
    else {
      (*sequence)++;
      if (*sequence != target)
	return t;
      else {
            Term alpha, beta;
            Context subst = U.get_context();
            Trail tr = NULL;
            if (direction == 1) {
                alpha = ARG(demod->literals->atom, 0);
                beta  = ARG(demod->literals->atom, 1);
            }
            else {
            alpha = ARG(demod->literals->atom, 1);
            beta  = ARG(demod->literals->atom, 0);
        }
        if (U.match(alpha, subst, t, &tr)) {
                Term result = U.apply(beta, subst);
                U.undo_subst(tr);
                *ipos = T.position_of_subterm(top, t);
                T.zap_term(t);
                t = result;
            }   
	else {
            
            cout<<"demodulator:"<< string(direction == 1 ? "" : " (R)");
            TopformContainer TF;
            TF.fprint_clause(cout, demod);
            cout<<"target="<<target;
            T.p_term(t); 
            fatal::fatal_error("part_recurse, not rewitable");
	}
	U.free_context(subst);
	return t;
      }
    }
  }
}

void Demod::particular_demod(Topform c, Topform demodulator, int target, int direction, Ilist *fpos, Ilist *ipos){
  Literals lit;
  int n = 0;
  int sequence = 0;
  

  for (lit = c->literals, *ipos = NULL; lit && *ipos == NULL; lit=lit->next) {
    n++;
    part_recurse(lit->atom, lit->atom, demodulator, target, direction, &sequence, ipos);
  }

  if (*ipos == NULL)
    fatal::fatal_error("particular_demod, clause not rewritable");
  else {
    IlistContainer I1,I2;
    I1.set_head(*fpos);
    *fpos = I1.ilist_prepend(direction);  /* side of demodulator */
    *fpos = I1.ilist_prepend(1);  /* literal number */
    I2.set_head(*ipos);
    *ipos = I2.ilist_prepend(n);  /* literal number */
    TopformContainer TF;
    TF.upward_clause_links(c);
  }
}
