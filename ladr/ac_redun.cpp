#include "ac_redun.h"


Ilist Ac_redun::C_symbols = NULL;   /* C: commutative */
Ilist Ac_redun::A1_symbols = NULL;   /* A1: associative   (xy)z = x(yz) */
Ilist Ac_redun::A2_symbols = NULL;   /* A2: c-associative x(yz) = y(xz) */
Ilist Ac_redun::AC_symbols = NULL;   /* AC: All three */




bool Ac_redun::same_top(Term t1, Term t2) {
  TermContainer T;
  if (VARIABLE(t1) || VARIABLE(t2))    return T.term_ident(t1, t2);
  else return SYMNUM(t1) == SYMNUM(t2);
}

int Ac_redun::commutativity(Term atom) {
  TermContainer T;
  if (T.eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 && same_top(alpha, beta)) {
      Term a = ARG(alpha,0);
      Term b = ARG(alpha,1);
      Term c = ARG(beta,0);
      Term d = ARG(beta,1);
      if (VARIABLE(a) && VARIABLE(b) &&
	  VARNUM(a) != VARNUM(b) &&
	  T.term_ident(a,d) && T.term_ident(b,c))
	return SYMNUM(alpha);
    }
  }
  return 0;
} 

int Ac_redun::associativity(Term atom) {
  TermContainer T;
  if (T.eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 &&
	same_top(alpha, beta) &&
	same_top(alpha, ARG(alpha,0)) &&
	same_top(alpha, ARG(beta,1))) {

      Term a = ARG(ARG(alpha,0),0);
      Term b = ARG(ARG(alpha,0),1);
      Term c = ARG(alpha,1);
      Term d = ARG(beta,0);
      Term e = ARG(ARG(beta,1),0);
      Term f = ARG(ARG(beta,1),1);
	
      if (VARIABLE(a) && VARIABLE(b) && VARIABLE(c) &&
	  VARNUM(a) != VARNUM(b) &&
	  VARNUM(b) != VARNUM(c) &&
	  VARNUM(a) != VARNUM(c) &&
	  T.term_ident(a,d) && T.term_ident(b,e) && T.term_ident(c,f))
	return SYMNUM(alpha);
    }
  }
  return 0;
}


int Ac_redun::c_associativity(Term atom) {
  TermContainer T;
  if (T.eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 &&
	same_top(alpha, beta) &&
	same_top(alpha, ARG(alpha,1)) &&
	same_top(alpha, ARG(beta,1))) {

      Term a = ARG(alpha,0);
      Term b = ARG(ARG(alpha,1),0);
      Term c = ARG(ARG(alpha,1),1);
      Term d = ARG(beta,0);
      Term e = ARG(ARG(beta,1),0);
      Term f = ARG(ARG(beta,1),1);
	
      if (VARIABLE(a) && VARIABLE(b) && VARIABLE(c) &&
	  VARNUM(a) != VARNUM(b) &&
	  VARNUM(b) != VARNUM(c) &&
	  VARNUM(a) != VARNUM(c) &&
	  T.term_ident(a,e) && T.term_ident(b,d) && T.term_ident(c,f))
	return SYMNUM(alpha);
    }
  }
  return 0;
} 


int Ac_redun::associativity3(Term atom){
  TermContainer T;
  if (T.eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 &&
	same_top(alpha, beta) &&
	same_top(alpha, ARG(alpha,0)) &&
	same_top(alpha, ARG(beta,1))) {

      Term a = ARG(ARG(alpha,0),0);
      Term b = ARG(ARG(alpha,0),1);
      Term c = ARG(alpha,1);
      Term d = ARG(beta,0);
      Term e = ARG(ARG(beta,1),0);
      Term f = ARG(ARG(beta,1),1);
	
      if (VARIABLE(a) && VARIABLE(b) && VARIABLE(c) &&
	  VARNUM(a) != VARNUM(b) &&
	  VARNUM(b) != VARNUM(c) &&
	  VARNUM(a) != VARNUM(c) &&
	  ((T.term_ident(a,d) && T.term_ident(b,e) && T.term_ident(c,f)) |
	   (T.term_ident(a,d) && T.term_ident(b,f) && T.term_ident(c,e)) |
	   (T.term_ident(a,e) && T.term_ident(b,d) && T.term_ident(c,f)) |
	   (T.term_ident(a,f) && T.term_ident(b,d) && T.term_ident(c,e))
	   )
	  )
	return SYMNUM(alpha);
    }
  }
  return 0;
} 


int Ac_redun::associativity4(Term atom) {
  TermContainer T;
  if (T.eq_term(atom)) {
    Term alpha = ARG(atom, 0);
    Term beta  = ARG(atom, 1);
    if (ARITY(alpha) == 2 &&
	same_top(alpha, beta) &&
	same_top(alpha, ARG(alpha,1)) &&
	same_top(alpha, ARG(beta,1))) {

      Term a = ARG(alpha,0);
      Term b = ARG(ARG(alpha,1),0);
      Term c = ARG(ARG(alpha,1),1);
      Term d = ARG(beta,0);
      Term e = ARG(ARG(beta,1),0);
      Term f = ARG(ARG(beta,1),1);
	
      if (VARIABLE(a) && VARIABLE(b) && VARIABLE(c) &&
	  VARNUM(a) != VARNUM(b) &&
	  VARNUM(b) != VARNUM(c) &&
	  VARNUM(a) != VARNUM(c) &&
	  ((T.term_ident(d,b) && T.term_ident(e,a) && T.term_ident(f,c)) |
	   (T.term_ident(d,b) && T.term_ident(e,c) && T.term_ident(f,a)) |
	   (T.term_ident(d,c) && T.term_ident(e,a) && T.term_ident(f,b)) |
	   (T.term_ident(d,c) && T.term_ident(e,b) && T.term_ident(f,a))))
	return SYMNUM(alpha);
    }
  }
  return 0;
}  


OrderType Ac_redun::can_compare(Term a, Term b, int sn) {
  TermOrder TO;
  if (VARIABLE(a) || VARIABLE(b))
    return TermOrder::term_compare_vcp(a, b);
  else if (SYMNUM(a) == sn && SYMNUM(b) == sn)
    return TermOrder::term_compare_vcp(a, b);
  else if (SYMNUM(a) == sn)
    return OrderType::GREATER_THAN;
  else if (SYMNUM(b) == sn)
    return OrderType::LESS_THAN;
  else
    return TO.term_compare_vcp(a, b);
} 


Term Ac_redun::canon(Term t, int flag) {
  TermflagContainer TF;
  IlistContainer I;
  if (TF.term_flag(t, flag)) return t;  /* already canonicalized */
  else if (VARIABLE(t))  return t;
  else {
    int sn = SYMNUM(t);
    int i;
    for (i = 0; i < ARITY(t); i++)  ARG(t,i) = canon(ARG(t,i), flag);
    I.set_head(AC_symbols);
    if (I.ilist_member(sn)) {
      Term a = ARG(t,0);
      Term b = ARG(t,1);
      
	  if (same_top(t, b) && can_compare(a, ARG(b,0), sn) == OrderType::GREATER_THAN) {
		Term b0 = ARG(b,0);
		ARG(t,0) = b0;
		ARG(b,0) = a;
		TF.term_flag_clear(b, flag);
		ARG(t,1) = canon(b, flag);
		t = canon(t, flag);
      }
      else 
		  if (can_compare(a, b, sn) == OrderType::GREATER_THAN) {
			ARG(t,0) = b;
			ARG(t,1) = a;
			t = canon(t, flag);
			}
    }
    else {
			I.set_head(C_symbols);
			if (I.ilist_member(sn)) {
			Term a = ARG(t,0);
			Term b = ARG(t,1);
			if (can_compare(a, b, sn) == OrderType::GREATER_THAN) {
				ARG(t,0) = b;
				ARG(t,1) = a;
			}
		}
    }
    TF.term_flag_set(t, flag);  /* mark as canonicalized */
    return t;
  }
} 


bool Ac_redun::cac_redundant_atom(Term atom) {
  TermContainer T;
  TermflagContainer TF;
  if (!T.eq_term(atom))
    return false;  /* must be equality atom */
  else if (VARIABLE(ARG(atom,0)) || VARIABLE(ARG(atom,1)))
    return false;  /* neither side can be a variable */
  else if (SYMNUM(ARG(atom,0)) != SYMNUM(ARG(atom,1)))
    return false;  /* function symbols must be the same */
  else if (T.symbol_count(ARG(atom,0)) != T.symbol_count(ARG(atom,1)))
    return false;  /* symbol counts must be the same */
  else if (commutativity(atom))
    return false;
  else if (associativity(atom))
    return false;
  else if (c_associativity(atom))
    return false;
  else {
    /* Copy terms, AC-canonicalize, check for identity. */

    int flag = TF.claim_term_flag();  /* for marking canonicalized subterms */
    bool cac_redund;
    Term a = T.copy_term(ARG(atom, 0));
    Term b = T.copy_term(ARG(atom, 1));

    a = canon(a, flag);
    b = canon(b, flag);

    cac_redund = T.term_ident(a,b);
    T.zap_term(a);
    T.zap_term(b);
    TF.release_term_flag(flag);
    return cac_redund;
  }
} 


bool Ac_redun::cac_tautology(Literals lits) {
  if (C_symbols == NULL)   return false;
  else if (lits == NULL)   return false;
  else if (lits->sign && cac_redundant_atom(lits->atom))   return true;
  else return cac_tautology(lits->next);
}


bool Ac_redun::cac_redundancy(Topform c, bool print) {

  IlistContainer I,I1,I2,I3;
  
  TermContainer T;
  SymbolContainer S;
  if (LADRV_GLOBAIS_INST.Lit.pos_eq_unit(c->literals)) {
    Term atom = c->literals->atom;
    int sn = SYMNUM(ARG(atom, 0));
    bool new_assoc;
    I.set_head(C_symbols);
    if (!I.ilist_member(sn) && commutativity(atom)) {
      C_symbols = I.ilist_append(sn);
	  I1.set_head(A1_symbols);
	  I2.set_head(A2_symbols);
      if (I1.ilist_member(sn) && I2.ilist_member(sn)) {
		if (print)
			cout<<endl<<"%% Operation "<<S.sn_to_str(sn)<< "associative-comutative; CAC redundancy checks enabled."<<endl;
		I3.set_head(AC_symbols);
		AC_symbols = I3.ilist_append(sn);
      }
      else {
			if (print) cout <<endl<<"%% Operation "<<S.sn_to_str(sn)<<" is commutative; C redundancy checks enabled."<<endl;
	  }
      return true;  /* New C symbol, may also be A1 and/or A2. */
    }

    if (!I1.ilist_member(sn) && associativity(atom)) {
      A1_symbols = I1.ilist_append(sn);
      new_assoc = true;
    }
    else if (!I2.ilist_member(sn) && c_associativity(atom)) {
      A2_symbols = I2.ilist_append(sn);
      new_assoc = true;
    }
    else
      new_assoc = false;

    if (new_assoc) {
      if (I.ilist_member(sn) &&
	  I1.ilist_member(sn) &&
	  I2.ilist_member(sn)) {
		if (print)
			cout<<endl<<"%% Operation "<<S.sn_to_str(sn)<<" is associative-commutative; CAC redundancy checks enabled."<<endl;
			AC_symbols = I3.ilist_append(sn);
			return true;  /* C, A1, A2 (A1 or A2 is newly found) */
	  }
      else	return false;  /* A1 or A2 newly found */
    }
  }  /* pos_eq_unit */

  if (cac_tautology(c->literals)) {
    LADRV_GLOBAIS_INST.Lit.zap_literals(c->literals);
    c->literals = LADRV_GLOBAIS_INST.Lit.get_literals();
    c->literals->sign = true;
    c->literals->atom = T.get_rigid_term(S.true_sym(), 0);
    c->literals->atom->container = c;
  }
  return false;

}
