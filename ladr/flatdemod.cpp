#include "flatdemod.h"
#include "flatterm.h"


int Flatdemod::Fdemod_attempts = 0;
int Flatdemod::Fdemod_rewrites = 0;



Flatterm Flatdemod::fapply_demod(Term t, Context c) {
  Flatterm f;
  FlattermContainer F; 	
  if (VARIABLE(t)) {
    f = F.copy_flatterm((Flatterm) c->terms[VARNUM(t)]);
    f->reduced_flag = true;
  }
  else {
    int n = 1;
    int i;
    Flatterm _end;
    f = F.get_flatterm();
    f->private_symbol = t->private_symbol;
    ARITY(f) = ARITY(t);
    _end = f;

    for (i = 0; i < ARITY(t); i++) {
      Flatterm a = fapply_demod(ARG(t,i), c);
      n += a->size;
      _end->next = a;
      a->prev = _end;
      _end = a->end;
    }
    f->size = n;
    f->end = _end;
  }
  return f;
} 

#if 0

void Flatdemod::maybe_unbind(Flatterm f, Context subst) {
  if (f->varnum_bound_to >= 0) {
    subst->terms[f->varnum_bound_to] = NULL;
    f->varnum_bound_to = -1;
  }
}  /* maybe_unbind */
#endif

Plist Flatdemod::discrim_flat_retrieve_leaf(Flatterm fin, Discrim root,Context subst, Flatterm *ppos) {
  Flatterm f = NULL;
  Discrim d = NULL;
  FlattermContainer F;
  bool status = (bool) Flatdemod_mode::_GO;

  if (root) {  /* first call */
    d = root->u.kids;
    f = fin;
    if (d == NULL)
      return NULL;
    else
      status = (bool) Flatdemod_mode::_GO;
  }
  else {
    f = *ppos;
    status = (bool) Flatdemod_mode::_BACKTRACK;
  }

  while (true) {

    if (status == (bool) Flatdemod_mode::_BACKTRACK) {
      while (f != fin && f->alternative == NULL) {
		MAYBE_UNBIND(f, subst);
		f = f->prev;
      }
      if (f == fin)	return NULL;  /* fail */
      else {
			MAYBE_UNBIND(f, subst);
			d = f->alternative;
			f->alternative = NULL;
			status = (bool) Flatdemod_mode::_GO;
      }
    }  /* backtrack */

    if (DVAR(d)) {
      int varnum = d->symbol;
      Flatterm b = (Flatterm) subst->terms[varnum];
      if (b) {
		f->alternative = d->next;
		if (F.flatterm_ident(f, b))
		f = f->end;  /* already bound to identical term */
	  else
		status = (bool) Flatdemod_mode::_BACKTRACK;  /* already bound to something else */
      }
      else {
			subst->terms[varnum] = (Term) f;
			f->varnum_bound_to = varnum;
			f->alternative = d->next;
			f = f->end;
      }
    }
    else if (VARIABLE(f))    status =(bool) Flatdemod_mode::_BACKTRACK;
    else {
      int symnum = SYMNUM(f);
      Discrim dd = NULL;
      while (d && d->symbol < symnum) {
		dd = d;
		d = d->next;
      }
      if (!d || d->symbol != symnum) status = (bool) Flatdemod_mode::_BACKTRACK;
    }
    
    if (status == (bool) Flatdemod_mode::_GO) {
      if (f == fin->end) {
		*ppos = f;
		return d->u.data;  /* succeed */
      }
      else {
		f = f->next;
		d = d->u.kids;
      }
    }  /* go */
  }  /* while */
}

void * Flatdemod::discrim_flat_retrieve_first(Flatterm f, Discrim root,  Context subst, Discrim_pos *ppos) {
  Plist tp;
  Flatterm f2 = NULL;
  DiscrimContainer D;
  tp = discrim_flat_retrieve_leaf(f, root, subst, &f2);
  if (tp == NULL)
    return NULL;
  else {
    Discrim_pos pos = D.get_discrim_pos();
    pos->query = f;
    pos->subst = subst;
    pos->data = tp;
    pos->backtrack = f2;
    *ppos = pos;
    return tp->v;
  }
}

void * Flatdemod::discrim_flat_retrieve_next(Discrim_pos pos) {
  Plist tp;
  DiscrimContainer D;  
  tp = pos->data->next;
  if (tp != NULL) {  /* if any more terms in current leaf */
    pos->data = tp;
    return tp->v;
  }
  else {  /* try for another leaf */
    tp = discrim_flat_retrieve_leaf((Flatterm) pos->query, NULL, pos->subst,  (Flatterm *) &(pos->backtrack));
    if (tp != NULL) {
      pos->data = tp;
      return tp->v;
    }
    else {
      D.free_discrim_pos(pos);
      return NULL;
    }
  }
} 


void Flatdemod::discrim_flat_cancel(Discrim_pos pos) {
  Flatterm f = (Flatterm) pos->backtrack;
  Flatterm query = (Flatterm) pos->query;
  DiscrimContainer D;

  while (f != query->prev) {
    if (f->varnum_bound_to >= 0) {
      pos->subst->terms[f->varnum_bound_to] = NULL;
      f->varnum_bound_to = -1;
    }
    f->alternative = NULL;
    f = f->prev;
  }
  D.free_discrim_pos(pos);
} 

Flatterm Flatdemod::fdemod(Flatterm f, Discrim root, Context subst,
		int *step_limit,
		int size_limit,
		int *current_size,
		int *sequence, I3list *just_head, bool lex_order_vars) {
  
  FlattermContainer F;
  if (*step_limit == 0 || *current_size > size_limit)    return f;
  else if (VARIABLE(f))   return f;
  else if (f->reduced_flag) {
    (*sequence) += F.flatterm_count_without_vars(f);
    return f;
  }
  else {
    int sequence_save = *sequence;
    { /* demodulate subterms */
      Flatterm arg = f->next;
      Flatterm _end = f;
      int n = 1;
      int i;
      for (i = 0; i < ARITY(f); i++) {
	    Flatterm next = arg->end->next;
	    Flatterm a2 = fdemod(arg, root, subst, step_limit,
			     size_limit, current_size,
			     sequence, just_head, lex_order_vars);
	    n += a2->size;
	    _end->next = a2;
	    a2->prev = _end;
	    _end = a2->end;
	    arg = next;
    }
      f->size = n;
      f->end = _end;
      f->prev = _end->next = NULL;  /* helpful for debugging */
    } /* end: demodulate subterms */

    if (*current_size > size_limit)     return f;  /* size limit has been reached */
    else if (*step_limit == 0)      return f;  /* step limit has been reached */
    else {
      /* try to rewrite top */
      Discrim_pos dpos;
      Term candidate =(Term) discrim_flat_retrieve_first(f, root, subst, &dpos);
      bool rewrite = false;

      Fdemod_attempts++;
      (*sequence)++;

      while (candidate && !rewrite) {
		Topform demodulator = (Topform) candidate->container;
		Term atom = demodulator->literals->atom;
		Term alpha = ARG(atom, 0);
		Term beta = ARG(atom, 1);
		bool match_left = (candidate == alpha);
		Term other = (match_left ? beta : alpha);
		Flatterm contractum = fapply_demod(other, subst);
        Parautil Pu;
        TermOrder T;
		if (Pu.oriented_eq(atom))
		rewrite = true;
		else if (T.flat_greater(f, contractum, lex_order_vars)) {
			rewrite = true;
	  /*
	    printf("kbo=%d ", rewrite); p_flatterm(f);
	    printf("      "    ); p_flatterm(contractum);
	    printf("\n");
	  */
	}
	else
	  rewrite = false;
	
	if (rewrite) {
	  int increase_in_size = contractum->size - f->size;
	  (*current_size) += increase_in_size;  /* likely to be negative */
	  (*step_limit)--;

	  Fdemod_rewrites++;
	  discrim_flat_cancel(dpos);
	  F.zap_flatterm(f);
	  I3listContainer I3;
	  I3.set_head(*just_head);
	  *just_head = I3.i3list_prepend(demodulator->id,
				      *sequence,
				      match_left ? 1 : 2);
	  *sequence = sequence_save;
	  f = fdemod(contractum, root, subst, step_limit,
		     size_limit, current_size,
		     sequence, just_head, lex_order_vars);
	}
	else {
	  F.zap_flatterm(contractum);
	  candidate = (Term)discrim_flat_retrieve_next(dpos);
	}
      }
    } /* end: try to rewrite top */
    f->reduced_flag = true;
    return(f);
  }
} 

Term Flatdemod::fdemodulate(Term t, Discrim root, int *step_limit, int *increase_limit, int *sequence,	 I3list *just_head, bool lex_order_vars) {
  FlattermContainer F;
  UnifyContainer U;
  TermContainer T;
  Flatterm f = F.term_to_flatterm(t);
  
  Context subst = U.get_context();
  
  int current_size = f->size;
  int size_limit = (*increase_limit==INT_MAX) ? INT_MAX : current_size + *increase_limit;
  Flatterm f2 = fdemod(f, root, subst, step_limit, size_limit, &current_size,  sequence, just_head, lex_order_vars);
  
  U.free_context(subst);

  if (current_size > size_limit)
    *increase_limit = -1;  /* lets callers know that limit has been exceeded */

  if (*just_head == NULL) {
    F.zap_flatterm(f2);
    return t;
  }
  else {
    Term t2 = F.flatterm_to_term(f2);
    F.zap_flatterm(f2);
    T.zap_term(t);
    return t2;
  }
} 

int Flatdemod::fdemod_attempts() {
  return Fdemod_attempts;
}

int Flatdemod::fdemod_rewrites() {
  return Fdemod_rewrites;
} 


void Flatdemod::fdemod_clause(Topform c, Mindex idx,  int *step_limit, int *increase_limit, bool lex_order_vars) {
  TopformContainer TF;
  MindexContainer M;
  if (M.mindex_empty(idx))    return;
  else {
    Literals lit;
	JustContainer J;
    I3list steps = NULL;
    /* The "sequence" variable counts the (nonvariable) subterms that
       are visited as we demodulate.  Bottom-up, left-to-right.  When
       a term is rewritten, the justification is a triple of integers:
       <demodulator-id, sequence-number, direction>.  The sequence number
       n means that the n-th node is rewritten, counting as if this
       is the only demodulation step applied to the clause.
     */
    int sequence = 0;
    for (lit = c->literals; lit != NULL; lit = lit->next)
      lit->atom = fdemodulate(lit->atom, idx->discrim_tree, step_limit, increase_limit,   &sequence, &steps, lex_order_vars);
	
    I3listContainer I3;
    TF.upward_clause_links(c);
    if (steps != NULL) {
      I3.set_head(steps);
      steps = I3.reverse_i3list();
      c->justification = J.append_just(c->justification, J.demod_just(steps));
    }
  }
} 
