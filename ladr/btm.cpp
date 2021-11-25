#include "btm.h"
#include "memory.h"
#include "fatal.h"
#include "term.h"
#include "accanon.h"
#include "termorder.h"
#include "symbols.h"

#include "ladrvglobais.h"


#include <iostream>
#include <iomanip>






GlobalBtm::GlobalBtm() {
    Ac_match_pos_gets=0;
    Ac_match_pos_frees=0;
    Ac_match_free_vars_pos_gets=0;
    Ac_match_free_vars_pos_frees=0;
    Btm_state_gets=0;
    Btm_state_frees=0;
}

GlobalBtm::~GlobalBtm() {
}





BtmContainer::BtmContainer() { root=NULL;}
BtmContainer::~BtmContainer(){ root=NULL;}




Ac_match_pos BtmContainer::get_ac_match_pos(void) {
  Ac_match_pos p =  (Ac_match_pos) Memory::memCNew(sizeof(struct ac_match_pos));
  LADR_GLOBAL_BTM.Ac_match_pos_gets++;
  return(p);
}

void BtmContainer::free_ac_match_pos(Ac_match_pos p) {
  Memory::memFree( (void *) p, sizeof(struct ac_match_pos));
  LADR_GLOBAL_BTM.Ac_match_pos_frees++;
}

Ac_match_free_vars_pos BtmContainer::get_ac_match_free_vars_pos(void) {
  Ac_match_free_vars_pos p = (Ac_match_free_vars_pos) Memory::memCNew(sizeof(struct ac_match_free_vars_pos));
  LADR_GLOBAL_BTM.Ac_match_free_vars_pos_gets++;
  return(p);
}


void BtmContainer::free_ac_match_free_vars_pos(Ac_match_free_vars_pos p) {
  Memory::memFree((void*) p, sizeof(struct ac_match_free_vars_pos)); 		
  LADR_GLOBAL_BTM.Ac_match_free_vars_pos_frees++;
}


Btm_state BtmContainer::get_btm_state(void) {
  Btm_state p = (Btm_state) Memory::memCNew(sizeof(struct btm_state));
  p->varnum = -1;
  p->alternative = Unif_alternative::NO_ALT;
  LADR_GLOBAL_BTM.Btm_state_gets++;
  return(p);
}


void BtmContainer::free_btm_state(Btm_state p) {
  Memory::memFree((void *) p, sizeof(struct btm_state));
  LADR_GLOBAL_BTM.Btm_state_frees++;
} 

void BtmContainer::fprint_btm_mem(ostream &o, bool heading) {
  int n;
  if (heading)
	o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct ac_match_pos);
  o<<"ac_match_pos ("<<setw(4)<<n<<")     "<<setw(11)<<LADR_GLOBAL_BTM.Ac_match_pos_gets;
  o<<setw(11)<<LADR_GLOBAL_BTM.Ac_match_pos_frees;
  o<<setw(11)<<LADR_GLOBAL_BTM.Ac_match_pos_gets - LADR_GLOBAL_BTM.Ac_match_pos_frees; 
  o<<setw(9)<< ( (LADR_GLOBAL_BTM.Ac_match_pos_gets - LADR_GLOBAL_BTM.Ac_match_pos_frees) *n)/1024<<"K"<<endl;

  n = sizeof(struct ac_match_free_vars_pos);
  o<<"ac_match_free_vars_pos ("<<setw(4)<<n<<")"<<setw(7)<<LADR_GLOBAL_BTM.Ac_match_free_vars_pos_gets;
  o<<setw(11)<<LADR_GLOBAL_BTM.Ac_match_free_vars_pos_frees;
  o<<setw(11)<<LADR_GLOBAL_BTM.Ac_match_free_vars_pos_gets - LADR_GLOBAL_BTM.Ac_match_free_vars_pos_frees;
  o<<setw(9)<< ( (LADR_GLOBAL_BTM.Ac_match_free_vars_pos_gets - LADR_GLOBAL_BTM.Ac_match_free_vars_pos_frees) *n)/1024<<"K"<<endl;
    
  n = sizeof(struct btm_state);
  o<<"btm_state   ("<<setw(4)<<n;
  o<<")        "<<setw(11)<<LADR_GLOBAL_BTM.Btm_state_gets;
  o<<setw(11)<<LADR_GLOBAL_BTM.Btm_state_frees;
  o<<setw(11)<<LADR_GLOBAL_BTM.Btm_state_gets - LADR_GLOBAL_BTM.Btm_state_frees;
  o<<setw(9)<< ( (LADR_GLOBAL_BTM.Btm_state_gets - LADR_GLOBAL_BTM.Btm_state_frees) *n)/1024<<"K"<<endl;
}  

void BtmContainer::p_btm_mem(){
  fprint_btm_mem(cout,true);
}



void BtmContainer::flatten_mult(Term t, Term *a, int *m, int *ip, int *totp, int (*comp_proc) (void *, void *)) {
  Term t1;
  int sn, i;
  TermContainer T;
  sn = SYMNUM(t);
  for (i = 0; i < ARITY(t); i++) {
    t1 = ARG(t,i);
    if (SYMNUM(t1) == sn)
      flatten_mult(t1, a, m, ip, totp, comp_proc);
    else {
      (*totp)++;
      if (*ip > 0 && (*comp_proc)(t1, a[(*ip)-1]) == (int)OrderType::SAME_AS)
	m[(*ip)-1]++;
      else {
	if (*ip >= MAX_ACM_ARGS) {
	
	  T.fprint_term(cout, t);
	  fatal::fatal_error("flatten_mult, too many arguments.");
	}
		a[*ip] = t1;
		m[*ip] = 1;
		(*ip)++;
      }
    }
  }
} 


int BtmContainer::macbv_rec(int ac_sn, Term t, Term *args2, int *mults2,int *match2, int n2, int *bound_matches, int *bp) {
  int i, available;
  TermContainer T;  
  if (!COMPLEX(t) || SYMNUM(t) != ac_sn) {
    for (i = 0; i < n2; i++) {
      available = mults2[i] - match2[i];
      if (available > 0 && T.term_ident(t, args2[i])) {
        match2[i]++;
        bound_matches[(*bp)++] = i;
        return(1);
      }
    }
    return(0);
  }
  else {
    if (!macbv_rec(ac_sn,ARG(t,0),args2,mults2,match2,n2,bound_matches,bp))   return(0);
    else  return(macbv_rec(ac_sn,ARG(t,1),args2,mults2,match2,n2, bound_matches,bp));
  }
} 






int BtmContainer::match_ac_bound_vars(int ac_sn, Term *args1, int n1,
			Term *args2, int *mults2, int *match2,
			int n2, int begin,
			int *bound_matches, int *bp, Context c1)
{
  int i, ok, vn;
  Term t;

  for (i=begin, ok=1, *bp=0; i < n1 && ok; i++) {
    vn = VARNUM(args1[i]);
    t = c1->terms[vn];
    if (t)
      ok = macbv_rec(ac_sn,t,args2,mults2,match2,n2,bound_matches,bp);
  }
  if (!ok) {
    /* Subtract any matches that were made before failure. */
    for (i = 0; i < *bp; i++)
      match2[bound_matches[i]] -= 1;
    *bp = 0;  /* Not really necessary, but helpful for debugging. */
  }
  return(ok);
}  /* match_ac_bound_vars */


void BtmContainer::set_up_free_vars(Ac_match_pos ac, Context c1) {
  Ac_match_free_vars_pos p1, p2;
  Term t;
  int i, temp;

  ac->free_first = NULL; ac->free_last = NULL;
  for (i = ac->last_a1_symbol+1; i < ac->n1; i++) {
    t = ac->args1[i];
    if (c1->terms[VARNUM(t)] == NULL) {
      /* We have a free variable. */
      for (p1=ac->free_first; p1 && p1->varnum!=VARNUM(t); p1=p1->next);
      if (p1 != NULL)
	(p1->coef)++;
      else {
	p1 = get_ac_match_free_vars_pos();
	p1->varnum = VARNUM(t);
	p1->coef = 1;
	p1->next = NULL;
	p1->prev = ac->free_last;
	if (ac->free_last)
	  ac->free_last->next = p1;
	else
	  ac->free_first = p1;
	ac->free_last = p1;
      }
    }
  }
  /* Now sort -- nonincreasing coefficients. */
  /* There won't be many, so use a quadratic sort. */
  p1 = ac->free_first;
  if (p1) {
    while (p1->next) {
      for (p2 = p1->next; p2; p2 = p2->next) {
	if (p1->coef < p2->coef) {
	  temp = p2->coef;
	  p2->coef = p1->coef;
	  p1->coef = temp;
	  temp = p2->varnum;
	  p2->varnum = p1->varnum;
	  p1->varnum = temp;
	}
      }
      p1 = p1->next;
    }
  }
} 

void BtmContainer::unbind_free_var(Ac_match_free_vars_pos pos, Context c) {
  int i, j;
  Term t, t1;
  TermContainer T;
  /* Free the temporary substitution term, if necessary. */
  /* First count how many nodes have to be deleted. */
  for (i = j = 0; i < pos->n; i++)
    if (pos->combo[i])
      j++;
  t = c->terms[pos->varnum];
  for (i = 0; i < j-1; i++) {
 
	t1 = ARG(t,1);
    T.free_term(t);
    t = t1;
  }
  /* unbind variable */
  c->terms[pos->varnum] = NULL;
} 



int BtmContainer::free_var_match(Ac_match_free_vars_pos pos, Term *args2,
		   int *mults2, int *match2, int n2, Context c1,
		   int symbol, int match_all)
{
  Term t;
  int i, j, k, n, ok, go, avail;

  t = c1->terms[pos->varnum];

  if (!t) {
    /* It is not a continuation, so set up everything.
     *
     * Loop through args2, collecting targets, combinations of which
     * can be substituted for the current variable.
     * Example: current variable is 2x; terms available for
     * matching are 4a, 3b, 2c, 1d; targets are a,a,b,c.
     */
  
  n = 0;
    for (i = 0; i < n2; i++) {
      avail = mults2[i] - match2[i];
      if (match_all && (avail % pos->coef != 0)) return 0; /* Fail, because there will be unmatched term(s) */
      j = avail / pos->coef;  /* integer division */
      for (k = 0; k < j; k++) pos->targets[n++] = i;
    }

    pos->n = n;
    if (n == 0)
      return 0;
    else {
      for (i = 0; i < n; i++)
	pos->combo[i] = 1;
    }
  }
  else {
    /* continutation */
    unbind_free_var(pos, c1);

    /* unmark args2 terms */
    for (i = 0; i < pos->n; i++)
      if (pos->combo[i]) {
		match2[pos->targets[i]] -= pos->coef;
      }

    if (match_all) {
      for (i = 0; i < pos->n; i++)
	pos->combo[i] = 0;
      return 0;
    }
    else {
      go = 1;
      while (go) {
	/* subtract 1 from  combo */
	for (i = (pos->n)-1; i >= 0 && pos->combo[i] == 0; i--)
	  pos->combo[i] = 1;
	if (i < 0)
	  return(0);
	else {
	  pos->combo[i] = 0;
	  /* Check redundancy condition. */
	  for (i = 0, ok = 1; i < (pos->n)-1 && ok; i++)
	    if (pos->targets[i] == pos->targets[i+1] &&
		pos->combo[i] < pos->combo[i+1])
	      ok = 0;
	  go = !ok;
	}
      }

      /* Now make sure that combo is not empty. */
      for (i = 0, ok = 0; i < pos->n && !ok; i++)
	ok = pos->combo[i];
      if (!ok)
	return 0;
    }
  }

  /* All is well---we have a match for the current variable. */
  /* Build a temporary substitution term, if necessary. */
  /* Note order in which it is built---this makes it AC canonical. */

  t = NULL;
  TermContainer T;
  for (i = pos->n-1; i >= 0; i--) 
    if (pos->combo[i]) {
      if (t == NULL)
	t = args2[pos->targets[i]];
      else
	t = T.build_binary_term(symbol, args2[pos->targets[i]], t);
    }

  /* Bind variable. */
  c1->terms[pos->varnum] = t;
    
  /* Mark args2 terms matched to the current variable. */
  for (i = 0; i < pos->n; i++)
    if (pos->combo[i])
      match2[pos->targets[i]] += pos->coef;

  return 1;
}  /* free_var_match */


Term BtmContainer::build_partial_term(Ac_match_pos ac) {
  int i, j, k, n;
  Term t;
  TermContainer T;
	
  t = NULL; k = 0;
  for (i = 0; i < ac->n2; i++) {
    n = ac->mults2[i] - ac->match2[i];
    for (j = 0; j < n; j++) {
      k++;
      if (!t)	t = ac->args2[i];
      else	t = T.build_binary_term(SYMNUM(ac->t1), ac->args2[i], t);
    }
  }
  ac->partial_term_size = k;
  return t;
} 


void BtmContainer::clear_partial_term(Ac_match_pos ac) {
  int i;
  Term t, t1;
  TermContainer T;   	
  t = ac->c1->partial_term;
  ac->c1->partial_term = NULL;
    
  for (i = 0; i < ac->partial_term_size - 1; i++) {
    t1 = ARG(t,1);
    T.free_term(t);
    t = t1;
  }
  ac->partial_term_size = 0;
}


int BtmContainer::match_ac(Term t1, Context c1, Term t2, Btm_state bt) {
  int status, n1, n2, total2, i, ok, a1_pos, a2_pos;
  int free_var_forward;
  Term a1, a2;
  Ac_match_pos ac;
  Ac_match_free_vars_pos free_pos, p1, p2;
  Btm_state bt1 = NULL;
  

  a1_pos = a2_pos = free_var_forward = 0;  /* to quiet compiler */
  free_pos = NULL;  /* to quiet compiler */

  if (bt->alternative == Unif_alternative::NO_ALT) {  /* initialize, get first matcher */
    ac = get_ac_match_pos();
    bt->acm = ac;
    ac->t1 = t1; ac->t2 = t2; ac->c1 = c1;
    ac->free_first = NULL; ac->partial_term_size = 0;
    n1 = 0; n2 = 0; total2 = 0; 
    Accnon::flatten(t1, ac->args1, &n1);
    flatten_mult(t2, ac->args2, ac->mults2, &n2, &total2, (int (*)(void*,void*)) TermOrder::term_compare_ncv);
    if (n1 > total2)  /* fail if t1 has more arguments */
        status = FAILURE;
    else {
      /* Assume inputs are ac_canonical, so don't sort.       */
      /* Don't bother to eliminate common arguments, because  */
      /* It usually doesn't pay off.                          */
      ac->n1 = n1; ac->n2 = n2;
      for (i = 0; i < n1; i++)
		ac->match1[i] = -1;
      for (i = 0; i < n2; i++)
		ac->match2[i] = 0;
      for (i = 0; i < n1 && !VARIABLE(ac->args1[i]); i++);
      ac->last_a1_symbol = i-1;
      a1_pos = 0; a2_pos = 0; bt1 = NULL;
      status = GO_FUNCTORS;
    }
  }
  else {  /* continuation, get next matcher */
    ac = bt->acm;
    if (bt->partial) {
      printf("WARNING: partial match_ac on continuation.\n");
      if (c1->partial_term)	clear_partial_term(ac);
    }
    n1 = ac->n1; n2 = ac->n2;
    if (n1 == 0 && n2 == 0)  /* vacuous success last time */
      status = FAILURE;
    else {
      free_pos = ac->free_last;
      free_var_forward = 0;
      status = GO_FREE_VARS;
    }
  }
    
  while (status != SUCCESS && status != FAILURE) {
    while (status == GO_FUNCTORS) {
      if (a1_pos > ac->last_a1_symbol)	status = GO_BOUND_VARS;
      else if (a1_pos < 0)	status = FAILURE;
      else {
		if (bt1) {
		/* remove arrow */
			ac->match1[a1_pos] = -1;
			ac->bt1[a1_pos] = NULL;
			ac->match2[a2_pos]--;
			/* Try for another match with this pair. */
			bt1 = match_bt_next(bt1);
			if (!bt1)  a2_pos++;
		}
		if (!bt1) {
		/* Look for a match for a1, starting with a2. */
		  a1 = ac->args1[a1_pos];
		  while (bt1 == NULL && a2_pos < ac->n2) {
			a2 = ac->args2[a2_pos];
			if (SYMNUM(a1) == SYMNUM(a2) &&	ac->match2[a2_pos] < ac->mults2[a2_pos]) bt1 = match_bt_first(a1, c1, a2, 0);
			if (bt1 == NULL)  a2_pos++;
		  }
		}
		if (bt1) {   /* We have a match: a1->a2. */
			/* draw arrow */
			ac->match1[a1_pos] = a2_pos;
			ac->bt1[a1_pos] = bt1;
			ac->match2[a2_pos]++;
			a1_pos++; a2_pos = 0; bt1 = NULL;
		}
		else {  /* back up */
			a1_pos--;
			a2_pos = ac->match1[a1_pos];
			bt1 = ac->bt1[a1_pos];
		}
      }
    }  /* while GO_FUNCTORS */
	
    if (status == GO_BOUND_VARS) {
      /* Try to macth (identically) bound variables. */
      ok = match_ac_bound_vars(SYMNUM(t1), ac->args1, n1,
			       ac->args2, ac->mults2, ac->match2, n2,
			       ac->last_a1_symbol+1, ac->bound_matches,
			       &(ac->bound_count), c1);
      if (ok) {
		free_pos = ac->free_first;
		free_var_forward = 1;
		status = GO_FREE_VARS;
      }
      else {  /* backup */
		a1_pos = ac->last_a1_symbol;
		if (a1_pos >= 0) {
			a2_pos = ac->match1[a1_pos];
			bt1 = ac->bt1[a1_pos];
		}
		status = GO_FUNCTORS;
      }
    }
    else if (status == GO_FREE_VARS) {
      if (ac->free_first == NULL) {
		set_up_free_vars(ac, c1);
		free_pos = ac->free_first;
      }
      while (free_pos) {
		if (free_var_match(free_pos, ac->args2, ac->mults2, ac->match2, ac->n2, c1, SYMNUM(ac->t1), !bt->partial && free_pos->next == NULL)) {
			free_pos = free_pos->next;
			free_var_forward = 1;
		}
		else {
			free_pos = free_pos->prev;
			free_var_forward = 0;
		}
      }
      if (free_var_forward) {
	/* Check for non-matched a2 terms. */
		for (i = 0, ok = 1; i < n2 && ok; i++) ok = ac->mults2[i] == ac->match2[i];
		if (!ok) {
		/*  Have at least 1 non-matched a2 term. */
			if (bt->partial) {
				c1->partial_term = build_partial_term(ac);
				status = SUCCESS;
			}
		else status = GO_FUNCTORS;  /* set up below */
		}
		else status = SUCCESS;
      }
      else 	status = GO_FUNCTORS;
      if (status == GO_FUNCTORS) {
		/* Unmark bound variable matches. */
		for (i = 0; i < ac->bound_count; i++)  ac->match2[ac->bound_matches[i]] -= 1;
		a1_pos = ac->last_a1_symbol;
		if (a1_pos >= 0) {
			a2_pos = ac->match1[a1_pos];
			bt1 = ac->bt1[a1_pos];
		}
      }
    }  /* if GO_FREE_VARS */
  }  /* while !SUCCESS && !FAILURE */

  if (status == SUCCESS)   bt->alternative = Unif_alternative::AC_ALT;
  else {
    /* free Memory:: */
    p1 = ac->free_first;
    while (p1) {
      p2 = p1;
      p1 = p1->next;
      free_ac_match_free_vars_pos(p2);
    }
    free_ac_match_pos(ac);
    bt->alternative = Unif_alternative::NO_ALT;
  }
  return(status == SUCCESS);
}    /* match_ac */

void BtmContainer::match_ac_cancel(Ac_match_pos ac){
  Ac_match_free_vars_pos p1, p2;
  int i;
    
  for (i = 0; i <= ac->last_a1_symbol; i++)  match_bt_cancel(ac->bt1[i]);
  p1 = ac->free_first;
  while (p1) {
    unbind_free_var(p1, ac->c1);
    p2 = p1;
    p1 = p1->next;
    free_ac_match_free_vars_pos(p2);
  }
  if (ac->partial_term_size > 0)  clear_partial_term(ac);
  free_ac_match_pos(ac);
}  /* match_ac_cancel */


Btm_state BtmContainer::match_bt_backup(Btm_state bt1) {
  Btm_state bt2, bt3;

  while (bt1 != NULL && bt1->alternative == Unif_alternative::NO_ALT) {

    if (bt1->cb) {  /* unbind variable */
      bt1->cb->terms[bt1->varnum] = NULL;
      bt1->cb->contexts[bt1->varnum] = NULL;
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
		free_btm_state(bt3);
      }

      if (bt1)
		bt1->first_child = bt1->last_child = NULL;
    }
  }
  return(bt1);
} 


Btm_state BtmContainer::match_bt_guts(Btm_state bt1) {
  Term t1, t2;
  Context c1;
  int vn1, status;
  Btm_state bt2, bt3;
  TermContainer T;
  SymbolContainer S;

  status = GO;

  while (status == GO) {

    t1 = bt1->t1;
    t2 = bt1->t2;
    c1 = bt1->c1;

    if (bt1->alternative == Unif_alternative::COMM_ALT) {
      if (match_commute(t1, c1, t2, bt1)) status = POP;
      else status = BACKTRACK;
    }
    else if (bt1->alternative == Unif_alternative::AC_ALT) {
      if (match_ac(t1, c1, t2, bt1)) status = POP;
      else	status = BACKTRACK;
    }
    else if (VARIABLE(t1)) {
      vn1 = VARNUM(t1);
      if (c1->terms[vn1]) {
		if (T.term_ident(c1->terms[vn1], t2)) status = POP;
		else status = BACKTRACK;
      }
      else {
			BIND_BT(vn1, c1, t2, NULL, bt1)
			status = POP;
      }
    }
	
    else if (VARIABLE(t2)) status = BACKTRACK;
    else if (SYMNUM(t1) != SYMNUM(t2)) status = BACKTRACK;
    else if (CONSTANT(t1)) status = POP;
	
    else {  /* both COMPLEX with same symbol (and same arity) */
			int arity = ARITY(t1);
			if (arity == 2 && S.is_commutative(SYMNUM(t1))) {
				if (match_commute(t1, c1, t2, bt1)) status = POP;
				else status = BACKTRACK;
			}
      else if (arity == 2 && S.is_assoc_comm(SYMNUM(t1))) {
		if (match_ac(t1, c1, t2, bt1))	  status = POP;
		else status = BACKTRACK;
      }
      else {
		int i;
		/* Set up children corresponding to args of <t1,t2>. */
		/* Order not important for correctness. */
		/* AC kids last for efficiency, but keep in order otherwise. */
		bt3 = NULL;
		for (i = 0; i < arity; i++) {
			bt2 = get_btm_state();
			bt2->t1 = ARG(t1,i);
			bt2->t2 = ARG(t2,i);
			bt2->c1 = c1;
			bt2->parent = bt1;

			if (S.is_assoc_comm(SYMNUM(bt2->t1))) {
				/* insert at end */
				bt2->prev = bt1->last_child;
				if (bt1->last_child) bt1->last_child->next = bt2;
				else bt1->first_child = bt2;
				bt1->last_child = bt2;
			}
			else {
				if (bt3) {
					/* insert after bt3 */
					bt2->next = bt3->next;
					bt2->prev = bt3;
					bt3->next = bt2;
					if (bt2->next) bt2->next->prev = bt2;
					else bt1->last_child = bt2;
				}
				else {
				/* insert at beginning */
					bt2->next = bt1->first_child;
					if (bt2->next) bt2->next->prev = bt2;
					else bt1->last_child = bt2;
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
      while (!bt1->next && bt1->parent)	bt1 = bt1->parent;
      if (!bt1->next)status = SUCCESS;
      else {
		bt1 = bt1->next;
		status = GO;
      }
    }
    else if (status == BACKTRACK) {
      bt1 = match_bt_backup(bt1);
      if (bt1)	status = GO;
      else	status = FAILURE;
    }
  }
  return(bt1);
}  /* match_bt_guts */


int BtmContainer::match_commute(Term t1, Context c1, Term t2, Btm_state bt) {
  Btm_state bt1, bt2;

  if (bt->alternative == Unif_alternative::NO_ALT) {  /* first call */
    bt->alternative = Unif_alternative::COMM_ALT;
    bt->flipped = 0;

    /* Set up 2 subproblems, then match guts. */

    bt1 = get_btm_state();  bt2 = get_btm_state();
    bt1->next = bt2; bt2->prev = bt1;
    bt1->c1 = c1;
    bt2->c1 = c1;
    bt1->t1 = ARG(t1,0); bt1->t2 = ARG(t2,0);
    bt2->t1 = ARG(t1,1); bt2->t2 = ARG(t2,1);

    bt->position_bt = match_bt_guts(bt1);
  }
  else  /* continuation */
    bt->position_bt = match_bt_next(bt->position_bt);

  if (!bt->position_bt && !bt->flipped) {

    /* Set up 2 subproblems, with t2 flipped, then match guts. */

    bt1 = get_btm_state();  bt2 = get_btm_state();
    bt1->next = bt2; bt2->prev = bt1;
    bt1->c1 = c1;
    bt2->c1 = c1;
    bt1->t1=ARG(t1,0); bt1->t2=ARG(t2,1);
    bt2->t1=ARG(t1,1); bt2->t2=ARG(t2,0);

    bt->flipped = 1;
    bt->position_bt = match_bt_guts(bt1);
  }

  if (bt->position_bt)
    return(1);
  else {
    bt->alternative = Unif_alternative::NO_ALT;
    return(0);
  }
    
}  /* match_commute */



void BtmContainer::p_acm(Ac_match_pos ac) {
  int i;
  Ac_match_free_vars_pos p;
  TermContainer T;
  
  printf("Ac_match_pos %p.\n", ac);
  printf("t1: "); T.p_term(ac->t1);
  printf("t2: "); T.p_term(ac->t2);
  
  for (i = 0; i < ac->n1; i++) {
    T.fprint_term(cout, ac->args1[i]);
    printf(" %d ",ac->match1[i]);
  }
  printf("\n");
  for (i = 0; i < ac->n2; i++) {
    T.fprint_term(cout, ac->args2[i]);
    printf(" <%d,%d> ",ac->mults2[i],ac->match2[i]);
  }
  printf("\n");

  printf("last_a1_symbol=%d.\n",ac->last_a1_symbol);
  printf("free vars list <symbol,coef>:\n");
  for (p = ac->free_first; p; p = p->next) {
    printf("<%d,%d>, ", p->varnum, p->coef);
    for (i = 0; i < p->n; i++) {
      T.fprint_term(cout,ac->args2[p->targets[i]]);
      printf(":%d ",p->combo[i]);
    }
    printf("\n");
  }
  printf("\n");
}  /* p_acm */


void BtmContainer::p_btm_state(Btm_state bt){
  TermContainer T;
  UnifyContainer U;
  printf("\nBtm node %p.\n", bt);
  printf("t1: "); T.p_term(bt->t1);
  printf("t2: "); T.p_term(bt->t2);
  printf("c1: "); U.p_context(bt->c1);
  printf("varnum: %d\n", bt->varnum);
  printf("cb: "); U.p_context(bt->c1);
  printf("alternative: %d\n", (int) bt->alternative);
  printf("flipped: %d\n", (int) bt->alternative);
  printf("position_bt: %p\n", bt->position_bt);
  printf("partial: %d\n", bt->partial);
  p_acm(bt->acm);
}

Btm_state BtmContainer::match_bt_first(Term t1, Context c1, Term t2, int partial) {
  Btm_state bt;
  bt = get_btm_state();
  bt->t1 = t1; bt->t2 = t2; bt->c1 = c1;
  bt->partial = partial;
  return(match_bt_guts(bt));

} 


Btm_state BtmContainer::match_bt_next(Btm_state bt1){
  /* Go to last node in tree, then back up to a node with an alternative. */

  while (bt1->next)
    bt1 = bt1->next;
  while (bt1->last_child)
    bt1 = bt1->last_child;

  bt1 = match_bt_backup(bt1);

  if (bt1)
    return(match_bt_guts(bt1));
  else
    return NULL;
}

void BtmContainer::match_bt_cancel(Btm_state bt){
  Btm_state bt1, bt2;

  for (bt1 = bt; bt1 != NULL; ) {

    match_bt_cancel(bt1->first_child);
	
    if (bt1->alternative == Unif_alternative::COMM_ALT)
      /* match_bt_guts leaves us at the second child. */
      match_bt_cancel(bt1->position_bt->prev);
    else if (bt1->alternative == Unif_alternative::AC_ALT) {
      match_ac_cancel(bt1->acm);
    }
    else if (bt1->cb) {
      bt1->cb->terms[bt1->varnum] = NULL;
      bt1->cb->contexts[bt1->varnum] = NULL;
    }
    bt2 = bt1;
    bt1 = bt1->next;
    free_btm_state(bt2);
  }
} 
