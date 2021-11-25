#include "termorder.h"
#include "mystring.h"
#include "symbols.h"
#include "multiset.h"
#include "fatal.h"

Order_method TermOrder::Ordering_method = Order_method::LRPO_METHOD; //by default

void TermOrder::assign_order_method(Order_method method) {
  Ordering_method = method;
}  /* assign_order_method */


/*
*************
 *
 *   term_compare_basic(t1, t2)
 *
 *************/

/* DOCUMENTATION
This routine compares two terms.
variable < nonvariable; within type, the order is by VARNUM
and lexigocgaphic by ASCII ordering.  The range of return values is
{SAME_AS, GREATER_THAN, LESS_THAN}.
*/

/* PUBLIC */
OrderType TermOrder::term_compare_basic(Term t1, Term t2) {
  OrderType  rc;
  if (!VARIABLE(t1) && !VARIABLE(t2)) {
    SymbolContainer S;
    string s1 = S.sn_to_str(SYMNUM(t1));
    string s2 = S.sn_to_str(SYMNUM(t2));
    int a1 = ARITY(t1);
    int a2 = ARITY(t2);
    if (myString::str_ident(s1, s2)) {
      /* allow for different arities with same symbol */
      int i;
      for (rc = OrderType::SAME_AS, i = 0; rc == OrderType::SAME_AS && i < a1 && i < a2; i++) rc = term_compare_basic(ARG(t1,i), ARG(t2,i));
      if (rc == OrderType::SAME_AS) rc = (a1 < a2 ? OrderType::LESS_THAN : (a1 > a2 ? OrderType::GREATER_THAN : OrderType::SAME_AS));
    }
    else {
      int r = (s1==s2);
      rc = (r < 0 ? OrderType::LESS_THAN : (r > 0 ? OrderType::GREATER_THAN : OrderType::SAME_AS));
    }
  }

  else if (VARIABLE(t1) && VARIABLE(t2)) {
    if (VARNUM(t1) == VARNUM(t2)) rc = OrderType::SAME_AS;
    else rc = (VARNUM(t1) > VARNUM(t2) ? OrderType::GREATER_THAN :OrderType::LESS_THAN);
  }

  else if (VARIABLE(t1)) rc = OrderType::LESS_THAN;
  else  rc = OrderType::GREATER_THAN;
  return rc;
}  /* term_compare_basic */


/*************
 *
 *    int term_compare_ncv(t1, t2)
 *
 *************/

/* DOCUMENTATION
This routine compares two terms.  The ordering is total:
CONSTANT < COMPLEX < VARIABLE; within type, the order is by VARNUM
and lexigocgaphic by SYMNUM.  The range of return values is<BR>
{SAME_AS, GREATER_THAN, LESS_THAN}.
*/

/* PUBLIC */
OrderType TermOrder::term_compare_ncv(Term t1, Term t2) {
  OrderType  rc;

  if ((COMPLEX(t1) && COMPLEX(t2)) || (CONSTANT(t1) && CONSTANT(t2))) {
    if (SYMNUM(t1) == SYMNUM(t2)) {
      int i;
      for (rc = OrderType::SAME_AS, i = 0; rc == OrderType::SAME_AS &&i < ARITY(t1); i++)
        rc = term_compare_ncv(ARG(t1,i), ARG(t2,i));
    }
    else if (SYMNUM(t1) > SYMNUM(t2))  rc = OrderType::GREATER_THAN;
    else rc = OrderType::LESS_THAN;
  }
  else if (VARIABLE(t1) && VARIABLE(t2)) {
    if (VARNUM(t1) == VARNUM(t2)) rc = OrderType::SAME_AS;
    else rc = (VARNUM(t1) > VARNUM(t2) ? OrderType::GREATER_THAN :OrderType::LESS_THAN);
  }
  /* Now we know they are different types. */
  else if (VARIABLE(t1)) rc = OrderType::GREATER_THAN;
  else if (VARIABLE(t2)) rc = OrderType::LESS_THAN;
  else if (COMPLEX(t1))  rc = OrderType::GREATER_THAN;
  else rc = OrderType::LESS_THAN;  /* CONSTANT(t1) && COMPLEX(t2) */
  return rc;
}  /* term_compare_ncv */



/*************
 *
 *    int term_compare_vcp(t1, t2)
 *
 *************/

/* DOCUMENTATION
This routine compares two terms.  The ordering is total:
VARIABLE < CONSTANT < COMPLEX; within type, the order is by VARNUM
and lexigocgaphic by SYMNUM.  The range of return values is<BR>
{SAME_AS, GREATER_THAN, LESS_THAN}.
*/

/* PUBLIC */
OrderType TermOrder::term_compare_vcp(Term t1, Term t2){
  OrderType  rc;

  if ((COMPLEX(t1) && COMPLEX(t2)) || (CONSTANT(t1) && CONSTANT(t2))) {
    if (SYMNUM(t1) == SYMNUM(t2)) {
      int i;
      for (rc = OrderType::SAME_AS, i = 0; rc == OrderType::SAME_AS &&i < ARITY(t1); i++)
            rc = term_compare_vcp(ARG(t1,i), ARG(t2,i));
    }
    else if (SYMNUM(t1) > SYMNUM(t2)) rc = OrderType::GREATER_THAN;
    else rc = OrderType::LESS_THAN;
  }
  else if (VARIABLE(t1) && VARIABLE(t2)) {
    if (VARNUM(t1) == VARNUM(t2))   rc = OrderType::SAME_AS;
    else  rc = (VARNUM(t1) > VARNUM(t2) ? OrderType::GREATER_THAN :OrderType::LESS_THAN);
  }
  /* Now we know they are different types. */
  else if (VARIABLE(t1))    rc = OrderType::LESS_THAN;
  else if (VARIABLE(t2))    rc = OrderType::GREATER_THAN;
  else if (COMPLEX(t1))     rc = OrderType::GREATER_THAN;
  else rc = OrderType::LESS_THAN;  /* CONSTANT(t1) && COMPLEX(t2) */
  return rc;
}  /* term_compare_vcp */


/*************
 *
 *   term_compare_vr(t1, t2)
 *
 *************/

/* DOCUMENTATION
This routine compares two terms.
variable < nonvariable; within type, the order is by VARNUM
and lexigocgaphic by symbol precedence.  The range of return values is<BR>
{SAME_AS, GREATER_THAN, LESS_THAN}.
*/

/* PUBLIC */
OrderType TermOrder::term_compare_vr(Term t1, Term t2){
  OrderType  rc;
  SymbolContainer S;
  if (!VARIABLE(t1) && !VARIABLE(t2)) {
    if (SYMNUM(t1) == SYMNUM(t2)) {
      int i;
      for (rc = OrderType::SAME_AS, i = 0; rc == OrderType::SAME_AS && i < ARITY(t1); i++)
        rc = term_compare_vr(ARG(t1,i), ARG(t2,i));
    }
    else rc = S.sym_precedence(SYMNUM(t1), SYMNUM(t2));
  }

  else if (VARIABLE(t1) && VARIABLE(t2)) {
    if (VARNUM(t1) == VARNUM(t2))      rc = OrderType::SAME_AS;
    else  rc = (VARNUM(t1) > VARNUM(t2) ? OrderType::GREATER_THAN :OrderType::LESS_THAN);
  }
  else if (VARIABLE(t1))  rc = OrderType::LESS_THAN;
  else rc = OrderType::GREATER_THAN;
  return rc;
}  /* term_compare_vr */



/* DOCUMENTATION
This routine compares two flatterms.
variable < nonvariable; within type, the order is by VARNUM
and lexigocgaphic by symbol precedence.  The range of return values is<BR>
{SAME_AS, GREATER_THAN, LESS_THAN}.
*/

/* PUBLIC */
OrderType TermOrder::flatterm_compare_vr(Flatterm a, Flatterm b) {
  OrderType  rc;
  SymbolContainer S;  
  if (!VARIABLE(a) && !VARIABLE(b)) {
    if (SYMNUM(a) == SYMNUM(b)) {
      int i;
      Flatterm ai = a->next;
      Flatterm bi = b->next;
      for (rc = OrderType::SAME_AS, i = 0; rc == OrderType::SAME_AS && i < ARITY(a); i++) {
        rc = flatterm_compare_vr(ai, bi);
        ai = ai->end->next;
        bi = bi->end->next;
      }
    }
    else rc = S.sym_precedence(SYMNUM(a), SYMNUM(b));
  }

  else if (VARIABLE(a) && VARIABLE(b)) {
    if (VARNUM(a) == VARNUM(b))
      rc = OrderType::SAME_AS;
    else
      rc = (VARNUM(a) > VARNUM(b) ? OrderType::GREATER_THAN : OrderType::LESS_THAN);
  }

  else if (VARIABLE(a)) rc = OrderType::LESS_THAN;
  else rc = OrderType::GREATER_THAN;

  return rc;
}  /* flatterm_compare_vr */


bool TermOrder::lrpo_lex(Term s, Term t, bool lex_order_vars) {
  
  TermContainer T;  
  int i;
  int arity = ARITY(s);

  /* First skip over any identical arguments. */

  for (i = 0; i < arity && T.term_ident(ARG(s,i),ARG(t,i)); i++);

  if (i == arity)  return false;  /* s and t identical */
  else if (lrpo(ARG(s,i), ARG(t,i), lex_order_vars)) {
    /* return (s > each remaining arg of t) */
    bool ok;
    for (ok = true, i++; ok && i < arity; i++)
      ok = lrpo(s, ARG(t,i), lex_order_vars);
    return ok;
  }
  else {
    /* return (there is a remaining arg of s s.t. arg >= t) */
    bool ok;
    for (ok = false, i++; !ok && i < arity; i++)
      ok = (T.term_ident(ARG(s,i), t) || lrpo(ARG(s,i), t, lex_order_vars));
    return ok;
  }
}  /* lrpo_lex */



/* DOCUMENTATION
This routine checks if Term s > Term t in the
Lexicographic Recursive Path Ordering (LRPO),
also known as Recursive Path Ordering with Status (RPOS).

<P>
Function symbols can have either multiset or left-to-right status
(see symbols.c).
If all symbols are multiset, this reduces to the Recursive
Path Ordering (RPO).
If all symbols are left-to-right, this reduces to Lexicographic
Path Ordering (LPO).
*/

/* PUBLIC */
bool TermOrder:: lrpo(Term s, Term t, bool lex_order_vars) {
  
 TermContainer T;  
 SymbolContainer S; 
 if (VARIABLE(s)) {
    if (lex_order_vars) return VARIABLE(t) && VARNUM(s) > VARNUM(t);
    else  return false;
  }

  else if (VARIABLE(t)) {
    if (lex_order_vars) return true;
    else  return T.occurs_in(t, s);  /* s > var iff s properly contains that var */
  }

  else if (SYMNUM(s) == SYMNUM(t) &&  S.sn_to_lrpo_status(SYMNUM(s)) == Lrpo_Status::LRPO_LR_STATUS)
    /* both have the same "left-to-right" symbol. */
    return lrpo_lex(s, t, lex_order_vars);

  else {
    OrderType p = S.sym_precedence(SYMNUM(s), SYMNUM(t));

    if (p == OrderType::SAME_AS) return lrpo_multiset(s, t, lex_order_vars);

    else if (p == OrderType::GREATER_THAN) {
      /* return (s > each arg of t) */
      int i;
      bool ok;
      for (ok = true, i = 0; ok && i < ARITY(t); i++)
            ok = lrpo(s, ARG(t,i), lex_order_vars);
      return ok;
    }

    else {  /* LESS_THAN or NOT_COMPARABLE */
      /* return (there is an arg of s s.t. arg >= t) */
      int i;
      bool ok;
      for (ok = false, i = 0; !ok && i < ARITY(s); i++)
	  ok = T.term_ident(ARG(s,i), t) || lrpo(ARG(s,i), t, lex_order_vars);
      return ok;
    }
  }
}  /* lrpo */






bool  TermOrder::lrpo_multiset(Term t1, Term t2, bool lex_order_vars)
{
  return MultiSet::greater_multiset(ARGS(t1), ARITY(t1), ARGS(t2), ARITY(t2),  lrpo, lex_order_vars);
}  /* lrpo_multiset */


/* DOCUMENTATION
Plist should be a list of terms, e.g., a=3, g=0.
Symbols are written as constants; arity is deduced from the symbol table.
*/

/* PUBLIC */
void TermOrder::init_kbo_weights(Plist weights) {
  SymbolContainer S;
  TermContainer T;
  Plist p;
  for (p = weights; p; p = p->next) {
    Term t =(Term) p->v;
    if (!S.is_eq_symbol(SYMNUM(t))) fatal::fatal_error("init_kbo_weights, not equality");
    else {
      Term a = ARG(t,0);
      Term b = ARG(t,1);
      if (!CONSTANT(a))	fatal::fatal_error("init_kbo_weights, symbol not constant");
      else {
            int wt = T.natural_constant_term(b);
            if (wt == -1) fatal::fatal_error("init_kbo_weights, weight not natural");
            else {
                string str = S.sn_to_str(SYMNUM(a));
                int symnum = S.function_or_relation_sn(str);
                if (symnum == -1) {
                    string mess;
                    mess="init_kbo_weights, symbol " + str + " not fount";
                    fatal::fatal_error(mess);
                }
                S.set_kb_weight(symnum, wt);
            }
      }
    }
  }
}  /* init_kbo_weights */


/* PUBLIC */
int TermOrder::kbo_weight(Term t){
   SymbolContainer S;
   if (VARIABLE(t)) return 1;
   else {
    int wt = S.sn_to_kb_wt(SYMNUM(t));
    int i;
    for (i = 0; i < ARITY(t); i++)
      wt += kbo_weight(ARG(t,i));
    return wt;
  }
}  /* kbo_weight */

/* DOCUMENTATION
Is alpha kbo-greater-than beta?
*/

/* PUBLIC */
bool TermOrder::kbo(Term alpha, Term beta, bool lex_order_vars) {
  TermContainer T;
  SymbolContainer S;
    if (VARIABLE(alpha)) {
    if (lex_order_vars)
       return VARIABLE(beta) && VARNUM(alpha) > VARNUM(beta);
       else  return false;
  }
  else if (VARIABLE(beta)) {
    if (lex_order_vars) return true;
    else return T.occurs_in(beta, alpha);
  }
  else if (ARITY(alpha) == 1 && ARITY(beta) == 1 &&   SYMNUM(alpha) == SYMNUM(beta))    return kbo(ARG(alpha, 0), ARG(beta, 0), lex_order_vars);
  else if (!T.variables_multisubset(beta, alpha)) return false;
  else {
    int wa = kbo_weight(alpha);
    int wb = kbo_weight(beta);
    /* printf("kbo_weight=%d: ", wa); p_term(alpha); */
    /* printf("kbo_weight=%d: ", wb); p_term(beta); */
    if (wa > wb) return true;
    else if (wa < wb)    return false;
    else if (!T.variables_multisubset(alpha, beta))   return false;  /* if weights same, multisets of variables must be same */
    else if (S.sym_precedence(SYMNUM(alpha), SYMNUM(beta)) == OrderType::GREATER_THAN)   return true;
    else if (SYMNUM(alpha) != SYMNUM(beta))   return false;
    else {
      /* Call KBO on first arguments that differ. */
      int i = 0;
      while (i < ARITY(alpha) && T.term_ident(ARG(alpha,i),ARG(beta,i))) i++;
      if (i == ARITY(alpha)) return false;
      else return kbo(ARG(alpha,i), ARG(beta,i), lex_order_vars);
    }
  }
}  /* kbo */


/* DOCUMENTATION
Is alpha > beta in the current term ordering?  (LPR, RPO, KBO)
*/

/* PUBLIC */
bool TermOrder::term_greater(Term alpha, Term beta, bool lex_order_vars) {
  if (Ordering_method == Order_method::KBO_METHOD)   return kbo(alpha, beta, lex_order_vars);
  else   return lrpo(alpha, beta, lex_order_vars);  /* LPO, RPO, LRPO */
}  /* term_greater */


/* DOCUMENTATION
Compare two terms with the current term ordering (LPR, RPO, KBO)
Return GREATER_THAN, LESS_THAN, SAME_AS, or NOT_COMPARABLE.
*/

/* PUBLIC */
OrderType TermOrder::term_order(Term alpha, Term beta) {
  TermContainer T;
  if (term_greater(alpha, beta, false))    return OrderType::GREATER_THAN;
  else if (term_greater(beta, alpha, false))  return OrderType::LESS_THAN;
  else if (T.term_ident(beta, alpha))    return OrderType::SAME_AS;
  else   return OrderType::NOT_COMPARABLE;
}  /* term_order */



/* PUBLIC */
int TermOrder::flat_kbo_weight(Flatterm f) {
  SymbolContainer S;
  if (VARIABLE(f))   return 1;
  else {
    int wt = S.sn_to_kb_wt(SYMNUM(f));
    int i;
    Flatterm fi = f->next;
    for (i = 0; i < ARITY(f); i++) {
      wt += flat_kbo_weight(fi);
      fi = fi->end->next;
    }
    return wt;
  }
}  /* flat_kbo_weight */



bool TermOrder::flat_kbo(Flatterm alpha, Flatterm beta, bool lex_order_vars) {
  FlattermContainer FT;
  SymbolContainer S;
  if (VARIABLE(alpha)) {
    if (lex_order_vars)  return VARIABLE(beta) && VARNUM(alpha) > VARNUM(beta);
    else return false;
  }
  else if (VARIABLE(beta)) {
    if (lex_order_vars)   return true;
    else  return FT.flat_occurs_in(beta, alpha);
  }
  else if (ARITY(alpha) == 1 && ARITY(beta) == 1 &&	   SYMNUM(alpha) == SYMNUM(beta))    return flat_kbo(alpha->next, beta->next, lex_order_vars);
  else if (!FT.flat_variables_multisubset(beta, alpha))  return false;
  else {
    int wa = flat_kbo_weight(alpha);
    int wb = flat_kbo_weight(beta);
    /* printf("kbo_weight=%d: ", wa); p_term(alpha); */
    /* printf("kbo_weight=%d: ", wb); p_term(beta); */
    if (wa > wb) return true;
    else if (wa < wb)     return false;
    else if (!FT.flat_variables_multisubset(alpha, beta))  return false;  /* multisets of variables must be the same */
    else if (S.sym_precedence(SYMNUM(alpha), SYMNUM(beta)) == OrderType::GREATER_THAN)   return true;
    else if (SYMNUM(alpha) != SYMNUM(beta))      return false;
    else {
      Flatterm ai = alpha->next;
      Flatterm bi = beta->next;
      int i = 0;
      while (i < ARITY(alpha) && FT.flatterm_ident(ai,bi)) {
        ai = ai->end->next;
        bi = bi->end->next;
        i++;
      }
      if (i == ARITY(alpha)) return false;
      else
	return flat_kbo(ai, bi, lex_order_vars);
    }
  }
}  /* flat_kbo */

bool TermOrder::flat_lrpo_multiset(Flatterm s, Flatterm t) {
  SymbolContainer S;
  FlattermContainer FT;
  cout<<"ready to abort"<<endl;
  S.p_syms();
  FT.p_flatterm(s);
  FT.p_flatterm(t);
  cout<<"lex vals: "<<S.sn_to_lex_val(SYMNUM(s))<<" "<< S.sn_to_lex_val(SYMNUM(s))<<endl;
  fatal::fatal_error("flat_lrpo_multiset not implemented");
  return false;
}  /* flat_lrpo_multiset */


bool TermOrder::flat_lrpo_lex(Flatterm s, Flatterm t, bool lex_order_vars) {
  FlattermContainer FT;
  int arity = ARITY(s);

  /* First skip over any identical arguments. */

  Flatterm si = s->next;
  Flatterm ti = t->next;
  int i = 0;

  while (i < arity && FT.flatterm_ident(si, ti)) {
    si = si->end->next;
    ti = ti->end->next;
    i++;
  }

  if (i == arity)    return false;  /* s and t identical */
  else if (flat_lrpo(si, ti, lex_order_vars)) {
    /* return (s > each remaining arg of t) */
    bool ok = true;
    i++;
    ti = ti->end->next;
    while (ok && i < arity) {
      ok = flat_lrpo(s, ti, lex_order_vars);
      ti = ti->end->next;
      i++;
    }
    return ok;
  }
  else {
    /* return (there is a remaining arg of s s.t. arg >= t) */
    bool ok = false;
    si = si->end->next;
    i++;
    while (!ok && i < arity) {
      ok = (FT.flatterm_ident(si, t) || flat_lrpo(si, t, lex_order_vars));
      si = si->end->next;
      i++;
    }
    return ok;
  }
}  /* flat_lrpo_lex */


bool TermOrder::flat_lrpo(Flatterm s, Flatterm t, bool lex_order_vars) {
 FlattermContainer FT; 
 SymbolContainer S;
 
 if (VARIABLE(s)) {
    if (lex_order_vars)  return VARIABLE(t) && VARNUM(s) > VARNUM(t);
    else  return false;
  }

  else if (VARIABLE(t)) {
    if (lex_order_vars)  return true;
    else  return FT.flat_occurs_in(t, s);
  }

  else if (SYMNUM(s) == SYMNUM(t) &&  S.sn_to_lrpo_status(SYMNUM(s)) == Lrpo_Status::LRPO_LR_STATUS)
    /* both have the same "left-to-right" symbol. */
    return flat_lrpo_lex(s, t, lex_order_vars);

  else {
    OrderType p = S.sym_precedence(SYMNUM(s), SYMNUM(t));

    if (p == OrderType::SAME_AS)  return flat_lrpo_multiset(s, t);

    else if (p == OrderType::GREATER_THAN) {
      /* return (s > each arg of t) */
      int i = 0;
      bool ok = true;
      Flatterm ti = t->next;
      while (ok && i < ARITY(t)) {
        ok = flat_lrpo(s, ti, lex_order_vars);
        ti = ti->end->next;
        i++;
      }
      return ok;
    }

    else {  /* LESS_THEN or NOT_COMPARABLE */
      /* return (there is an arg of s s.t. arg >= t) */
      int i = 0;
      bool ok = false;
      Flatterm si = s->next;
      while (!ok && i < ARITY(s)) {
        ok = FT.flatterm_ident(si, t) || flat_lrpo(si, t, lex_order_vars);
        si = si->end->next;
        i++;
      }
      return ok;
    }
  }
}  /* flat_lrpo */



/* PUBLIC */
bool TermOrder::flat_greater(Flatterm alpha, Flatterm beta, bool lex_order_vars) {
  FlattermContainer FT;
  TermContainer T;
  if (Ordering_method == Order_method::RPO_METHOD ||  Ordering_method == Order_method::LRPO_METHOD) {
    /* haven't done the flat versions of the multiset operations */
    Term t1 = FT.flatterm_to_term(alpha);
    Term t2 = FT.flatterm_to_term(beta);
    bool result = term_greater(t1, t2, lex_order_vars);  /* LPO, RPO, KBO */
    T.zap_term(t1);
    T.zap_term(t2);
    return result;
  }
  else if (Ordering_method == Order_method::LPO_METHOD)   return flat_lrpo(alpha, beta, lex_order_vars);
  else if (Ordering_method == Order_method::KBO_METHOD)   return flat_kbo(alpha, beta, lex_order_vars);
  else {
    fatal::fatal_error("flat_greater: unknown Ordering_method");
    return false;
  }
}  /* flat_greater */

/* PUBLIC */
bool TermOrder::greater_multiset_current_ordering(Term t1, Term t2) {
  return MultiSet::greater_multiset(ARGS(t1), ARITY(t1), ARGS(t2), ARITY(t2),  Ordering_method == Order_method::KBO_METHOD ? kbo : lrpo, false);
}  /* greater_multiset_current_ordering */
