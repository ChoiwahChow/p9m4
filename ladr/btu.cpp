#include "btu.h"
#include "memory.h"
#include "fatal.h"
#include "symbols.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>




GlobalBtu::GlobalBtu() {
    Btu_state_gets=0;
    Btu_state_frees=0;
    Ac_position_gets=0;
    Ac_position_frees=0;
}
 
GlobalBtu::~GlobalBtu() {
    
}
 
 



BtuContainer::BtuContainer() {
    root=NULL;
    
}

BtuContainer::~BtuContainer() {
    root=NULL;
    
}


Btu_state BtuContainer::get_btu_state(void) {
  Btu_state p = (Btu_state) Memory::memCNew(sizeof(struct btu_state));
  p->varnum = -1;
  LADR_GLOBAL_BTU.Btu_state_gets++;
  return(p);
}


void BtuContainer::free_btu_state(Btu_state p) {
  Memory::memFree((void *) p, sizeof(struct btu_state));
  LADR_GLOBAL_BTU.Btu_state_frees++;
}

Ac_position BtuContainer::get_ac_position(void){
  Ac_position p = (Ac_position) Memory::memCNew(sizeof(struct ac_position));
  LADR_GLOBAL_BTU.Ac_position_gets++;
  return(p);
}

void BtuContainer::free_ac_position(Ac_position p){
  Memory::memFree((void *) p, sizeof(struct ac_position));
  LADR_GLOBAL_BTU.Ac_position_frees++;
}


void BtuContainer::fprint_btu_mem(ostream &o, bool heading) {
  int n;
  if (heading)
   o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;

  n = sizeof(struct btu_state);
  o<<"btu_state   ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_BTU.Btu_state_gets;
  o<<setw(11)<<LADR_GLOBAL_BTU.Btu_state_frees;
  o<<setw(11)<<LADR_GLOBAL_BTU.Btu_state_gets - LADR_GLOBAL_BTU.Btu_state_frees;
  o<<setw(9)<<( (LADR_GLOBAL_BTU.Btu_state_gets - LADR_GLOBAL_BTU.Btu_state_frees) *n) /1024<<"K"<<endl;
  

  n = sizeof(struct ac_position);
  o<<"Ac_position  ("<<setw(4)<<n<<")     "<<setw(11)<<LADR_GLOBAL_BTU.Ac_position_gets;
  o<<setw(11)<<LADR_GLOBAL_BTU.Ac_position_frees;
  o<<setw(11)<<LADR_GLOBAL_BTU.Ac_position_gets - LADR_GLOBAL_BTU.Ac_position_frees;
  o<<setw(9)<<( (LADR_GLOBAL_BTU.Ac_position_gets - LADR_GLOBAL_BTU.Ac_position_frees) *n) /1024<<"K"<<endl;
  
}

void BtuContainer::p_btu_mem()
{
  fprint_btu_mem(cout, true);
}  /* p_btu_mem */


#if 0
void BtuContainer::p_binding(int vnum, Context vc, Term t, Context tc){
  TermContainer T;
  Term vt, ti;
  vt = T.get_variable_term(vnum);
  ti = T.apply(vt, vc);
  printf("[");
  T.fprint_term(cout,vt);
  printf(",0x%x:%d] -> [", (unsigned) vc, vc->multiplier);
  T.fprint_term(cout,t);
  printf(",0x%x:%d] (", (unsigned) tc, tc->multiplier);
  T.fprint_term(cout,ti);
  printf(")\n");
  T.free_term(vt);
  T.free_term(ti);
}  /* p_binding */
#endif


void BtuContainer::flatten_deref(Term t, Context tc, Term *a, Context *ac, int *ip) {
  Term t1;
 TermContainer T;
  Context c1;
  int sn, i;
  sn = SYMNUM(t);

  for (i = 0; i < ARITY(t); i++) {
    t1 = ARG(t,i);
    c1 = tc;
    DEREFERENCE(t1, c1);

    if (SYMNUM(t1) == sn)
      flatten_deref(t1, c1, a, ac, ip);
    else {
      if (*ip >= MAX_ACU_ARGS) {
       
        T.p_term(t1);
        fatal::fatal_error("flatten_deref, too many arguments.");
      }
      else {
        a[*ip] = t1;
        ac[*ip] = c1;
      }
      (*ip)++;
    }
  }
}

OrderType BtuContainer::compare_ncv_context(Term t1, Term t2,Context c1, Context c2) {
  int m1, m2;
  OrderType rc;

  if (CONSTANT(t1) && !CONSTANT(t2))    rc = OrderType::LESS_THAN;
  else if (!CONSTANT(t1) && CONSTANT(t2))    rc = OrderType::GREATER_THAN;
  else if (COMPLEX(t1) && VARIABLE(t2))      rc = OrderType::LESS_THAN;
  else if (VARIABLE(t1) && COMPLEX(t2))      rc = OrderType::GREATER_THAN;

  /* Now we know they are the same type. */

  else if VARIABLE(t1) {
    /* both variables */

    m1 = (c1 ? c1->multiplier : INT_MIN);
    m2 = (c2 ? c2->multiplier : INT_MIN);

    if (m1 < m2)
      rc = OrderType::LESS_THAN;
    else if (m1 > m2)
      rc = OrderType::GREATER_THAN;
    else if (VARNUM(t1) < VARNUM(t2))
      rc = OrderType::LESS_THAN;
    else if (VARNUM(t1) > VARNUM(t2))
      rc = OrderType::GREATER_THAN;
    else
      rc = OrderType::SAME_AS;
  }
  else {
    if (SYMNUM(t1) < SYMNUM(t2))
      rc = OrderType::LESS_THAN;
    else if (SYMNUM(t1) > SYMNUM(t2))
      rc = OrderType::GREATER_THAN;
    else {
      int i;
      for (i = 0, rc = OrderType::SAME_AS; i < ARITY(t1) && rc == OrderType::SAME_AS; i++) rc = compare_ncv_context(ARG(t1,i), ARG(t2,i), c1, c2);
    }
  }
  return(rc);
}


void BtuContainer::sort_ac(Term *a, Context *c, int n){
  int i, j, min_i;
  Term min_t;
  Context min_c;

  for (i = 0; i < n-1; i++) {
    min_t = a[i];
    min_c = c[i];
    min_i = i;
    for (j = i+1; j < n; j++) {
      if (compare_ncv_context(a[j], min_t, c[j], min_c) ==OrderType::LESS_THAN) {
        min_t = a[j];
        min_c = c[j];
        min_i = j;
      }
    }
    if (min_i != i) {
      a[min_i] = a[i];
      a[i] = min_t;
      c[min_i] = c[i];
      c[i] = min_c;
    }
  }

}


void BtuContainer::elim_con_context(Term *a1, Term *a2, Context *c1, Context *c2, int n1, int n2) {
  int i1, i2;
  OrderType rc;

  i1 = i2 = 0;
  while (i1 < n1 && i2 < n2) {
    rc = compare_ncv_context(a1[i1], a2[i2], c1[i1], c2[i2]);
    if (rc == OrderType::SAME_AS) {
      a1[i1] = NULL; c1[i1] = NULL; i1++;
      a2[i2] = NULL; c2[i2] = NULL; i2++;
    }
    else if (rc == OrderType::LESS_THAN)
      i1++;
    else
      i2++;
  }
}


void BtuContainer::ac_mult_context(Term *a, Context *c, int *mults, int *np){
  int it, im, i, m, j, n;
  n = *np;
  im = 0;
  it = 0;
  while (it < n) {
    if (!a[it])  it++;
    else {
            i = it+1;
      m = 1;
      while (i < n && a[i] && compare_ncv_context(a[it], a[i], c[it], c[i]) == OrderType::SAME_AS) {
        a[i] = NULL;
        c[i] = NULL;
        m++;
        i++;
      }
      mults[im++] = m;
      it = i;
    }
  }
  for (i = n-1; i >= 0; i--) {
    if (!a[i]) {
      for (j = i; j < n-1; j++) {
        a[j] = a[j+1];
        c[j] = c[j+1];
      }
      n--;
    }
  }
  if (n != im) {
    fatal::fatal_error("in ac_mult_context, n!=im.");
  }
  *np = n;
}  /* ac_mult_context */


/*************
 *
 *    ac_prepare
 *
 *    Final preparation for diophantine solver.  Fill in the arrays:
 *
 *        ab -          coefficients for both terms
 *        constraints -    0 for variable with context
 *                         -(varnum+1) for variable w/o context (unbindable)
 *                         else symbol number
 *        terms         the arguments
 *        contexts      contexts for arguments 
 *
 *************/


void BtuContainer::ac_prepare(Term *a1, Term *a2, Context *c1, Context *c2,  int *mults1, int *mults2, int n1, int n2, int *ab,    int *constraints,   Term *terms, Context *contexts) {
  int i;
    
  if (n1+n2 > MAX_COEF) {
    fatal::fatal_error("ac_prepare, too many arguments.");
  }

  for (i = 0; i < n1; i++) {
    ab[i] = mults1[i];
    if (VARIABLE(a1[i]))
      constraints[i] = (c1[i] ? 0 : (-VARNUM(a1[i])) - 1);
    else
      constraints[i] = SYMNUM(a1[i]);
    terms[i] = a1[i];
    contexts[i] = c1[i];
  }

  for (i = 0; i < n2; i++) {
    ab[i+n1] = mults2[i];
    if (VARIABLE(a2[i]))
      constraints[i+n1] = (c2[i] ? 0 : (-VARNUM(a2[i])) - 1);
    else
      constraints[i+n1] = SYMNUM(a2[i]);
    terms[i+n1] = a2[i];
    contexts[i+n1] = c2[i];
  }
}  /* ac_prepare */




/*************
 *
 *    set_up_basis_terms
 *
 *    Given the basis solutions, fill in a corresponding array of 
 *    partial terms to be used for building substitutions.
 *    This is done once for the basis, to avoid rebuilding the terms
 *    for each subset.
 *
 *    NOTE: the terms are not well-formed.  Each has form f(ti,NULL),
 *    so that it's quick to make, e.g., f(t1,f(t2,f(t3,t4))).
 *
 *************/

void BtuContainer::set_up_basis_terms(int sn, int (*basis)[MAX_COEF], int num_basis, int length, Term (*basis_terms)[MAX_COEF]) {
  Term t1, t2, t3;
  int i, j, k;
  TermContainer T;

  for (i = 0; i < num_basis; i++)
    for (j = 0; j < length; j++) {
      if (basis[i][j] == 0)	basis_terms[i][j] = NULL;
      else {
            
            t1 = T.get_variable_term(i);
            for (k = 2; k <= basis[i][j]; k++) {
                t2 = T.get_variable_term(i);
                t3 = T.get_rigid_term_dangerously(sn, 2);
                ARG(t3,0) = t2;
                ARG(t3,1) = t1;
                t1 = t3;
            }
            t2 = T.get_rigid_term_dangerously(sn, 2);
            ARG(t2,0) = t1;
            ARG(t2,1) = NULL;
            basis_terms[i][j] = t2;
        }
    }
} 




/*************
 *
 *    unify_ac
 *
 *    Associative-commutative Unification.  t1 and t2 have the same ac symbol.
 *    (t1, c1, t2, c2, are dereferenced terms from bt.)
 *
 *    If c1 and c2 are different they must have different multipliers!!!
 *
 *************/

/* DOCUMENTATION
This routine gets the first or next AC unifier for a pair of terms.
This is mutually recursive with unify_bt_guts() and unify_bt_next()
for sub-unification problems.  The top calls are made to the backtrack
unification routines unify_bt_first(), and unify_bt_next().
*/


int BtuContainer::unify_ac(Term t1, Context c1, Term t2, Context c2, struct btu_state *bt) {
  Term t3, t4, ti;
  Btu_state sub_problems, bt2, bt3;
  Ac_position ac;
  Term a1[MAX_ACU_ARGS], a2[MAX_ACU_ARGS];
  Context ac1[MAX_ACU_ARGS], ac2[MAX_ACU_ARGS], ci;
  int mults1[MAX_ACU_ARGS], mults2[MAX_ACU_ARGS];
  int ab[MAX_COEF], num_basis, n1, n2;
  int i, j, length, vn, ok, status;
  Unif_alternative continuation;
  UnifyContainer U;
  DiophantineEquation D;
  TermContainer T;

  num_basis = length = 0;  /* to quiet compiler */

  continuation = bt->alternative;

  if (continuation == Unif_alternative::NO_ALT) {  /* If first call, set up dioph eq and solve. */
	
    /* Stats[AC_INITIATIONS]++; */
    bt->alternative = Unif_alternative::AC_ALT;

    ac = get_ac_position();
    bt->ac = ac;
    ac->c3 = U.get_context();
    ac->superset_limit = -1;  /* Parms[AC_SUPERSET_LIMIT].val; */

#ifdef DEBUG
    printf("\nunify_ac, new problem:\n");
    printf("    "); fprint_term(stdout, t1); printf(" [0x%x:%d]\n", (unsigned) c1, c1->multiplier);
    printf("    "); fprint_term(stdout, t2); printf(" [0x%x:%d]\n", (unsigned) c2, c2->multiplier);
    printf("    c3 context is [0x%x:%d]\n", (unsigned) ac->c3, ac->c3->multiplier);

#endif

    n1 = 0;
    flatten_deref(t1,c1,a1,ac1,&n1);   /* put args in a1, incl. deref */
    sort_ac(a1, ac1, n1);              /* sort args */
    n2 = 0;
    flatten_deref(t2,c2,a2,ac2,&n2);  
    sort_ac(a2, ac2, n2);
	
    elim_con_context(a1, a2, ac1, ac2, n1, n2);  /* elim. common terms */
	
    ac_mult_context(a1, ac1, mults1, &n1);    /* get multiplicity */
    ac_mult_context(a2, ac2, mults2, &n2);

    if (n1 == 0 && n2 == 0) {
      /* Input terms are identical modulo AC.  */
      /* Succeed with no alternatives.         */
      U.free_context(ac->c3);
      free_ac_position(bt->ac);
      bt->ac = NULL;
      bt->alternative = Unif_alternative::NO_ALT;
      status = SUCCESS;
    }
    else {
	    
      ac_prepare(a1, a2, ac1, ac2, mults1, mults2, n1, n2, ab,
		 ac->constraints, ac->args, ac->arg_contexts);

      ok = D.dio(ab,n1,n2,ac->constraints,ac->basis,&(ac->num_basis));

      num_basis = ac->num_basis;
      length = n1 + n2;
	    
      if (ok == 1 && num_basis > 0) {
	/* if solutions, store data in ac_position */
	ac->m = n1;
	ac->n = n2;
		
	/* prepare for combination search */
		
	set_up_basis_terms(SYMNUM(t1), ac->basis, num_basis,
			   n1+n2, ac->basis_terms);
		
	status = GO;
      }
      else {
	status = FAILURE;
	if (ok == -1) {
	  printf("basis too big for %d %d.\n", n1, n2);
#if 0
	  fprint_term(stdout, t1); printf(" ");
	  fprint_term(stdout, t2); printf("\n");
	  p_ac_basis(ac->basis, ac->num_basis, n1, n2);
	  /* print out args2 */
	  for (i = 0; i < n2; i++)
	    fprint_term(stdout, a2[i]);
	  exit(34);
#endif		    
	}
      }
    }
  }

  else {  /* continuation */

    /* Stats[AC_CONTINUATIONS]++; */
    ac = bt->ac;
#ifdef DEBUG
    printf("\nunify_ac, continuation:\n");
    printf("    "); fprint_term(stdout, t1); printf(" [0x%x:%d]\n", (unsigned) c1, c1->multiplier);
    printf("    "); fprint_term(stdout, t2); printf(" [0x%x:%d]\n", (unsigned) c2, c2->multiplier);
    printf("    c3 context is [0x%x:%d]\n", (unsigned) ac->c3, ac->c3->multiplier);

#endif
    if (ac->sub_position) { /* if subproblems pending */
      ac->sub_position = unify_bt_next(ac->sub_position);
      status = (ac->sub_position ? SUCCESS : GO);
    }
    else
      status = GO;
    num_basis = ac->num_basis;
    length = ac->m + ac->n;
  }

  while (status == GO) {

    if ((int)continuation) {

      /* Undo bindings from previous combination. */

      for (i = length-1; i >= 0; i--) {

	ti = ARG(ac,i); ci = ac->arg_contexts[i];

	if (ci && VARIABLE(ti)) {
	  vn = VARNUM(ti);
#ifdef DEBUG
	  printf("<-<-<- clearing (ci) ");
	  fflush(stdout);
	  p_binding(vn, ci, ci->terms[vn], ci->contexts[vn]);
#endif
	  ci->terms[vn] = NULL;
	  ci->contexts[vn] = NULL;
	}

	else if (CONSTANT(ti) || (!ci && VARIABLE(ti))) {
#ifdef DEBUG
	  printf("<-<-<- clearing (c3)"); fprint_term(stdout, ac->new_terms[i]);
	  p_binding(SYMNUM(ac->new_terms[i]), ac->c3,
		    ac->c3->terms[VARNUM(ac->new_terms[i])],
		    ac->c3->contexts[VARNUM(ac->new_terms[i])]);
#endif
	  ac->c3->terms[VARNUM(ac->new_terms[i])] = NULL;
	  ac->c3->contexts[VARNUM(ac->new_terms[i])] = NULL;
	}
      }
    }

    /* Get first or next relevant subset of the basis solutions.
     * A parameter limits the number of combinations (and makes AC
     * unification incomplete).  -1 means that there is no limit.
     * 0 means that no supsersets are allowed, 1 means that supersets
     * with one additional element are allowed, etc..  Also, if there
     * is a limit, then at most MAX_COMBOS combinations will be returned.
     */

    if (ac->superset_limit < 0)
      ok = D.next_combo_a(length, ac->basis, num_basis, ac->constraints,ac->combo, ac->sum, !(int)continuation);
    else
      ok = D.next_combo_ss(length, ac->basis, num_basis, ac->constraints,
			 ac->combo, ac->sum, !(int)continuation, ac->combos,
			 &(ac->combos_remaining), ac->superset_limit);
#ifdef DEBUG
    printf("    ----Combination? %s\n", ok ? "YES" : "NO");
#endif

    if (ok) {

      /* We now have a potential unifier.  It's not guaranteed,
       * because it may have subterms to be unified.
       */

      sub_problems = bt3 = NULL;

      /* A variable is associated with each row of the basis.
       * ac->combo is the current subst of the rows.
       *
       * Loop through columns, building a term (t4) for each.
       */

      for (i = 0; i < length; i++) {
	t4 = NULL;
	/* Loop through rows, building t4. */
	for (j = 0; j < num_basis; j++) {
	  if (ac->combo[j]) {
	    t3 = ac->basis_terms[j][i];
	    if (t3) {
	      if (!t4)
		t4 = ARG(t3,0);
	      else {
		ARG(t3,1) = t4;
		t4 = t3;
	      }
	    }
	  }
	}
	ac->new_terms[i] = t4;
#ifdef DEBUG
	printf("    ---- arg %d goes with ", i);
	p_term(t4);
#endif
		
	/* t4 must now be unified with args[i].
	 * switch args[i]
	 *   variable: just bind it.
	 *   constant: bind t4 (which is a variable in this case).
	 *   complex:  add t4=args[i] to the set of subproblems.
	 */

	ti = ARG(ac,i); ci = ac->arg_contexts[i];

	if (ci && VARIABLE(ti)) {
	  vn = VARNUM(ti);
	  ci->terms[vn] = t4;
	  ci->contexts[vn] = ac->c3;
#ifdef DEBUG
	  printf("->->->-> binding (ci) ");
	  p_binding(vn, ci, t4, ac->c3);
#endif
	}
	else if (CONSTANT(ti) || (!ci && VARIABLE(ti))) {
	  ac->c3->terms[VARNUM(t4)] = ti;
	  ac->c3->contexts[VARNUM(t4)] = ci;
#ifdef DEBUG
	  printf("->->->-> binding (c3)");
	  p_binding(VARNUM(t4), ac->c3, ti, ci);
#endif
	}
	else {
	  bt2 = get_btu_state();
	  bt2->prev = bt3;
	  if (bt3)
	    bt3->next = bt2;
	  else
	    sub_problems = bt2;
	  bt2->t1 = t4;
	  bt2->c1 = ac->c3;
	  bt2->t2 = ti;
	  bt2->c2 = ci;
	  bt3 = bt2;
#ifdef DEBUG
	  printf("->->->-> subproblem  [");
	  fprint_term(stdout,t4);
	  printf(",0x%x:%d] :: [", (unsigned) ac->c3, ac->c3->multiplier);
	  fprint_term(stdout,ti);
	  printf(",0x%x:%d]\n", (unsigned) ci, ci->multiplier);
#endif
	}
      }  /* for each arg */

      if (sub_problems) {
        ac->sub_position = unify_bt_guts(sub_problems);
        if (ac->sub_position)
            status = SUCCESS;
	else {
#ifdef DEBUG
	  printf("    subproblems failed; continuing\n");
#endif
	  continuation = Unif_alternative(1);
	  status = GO;
	}
      }
      else {
	ac->sub_position = NULL;
	status = SUCCESS;
      }
    }  /* if (ok) */
    else  /* There are no more combinations, so stop. */
      status = EXHAUSTED;
  }  /* while (STATUS == go) */

  if (status == SUCCESS) {
#ifdef DEBUG
    printf("\nunify_ac, success:\n");
    printf("    "); fprint_term(stdout, t1); printf(" [0x%x:%d]\n", (unsigned) c1, c1->multiplier);
    printf("    "); fprint_term(stdout, t2); printf(" [0x%x:%d]\n", (unsigned) c2, c2->multiplier);
    printf("    c3 context is [0x%x:%d]\n", (unsigned) ac->c3, ac->c3->multiplier);
    {
      Term s1, s2;
      s1 = apply(t1, c1);
      s2 = apply(t2, c2);
      printf("    t1 instance: "); fprint_term_nl(stdout, s1);
      printf("    t2 instance: "); fprint_term_nl(stdout, s2);
      zap_term(s1);
      zap_term(s2);
    }
	  

#endif
    return(1);
  }
  else {
    /* Free Memory::, clean up, and fail. */

#ifdef DEBUG
    printf("\nunify_ac, %s:\n", status == EXHAUSTED ? "finished" : "fail");
    printf("    "); fprint_term(stdout, t1); printf(" [0x%x:%d]\n", (unsigned) c1, c1->multiplier);
    printf("    "); fprint_term(stdout, t2); printf(" [0x%x:%d]\n", (unsigned) c2, c2->multiplier);
    printf("    c3 context is [0x%x]\n", (unsigned) ac->c3);

#endif
    if (status == EXHAUSTED) {
      /* Delete all terms in basis_terms. */
      for (i = 0; i < num_basis; i++)
	for (j = 0; j < length; j++)
	  if (ac->basis_terms[i][j]) {
	    t2 = ac->basis_terms[i][j];
	    T.zap_term(ARG(t2,0));
	    T.free_term(t2);
	  }
    }

    U.free_context(ac->c3);
    free_ac_position(bt->ac);
    bt->ac = NULL;
    bt->alternative = Unif_alternative::NO_ALT;
    return(0);
  }
}  /* unify_ac */


void BtuContainer::unify_ac_cancel(Ac_position ac) {
  int i, j, length, vn;
  Context ci;
  Term t2, ti;
  Btu_state bt;
  TermContainer T;

  length = ac->m + ac->n;
    
  /* Undo bindings from previous combination. */
    
  for (i = 0; i < length; i++) {
	
    ti = ARG(ac,i); ci = ac->arg_contexts[i];
	
    if (ci && VARIABLE(ti)) {
      vn = VARNUM(ARG(ac,i));
      ci->terms[vn] = NULL;
      ci->contexts[vn] = NULL;
    }
	
    else if (CONSTANT(ti) || (!ci && VARIABLE(ti))) {
      ac->c3->terms[VARNUM(ac->new_terms[i])] = NULL;
      ac->c3->contexts[VARNUM(ac->new_terms[i])] = NULL;
    }
  }
    
  /* Delete all terms in basis_terms. */
    
  for (i = 0; i < ac->num_basis; i++)
    for (j = 0; j < length; j++)
      if (ac->basis_terms[i][j]) {
       
        t2 = ac->basis_terms[i][j];
        T.zap_term(ARG(t2,0));
        T.free_term(t2);
      }

  if (ac->sub_position) {
    /* unity_bt leaves you at the end of the list, so get to the start. */
    for (bt = ac->sub_position; bt->prev; bt = bt->prev);
    unify_bt_cancel(bt);
  }
  UnifyContainer U;  
  U.free_context(ac->c3);
  free_ac_position(ac);
    
}  /* unify_ac_cancel */

/*************
 *
 *   p_ac_position() - print ac_position
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) some of the data in an Ac_position.
*/


void BtuContainer::p_ac_position(Ac_position ac, int n) {
  UnifyContainer U;
  if (!ac)
    printf("\nac_position is NULL.\n");
  else {
    printf("\nac_position, context c3:\n");
    U.p_context(ac->c3);
    printf("\nSub problems:\n");
    if (!ac->sub_position)
      printf("none.\n");
    else {
      Btu_state bt;
      /* Get to the beginning of the list of subproblems. */
      for (bt = ac->sub_position; bt->prev; bt = bt->prev);
      for (; bt; bt = bt->next) p_bt_tree(bt, n);
    }
    printf("end of ac_position, context c3:\n");
  }
}  /* p_ac_position */

/*************
 *
 *    Btu_state unify_bt_backup(bt)
 *
 *    Back up (freeing nodes) to the most recent node with an alternative.
 *
 *************/


Btu_state BtuContainer::unify_bt_backup(Btu_state bt1) {
  Btu_state bt2, bt3;

  while (bt1 != NULL && bt1->alternative == Unif_alternative::NO_ALT) {

    if (bt1->cb) {  /* unbind variable */
#ifdef DEBUG
      printf("CLEAR: v%d, c%d\n", bt1->varnum, bt1->cb->multiplier);
      fflush(stdout);
#endif
      bt1->cb->terms[bt1->varnum] = NULL;
      bt1->cb->contexts[bt1->varnum] = NULL;
      bt1->cb = NULL;
    }
	
    if (bt1->prev) {
      bt1 = bt1->prev;
      while (bt1->last_child)
	bt1 = bt1->last_child;
    }
    else {
      bt2 = bt1;
      bt1 = bt1->parent;

      while (bt2) {
        bt3 = bt2;
        bt2 = bt2->next;
        free_btu_state(bt3);
      }

      if (bt1)
	bt1->first_child = bt1->last_child = NULL;
    }
  }
    
  return(bt1);
	
}  /* unify_bt_backup */

/*************
 *
 *  unify_commute()
 *
 *  Commutative unification.  t1 and t2 have the same commutative symbol.
 *  t1, c1, t2, c2, are dereferenced terms from bt.
 *
 *  This simply tries to unify both ways.
 *  We can get redundant unifiers if both ways unify, for example,
 *  f(a,x) and f(a,a) unify twice, both times with the same substitution.
 *
 *************/


int BtuContainer::unify_commute(Term t1, Context c1, Term t2, Context c2, Btu_state bt) {
  Btu_state bt1, bt2;

  if (bt->alternative ==Unif_alternative::NO_ALT) {  /* first call */
    bt->alternative = Unif_alternative::COMM_ALT;
    bt->flipped = 0;

    /* Set up 2 subproblems, then unify guts. */

    bt1 = get_btu_state();  bt2 = get_btu_state();
    bt1->next = bt2; bt2->prev = bt1;
    bt1->c1 = c1; bt1->c2 = c2;
    bt2->c1 = c1; bt2->c2 = c2;
    bt1->t1=ARG(t1,0); bt1->t2=ARG(t2,0);
    bt2->t1=ARG(t1,1); bt2->t2=ARG(t2,1);

    bt->position_bt = unify_bt_guts(bt1);
  }
  else  /* continuation */
    bt->position_bt = unify_bt_next(bt->position_bt);

  if (!bt->position_bt && !bt->flipped) {

    /* Set up 2 subproblems, with t2 flipped, then unify guts. */

    bt1 = get_btu_state();  bt2 = get_btu_state();
    bt1->next = bt2; bt2->prev = bt1;
    bt1->c1 = c1; bt1->c2 = c2;
    bt2->c1 = c1; bt2->c2 = c2;
    bt1->t1=ARG(t1,0); bt1->t2=ARG(t2,1);
    bt2->t1=ARG(t1,1); bt2->t2=ARG(t2,0);

    bt->flipped = 1;
    bt->position_bt = unify_bt_guts(bt1);
  }

  if (bt->position_bt)
    return(1);
  else {
    bt->alternative = Unif_alternative::NO_ALT;
    return(0);
  }
    
}  /* unify_commute */

/*************
 *
 *    unify_bt_first
 *
 *    This is backtracking unification, to be used when there
 *    can be more than one unifier.  This version handles (any number of)
 *    commutative and associative-commutative function symbols.
 *
 *    Get first unifier.  Return position for unify_bt_next calls.
 *    This procedure can also be used for matching, because a NULL
 *    context causes the corresponding term to be treated as ground.
 *    
 *    Here is an example of its use:
 *
 *        c1 = get_context();
 *        c2 = get_context();
 *        bt = unify_bt_first(t1, c1, t2, c2);
 *        while (bt) {
 *            t3 = apply(t1, c1);
 *            t4 = apply(t2, c2);
 *            zap_term(t3);
 *            zap_term(t4);
 *            bt = unify_bt_next(bt);
 *            }
 *        free_context(c1);
 *        free_context(c2);
 *
 *************/

/* DOCUMENTATION
This routine gets the first unifier for a pair of terms and
returns a Btu_state (or NULL if there are no unifiers) to be
used for calls to unify_bt_next() to get the rest of the unifiers.
This unification handles associative-commutative (AC) and 
commutative (C) symbols, so there can be more than one unifier.
(Commutatvie unification is primitive, and you can get duplicate unifiers.)
<P>
This is called "backtrack unification", because the unifiers
are constructed incrementally, as needed.  Here is an example
of how to do it.  Assume we have Terms t1 and t2.
<PRE>
  {
    Context c1 = get_context();
    Context c2 = get_context();
    bt = unify_bt_first(t1, c1, t2, c2);
    while (bt != NULL) {
      t3 = apply(t1, c1);
      t4 = apply(t2, c2);
      <Now, t3 and t4 should be identical, mod any AC or C symbols.>
      zap_term(t4);
      bt = unify_bt_next(bt);
      }
    free_context(c1);
    free_context(c2);
  }
</PRE>
The routine unify_bt_next() takes care of clearing the substitutions
before getting the next unifier.
If you decide not to get all of the unifiers, you should call
unify_bt_cancel() to free the Memory:: used by the Btu_state.
<P>
If there are no AC or C symbols, it is a little bit faster to
use unify() (ordinary unification) instead of backtrack unification.
*/

/* PUBLIC */
Btu_state BtuContainer::unify_bt_first(Term t1, Context c1,
			Term t2, Context c2)
{
  Btu_state bt = get_btu_state();
  bt->t1 = t1; bt->t2 = t2; bt->c1 = c1; bt->c2 = c2;
  /* p_term(t1); p_term(t2); printf("\n"); */
  return(unify_bt_guts(bt));
}  /* unify_bt */

/*************
 *
 *    unify_bt_next
 *
 *    Get next unifier.  Return position for subsequent calls.
 *
 *************/

/* DOCUMENTATION
This routine gets the next unifier for "backtrack unification".
See unify_bt_first() for an explanation.
*/

/* PUBLIC */
Btu_state BtuContainer::unify_bt_next(Btu_state bt1)
{
  /* Go to last node in tree, then back up to a node with an alternative. */

  while (bt1->next != NULL)
    bt1 = bt1->next;
  while (bt1->last_child != NULL)
    bt1 = bt1->last_child;
  bt1 = unify_bt_backup(bt1);
  if (bt1 != NULL)
    return(unify_bt_guts(bt1));
  else
    return(NULL);
}  /* unify_bt_next */

/*************
 *
 *    unify_bt_cancel
 *
 *    This routine should be called if the rest of a sequence of
 *    unifiers is not called for.  It clears substitutions as well
 *    frees Memory::.
 *
 *************/

/* DOCUMENTATION
This routine frees the Memory:: associated with a state in
backtrack unification.  This should be called if you decide
to get some, but not all, unifiers from unify_bt_first() and
unify_bt_next().
*/

/* PUBLIC */
void BtuContainer::unify_bt_cancel(Btu_state bt) {
  Btu_state bt1, bt2;

  for (bt1 = bt; bt1 != NULL; ) {

    unify_bt_cancel(bt1->first_child);
	
    if (bt1->alternative == Unif_alternative::COMM_ALT)
      unify_bt_cancel(bt1->position_bt);
    else if (bt1->alternative == Unif_alternative::AC_ALT) {
      unify_ac_cancel(bt1->ac);
    }
    else if (bt1->cb != NULL) {
      bt1->cb->terms[bt1->varnum] = NULL;
      bt1->cb->contexts[bt1->varnum] = NULL;
    }
    bt2 = bt1;
    bt1 = bt1->next;
    free_btu_state(bt2);
  }
}  /* unify_bt_cancel */

/*************
 *
 *    unify_bt_guts
 *
 *    Main loop for backtracking unification.
 *
 *************/

/* DOCUMENTATION
This routine (mutually recursive with unify_ac()), does 
the important work of backtrack unification.  It is called
bt unify_bt_first() and unify_bt_next().
*/


Btu_state BtuContainer::unify_bt_guts(Btu_state bt1) {
  Term t1, t2;
  Context c1, c2;
  int vn1, vn2, status;
  Btu_state bt2, bt3;
  SymbolContainer S; 
  UnifyContainer U;
  status = GO;
  while (status == GO) {
    t1 = bt1->t1;
    t2 = bt1->t2;
    c1 = bt1->c1;
    c2 = bt1->c2;
    DEREFERENCE(t1, c1)
    DEREFERENCE(t2, c2)

#ifdef DEBUG
    printf("guts loop (derefed) ");
    fprint_term(stdout, t1); printf(" %d ",   c1 ? c1->multiplier : -2);
    fprint_term(stdout, t2); printf(" %d \n", c2 ? c2->multiplier : -2);
#endif	    
	
    if (bt1->alternative == Unif_alternative::COMM_ALT) {
      if (unify_commute(t1, c1, t2, c2, bt1))
	status = POP;
      else
	status = BACKTRACK;
    }
    else if (bt1->alternative == Unif_alternative::AC_ALT) {
      if (unify_ac(t1, c1, t2, c2, bt1))
	status = POP;
      else
	status = BACKTRACK;
    }
    else if (c1 && VARIABLE(t1)) {
      vn1 = VARNUM(t1);
      if (VARIABLE(t2)) {
	if (vn1 == VARNUM(t2) && c1 == c2)
	  status = POP;
	else {
#ifdef DEBUG 
	  printf("BIND: v%d, c%d\n", vn1, c1->multiplier);
	  fflush(stdout);
#endif
	  BIND_BT(vn1, c1, t2, c2, bt1)
	  status = POP;
	}
      }
      else {
	/* t1 variable, t2 not variable */
	/* Stats[BT_OCCUR_CHECKS]++; */
	if (U.occur_check(vn1, c1, t2, c2)) {
#ifdef DEBUG
	  printf("BIND: v%d, c%d\n", vn1, c1->multiplier);
	  fflush(stdout);
#endif
	  BIND_BT(vn1, c1, t2, c2, bt1)
	  status = POP;
	}
	else
	  status = BACKTRACK;
      }
    }
	
    else if (c2 && VARIABLE(t2)) {
      /* t2 variable, t1 not variable */
      vn2 = VARNUM(t2);
      /* Stats[BT_OCCUR_CHECKS]++; */
      if (U.occur_check(vn2, c2, t1, c1)) {
#ifdef DEBUG
	printf("BIND: v%d, c%d\n", vn2, c2->multiplier);
	fflush(stdout);
#endif
	BIND_BT(vn2, c2, t1, c1, bt1)
	status = POP;
      }
      else
	status = BACKTRACK;
    }

    else if (SYMNUM(t1) != SYMNUM(t2))
      status = BACKTRACK;

    else if (CONSTANT(t1))
      status = POP;
	
    else {  /* both COMPLEX with same symbol (and same arity) */

      if (S.is_commutative(SYMNUM(t1))) {
	if (unify_commute(t1, c1, t2, c2, bt1))
	  status = POP;
	else
	  status = BACKTRACK;
      }
      else if (S.is_assoc_comm(SYMNUM(t1))) {
	if (unify_ac(t1, c1, t2, c2, bt1))
	  status = POP;
	else
	  status = BACKTRACK;
      }
      else {
	/* Set up children corresponding to args of <t1,t2>.
	 * Order not important for correctness.
	 * AC kids last for efficiency, but keep in order otherwise.
	 */
	int i;
	bt3 = NULL;

	for (i = 0; i < ARITY(t1); i++) {

	  bt2 = get_btu_state();
	  bt2->t1 = ARG(t1,i);
	  bt2->t2 = ARG(t2,i);
	  bt2->c1 = c1;
	  bt2->c2 = c2;
	  bt2->parent = bt1;

	  if (S.is_assoc_comm(SYMNUM(ARG(t1,i)))) {
	    /* insert at end */
	    bt2->prev = bt1->last_child;
	    if (bt1->last_child)
	      bt1->last_child->next = bt2;
	    else
	      bt1->first_child = bt2;
	    bt1->last_child = bt2;
	  }
	  else {
	    if (bt3) {
	      /* insert after bt3 */
	      bt2->next = bt3->next;
	      bt2->prev = bt3;
	      bt3->next = bt2;
	      if (bt2->next)
		bt2->next->prev = bt2;
	      else
		bt1->last_child = bt2;
	    }
	    else {
	      /* insert at beginning */
	      bt2->next = bt1->first_child;
	      if (bt2->next)
		bt2->next->prev = bt2;
	      else
		bt1->last_child = bt2;
	      bt1->first_child = bt2;
	    }
	    bt3 = bt2;
	  }
	}

	bt1 = bt1->first_child;
	status = GO;
      }
    }
	
    if (status == POP) {
      while (!bt1->next && bt1->parent)
	bt1 = bt1->parent;
      if (!bt1->next)
	status = SUCCESS;
      else {
	bt1 = bt1->next;
	status = GO;
      }
    }
    else if (status == BACKTRACK) {
      bt1 = unify_bt_backup(bt1);
      if (bt1)
	status = GO;
      else
	status = FAILURE;
    }
  }
  return(bt1);
}  /* unify_bt_guts */

/*************
 *
 *    p_bt_tree -- print a bt tree (This could be improved!)
 *
 *************/

/* DOCUMENTATION
This (recursive) routine prints (to stdout) a backtrack unification state.
Parameter n should be 0 on the top call.
*/

/* PUBLIC */
void BtuContainer::p_bt_tree(Btu_state bt, int n) {
  int i;
  Btu_state curr, prev;
  TermContainer T;
  if (bt == NULL) 
    printf("bt tree NULL.\n");
  else {
    printf("\n" );
    for (i = 0; i < n%100; i++)
      printf("----");
    printf(" bt_tree: %d\n", n);
   
    T.fprint_term(cout, bt->t1); printf(" [ %p ]\n", bt->c1);
    T.fprint_term(cout, bt->t2); printf(" [ %p ]\n", bt->c2);
    UnifyContainer U;
    U.p_context(bt->c1);
    U.p_context(bt->c2);

    if (bt->alternative == Unif_alternative::AC_ALT) {
      p_ac_position(bt->ac, n+100);
    }
	
    prev = NULL;
    for (curr = bt->first_child; curr; curr = curr->next) {
      if (curr->parent != bt)
	printf("parent error\n");
      if (curr->prev != prev)
	printf("prev error\n");
      p_bt_tree(curr, n+1);
      prev = curr;
    }
    if (bt->last_child != prev)
      printf("last error\n");
    printf(" end of bt_tree: %d\n", n);
  }
}  /* p_bt_tree */




