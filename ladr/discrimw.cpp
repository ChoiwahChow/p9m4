#include "discrimw.h"
#include "memory.h"
#include "symbols.h"
#include "fatal.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>




GlobalDiscrimW::GlobalDiscrimW() {
    Flat_gets=0;
    Flat_frees=0;
}

GlobalDiscrimW::~GlobalDiscrimW() {
}


DiscrimWContainer::DiscrimWContainer()
{

    root=NULL;
}

DiscrimWContainer::~DiscrimWContainer()
{
    root=NULL;
}

Flat DiscrimWContainer::get_flat(void) {
  Flat p = (Flat ) Memory::memCNew(sizeof(struct flat));
  LADR_GLOBAL_DISCRIM_W.Flat_gets++;

#if 0  /* assume these will be set by the caller? */
  p->t = NULL;
  p->prev = NULL;
  p->last = NULL;
#endif

  p->next = NULL;
  p->alternatives = NULL;
  p->bound = 0;
  p->place_holder = 0;
  p->commutative = 0;
  p->flip = 0;
  return(p);
} 

void DiscrimWContainer::free_flat(Flat p) {
  Memory::memFree((void *)p, sizeof(struct flat));
  LADR_GLOBAL_DISCRIM_W.Flat_frees++;
}

void DiscrimWContainer::fprint_discrimw_mem(ostream &o, bool heading)
{
  int n;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;

  n = sizeof(struct flat);
  o<<"flat          ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_DISCRIM_W.Flat_gets;
  o<<setw(11)<<LADR_GLOBAL_DISCRIM_W.Flat_frees;
  o<<setw(11)<<LADR_GLOBAL_DISCRIM_W.Flat_gets - LADR_GLOBAL_DISCRIM_W.Flat_frees;
  o<<setw(9)<<((LADR_GLOBAL_DISCRIM_W.Flat_gets - LADR_GLOBAL_DISCRIM_W.Flat_frees) *n)/1024<<"K"<<endl;
  
}


void DiscrimWContainer::p_discrimw_mem(void) {
  fprint_discrimw_mem(cout, true);
}

Flat DiscrimWContainer::check_flat(Flat f) {
  Flat last;
  int i, arity;

  if (f->next != NULL && f->next->prev != f)
		cerr<<"check_flat: next-prev error"<<endl;

  if (f->place_holder) arity = 0;
  else arity = ARITY(f->t);
  
  last = f;
  for (i = 0; i < arity; i++) 
    last = check_flat(last->next);
  if (f->last != last) cerr<<"check_flat: last is wrong"<<endl;
  return last;
}

void p_flat(Flat f) {
  SymbolContainer S;
  while (f != NULL) {
    cout<<(VARIABLE(f->t) ? "*" : S.sn_to_str(SYMNUM(f->t)));
	if (f->place_holder) cout<<"[]";
    f = f->next;
    if (f != NULL) cout<<"-";
  }
  cout<<endl;
}


void DiscrimWContainer::flip_flat(Flat f) {
  /* Assume f is binary. */
  Flat arg1 = f->next;
  Flat arg2 = arg1->last->next;
  Flat after = f->last->next;
  Flat p;
  
  f->next = arg2;
  arg2->last->next = arg1;
  arg1->last->next = after;
  f->last = arg1->last;
  arg2->prev = f;
  arg1->prev = arg2->last;
  if (after != NULL)
    after->prev = arg1->last;

  /* arg2->last is the old end of f */

  for (p = f->prev; p != NULL; p = p->prev)
    if (p->last == arg2->last)
      p->last = f->last;
#if 0
  for (p = f; p->prev != NULL; p = p->prev);
  check_flat(p);
#endif
} 


int DiscrimWContainer::num_ac_args(struct term *t, int symbol) {
  if (!COMPLEX(t) || SYMNUM(t) != symbol) return 1;
  else  return (num_ac_args(ARG(t,0), symbol) +   num_ac_args(ARG(t,1), symbol));
}

int DiscrimWContainer::num_ac_nv_args(struct term *t, int symbol) {
  if (!COMPLEX(t) || SYMNUM(t) != symbol)  return (VARIABLE(t) ? 0 : 1);
  else  return (num_ac_nv_args(ARG(t,0), symbol) +  num_ac_nv_args(ARG(t,1), symbol));
}


void DiscrimWContainer::print_discrim_wild_tree(ostream &o, Discrim d, int n, int depth) {
  int arity, i;
  SymbolContainer S;

  for (i = 0; i < depth; i++) o<<" -";
  if (depth == 0) o<<endl<<"root";
  else if (d->type == DiscriminationTreeNode::AC_ARG_TYPE) o<<"AC "<<d->symbol<<" args";
  else if (d->type == DiscriminationTreeNode::AC_NV_ARG_TYPE) o<<"AC "<<d->symbol<<" NV args";
  else if (DVAR(d)) o<<"*";
  else o<<S.sn_to_str(d->symbol);
  o<<" ("<<d<<")";

  if (n == 0) {
    Plist p;
    for (i = 0, p = d->u.data; p; i++, p = p->next);
	o<<": leaf has "<<i<<" objects"<<endl;
  }
  else {
    Discrim d1;
	o<<endl;
    for (d1 = d->u.kids; d1 != NULL; d1 = d1->next) {
      if (d1->type == DiscriminationTreeNode::AC_ARG_TYPE || d1->type == DiscriminationTreeNode::AC_NV_ARG_TYPE)
	arity = 0;
      else if (DVAR(d1))
	arity = 0;
      else
	arity = S.sn_to_arity(d1->symbol);
      print_discrim_wild_tree(o, d1, n+arity-1, depth+1);
    }
  }
}


void DiscrimWContainer::fprint_discrim_wild_index(ostream &o, Discrim d)  {
  print_discrim_wild_tree(o, d, 1, 0);
} 


void DiscrimWContainer::p_discrim_wild_index(Discrim d) {
  fprint_discrim_wild_index(cout, d);
}

Discrim DiscrimWContainer::discrim_wild_insert_ac(Term t, Discrim d) {
  int num_args, num_nv_args;
  Discrim d1, d2, dnew, prev;
  DiscrimContainer D;
  num_args = num_ac_args(t, SYMNUM(t));
  for (d1 = d->u.kids, prev = NULL;  d1 && d1->symbol < num_args; prev = d1, d1 = d1->next);
  if (d1 == NULL || d1->symbol != num_args) {
    dnew = D.get_discrim();
    dnew->type = DiscriminationTreeNode::AC_ARG_TYPE;
    dnew->symbol = num_args;
    dnew->next = d1;
    if (prev != NULL)
      prev->next = dnew;
    else
      d->u.kids = dnew;
    d1 = dnew;
  }

  num_nv_args = num_ac_nv_args(t, SYMNUM(t));

  for (d2 = d1->u.kids, prev = NULL; d2 && d2->symbol < num_nv_args; prev = d2, d2 = d2->next);
  if (!d2 || d2->symbol != num_nv_args) {
    dnew = D.get_discrim();
    dnew->type = DiscriminationTreeNode::AC_NV_ARG_TYPE;
    dnew->symbol = num_nv_args;
    dnew->next = d2;
    if (prev != NULL)
      prev->next = dnew;
    else
      d1->u.kids = dnew;
    d2 = dnew;
  }
  return d2;
} 

Discrim DiscrimWContainer::discrim_wild_insert_rec(Term t, Discrim d) {
  Discrim d1, prev, d2;
  int sym;
  DiscrimContainer D;
  SymbolContainer S;

  if (VARIABLE(t)) {
    d1 = d->u.kids;
    if (d1 == NULL || !DVAR(d1)) {
      d2 = D.get_discrim();
      d2->type = DiscriminationTreeNode::DVARIABLE;
      d2->symbol = 0;    /* HERE */
      d2->next = d1;
      d->u.kids = d2;
      return d2;
    }
    else  /* found node */
      return d1;
  }

  else {  /* constant || complex */
    d1 = d->u.kids;
    prev = NULL;
    /* arities fixed: handle both NAME and COMPLEX */
    sym = SYMNUM(t);
    if (d1 && DVAR(d1)) {  /* skip variable */
      prev = d1;
      d1 = d1->next;
    }
    while (d1 && d1->symbol < sym) {
      prev = d1;
      d1 = d1->next;
    }
    if (d1 == NULL || d1->symbol != sym) {
      d2 = D.get_discrim();
      d2->type = DiscriminationTreeNode::DRIGID;
      d2->symbol = sym;
      d2->next = d1;
      d1 = d2;
    }
    else
      d2 = NULL;  /* new node not required at this level */

    if (S.is_assoc_comm(SYMNUM(t))) {
      d1 = discrim_wild_insert_ac(t, d1);
    }
    else {
      int i;
      for (i = 0; i < ARITY(t); i++)
	d1 = discrim_wild_insert_rec(ARG(t,i), d1);
    }

    if (d2 != NULL) {  /* link in new subtree (possibly a leaf) */
      if (prev == NULL)
	d->u.kids = d2;
      else
	prev->next = d2;
    }
	    
    return d1;  /* d1 is leaf corresp. to end of input term */
  }
}  

Discrim DiscrimWContainer::discrim_wild_end(Term t, Discrim d, Plist *path_p) {
  Discrim d1;
  Plist p;
  int sym;
  PlistContainer P;
  SymbolContainer S;

  /* add current node to the front of the path list. */

  p = P.get_plist();
  p->v = d;
  p->next = *path_p;
  *path_p = p;

  if (VARIABLE(t)) {
    d1 = d->u.kids;
    if (d1 && DVAR(d1))
      return d1;
    else
      return NULL;
  }

  else {  /* constant || complex */
    d1 = d->u.kids;
    sym = SYMNUM(t);
    if (d1 && DVAR(d1))  /* skip variables */
      d1 = d1->next;
    while (d1 && d1->symbol < sym)
      d1 = d1->next;

    if (d1 == NULL || d1->symbol != sym)
      return NULL;
    else if (S.is_assoc_comm(SYMNUM(t))) {
      int num_args, num_nv_args;
      Discrim d2, d3;
		
      num_args = num_ac_args(t, SYMNUM(t));
      num_nv_args = num_ac_nv_args(t, SYMNUM(t));
		
      for (d2 = d1->u.kids; d2 && d2->symbol != num_args; d2 = d2->next);
      if (d2 == NULL)
	return NULL;
      else {
	for (d3 = d2->u.kids; d3 && d3->symbol != num_nv_args; d3 = d3->next);
	if (d3 == NULL)
	  return NULL;
	else {
	  p = P.get_plist();
	  p->v = d1;
	  p->next = *path_p;
	  *path_p = p;
	  p = P.get_plist();
	  p->v = d2;
	  p->next = *path_p;
	  *path_p = p;
	  return d3;
	}
      }
    }
    else {
      int i;
      for (i = 0; d1 && i < ARITY(t); i++)	d1 = discrim_wild_end(ARG(t,i), d1, path_p);
      return d1;
    }
  }
} 


void DiscrimWContainer::discrim_wild_delete(Term t, Discrim root, void *object) {
  Discrim end, i2, i3, parent;
  Plist tp1, tp2;
  Plist isp1, path;
  PlistContainer P;
  DiscrimContainer D;

    /* First find the correct leaf.  path is used to help with  */
    /* freeing nodes, because nodes don't have parent pointers. */

  path = NULL;
  end = discrim_wild_end(t, root, &path);
  if (end == NULL) {
    fatal::fatal_error("discrim_wild_delete, cannot find end.");
  }

    /* Free the pointer in the leaf-list */

  tp1 = end->u.data;
  tp2 = NULL;
  while(tp1 && tp1->v != object) {
    tp2 = tp1;
    tp1 = tp1->next;
  }
  if (tp1 == NULL) {
    fatal::fatal_error("discrim_wild_delete, cannot find term.");
  }

  if (tp2 == NULL)
    end->u.data = tp1->next;
  else
    tp2->next = tp1->next;
  P.free_plist(tp1);

  if (!end->u.data) {
    /* free tree nodes from bottom up, using path to get parents */
    end->u.kids = NULL;  /* not really necessary */
    isp1 = path;
    while (!end->u.kids && end != root) {
      parent = (Discrim) isp1->v;
      isp1 = isp1->next;
      i2 = parent->u.kids;
      i3 = NULL;
      while (i2 != end) {
	i3 = i2;
	i2 = i2->next;
      }
      if (i3 == NULL)
	parent->u.kids = i2->next;
      else
	i3->next = i2->next;
      D.free_discrim(i2);
      end = parent;
    }
  }

  /* free path list */

  while (path) {
    isp1 = path;
    path = path->next;
    P.free_plist(isp1);
  }
}

void DiscrimWContainer::discrim_wild_insert(Term t, Discrim root, void *object) {
  Discrim d;
  Plist gp1, gp2;
  PlistContainer P;

  d = discrim_wild_insert_rec(t, root);
  gp1 = P.get_plist();
  gp1->v = object;
  gp1->next = NULL;

  /* Install at end of list. */
  if (d->u.data == NULL)
    d->u.data = gp1;
  else {
    for (gp2 = d->u.data; gp2->next; gp2 = gp2->next);
    gp2->next = gp1;
  }
} 


void DiscrimWContainer::discrim_wild_update(Term t, Discrim root, void *object, Indexop op) {
  if (op == Indexop::INSERT)
    discrim_wild_insert(t, root, object);
  else
    discrim_wild_delete(t, root, object);
} 

Plist DiscrimWContainer::discrim_wild_retrieve_leaf(Term t_in, Discrim root, Flat *ppos) {
  Flat f, f1, f2;
  Flat f_save = NULL;
  Term t;
  Discrim d = NULL;
  int symbol, status;
  SymbolContainer S;
  TermContainer T;

  f = *ppos;  /* Don't forget to reset before return. */
  t = t_in;

  if (t != NULL) {  /* if first call */
    d = root->u.kids;
    if (d != NULL) {
      f = get_flat();
      f->t = t;
      f->last = f;
      f->prev = NULL;
      f->next = NULL;
      f->place_holder = COMPLEX(t);
      status = GO;
    }
    else
      status = FAILURE;
  }
  else
    status = BACKTRACK;

  while (status == GO || status == BACKTRACK) {

    /* Three things determine the state at this point.
     * 1. d is the current node in the discrimination tree.
     * 2. f is the current node in the stack of flats.
     * 3. status is either GO or BACKTRACK.
     *
     * Commutative symbols:
     * This is more complicated than it needs to be, because
     * alternatives are handled awkwardly.
     * flip == 0 means that we haven't descended into the commutative term.
     * flip == 1 means that we are in the unflipped commutative term, and
     * flip == 2 means that we are in the flipped commutative term.
     */

    if (status == BACKTRACK) {
#if 0
      printf("backtracking\n");
#endif
      /* Back up to a place with an aternate branch. */
      while (f != NULL  && f->alternatives == NULL) {
	if (f->commutative && f->flip == 2) {
	  /* commutative with no alternative */
	  flip_flat(f);   /* un-flip */
#if 0
	  printf("AFTER un-flip: "); p_flat(f);
#endif
	  f->flip = 0;
	}
	f_save = f;
	f = f->prev;
      }

      if (f != NULL) {
#if 0
	printf("alternative: (%x)\n", (unsigned) f->alternatives);
#endif
      if (f->commutative && f->flip == 1) {
	flip_flat(f);   /* flip */
#if 0
	printf("AFTER flip: "); p_flat(f);
#endif
	f->flip = 2;
      }


	d = f->alternatives;
	f->alternatives = NULL;
	status = GO;
      }
      else {
	/* Free stack of flats and fail. */
	while (f_save != NULL) {
	  f1 = f_save;
	  f_save = f_save->next;
	  free_flat(f1);
	}
	status = FAILURE;
      }
    }  /* backtrack */

    if (status == GO) {
#if 0
      printf("go with (%x) %s\n", (unsigned) d,
	     DVAR(d) ? "*" : sn_to_str(d->symbol));
#endif
      if (d && d->type == DiscriminationTreeNode::AC_ARG_TYPE) {
	if (d->symbol <= f->num_ac_args)
	  f->alternatives = d->next;
	else
	  status = BACKTRACK;
      }
      else if (d && d->type == DiscriminationTreeNode::AC_NV_ARG_TYPE) {
	if (d->symbol <= f->num_ac_nv_args)
	  f->alternatives = d->next;
	else
	  status = BACKTRACK;
      }
      else if (d && DVAR(d)) {
	/* push alternatives */
	f->alternatives = d->next;
	f = f->last;
      }
      else if (VARIABLE(f->t))
	status = BACKTRACK;
      else {
	symbol = SYMNUM(f->t);
	while (d && d->symbol < symbol)
	  d = d->next;

	if (!d || d->symbol != symbol)
	  status = BACKTRACK;
	else {
	  if (f->place_holder) {

	    /* Insert skeleton in place_holder.  This is tricky, because 
	     * someone's "last" pointer may be pointing to f.  Therefore,
	     * f becomes the last argument of the skeleton.
	     */

	    if (S.is_assoc_comm(SYMNUM(f->t))) {  /* AC case */
	      f1 = get_flat();
	      f1->t = f->t;
	      f1->prev = f->prev;
	      f1->last = f;
	      if (f1->prev)
		f1->prev->next = f1;

	      f2 = get_flat();
	      f2->prev = f1;
	      f2->last = f2;
	      f2->next = f;
	      f->prev = f2;
	      f1->next = f2;
	      f->last = f;

	      /* Now, f2 is the AC_ARGS node, and f is the AC_NV_ARGS node. */
	      f2->num_ac_args = num_ac_args(f1->t, SYMNUM(f1->t));
	      f->num_ac_nv_args = num_ac_nv_args(f1->t, SYMNUM(f1->t));

	      f = f1;
	    }

	    else {  /* non AC case */
	      int i;
	      f1 = get_flat();
	      f1->t = f->t;
	      f1->prev = f->prev;
	      f1->last = f;
	      f_save = f1;
	      if (f1->prev)
		f1->prev->next = f1;

	      t = f->t;
	      for (i = 0; i < ARITY(t); i++) {
		if (i < ARITY(t)-1)
		  f2 = get_flat();
		else
		  f2 = f;
		f2->place_holder = COMPLEX(ARG(t,i));
		f2->t = ARG(t,i);
		f2->last = f2;
		f2->prev = f1;
		f1->next = f2;
		f1 = f2;
	      }
	      f = f_save;
	      if (S.is_commutative(SYMNUM(f->t)) &&
		  !T.term_ident(ARG(f->t,0), ARG(f->t,1)))
		f->commutative = 1;
	    }  /* non-AC */
	  }  /* if f->place_holder */
	  if (f->commutative && f->flip == 0) {
	    f->alternatives = d;
	    f->flip = 1;
	  }
	}  /* rigid symbols match */
      }  /* non-variable */
      if (status == GO) {
	if (f->next) {
	  f = f->next;
	  d = d->u.kids;
	}
	else
	  status = SUCCESS;
      }  /* if go */
    }  /* if go */
  }  /* while go or backtrack */
  if (status == SUCCESS) {
    *ppos = f;
    return d->u.data;
  }
  else
    return NULL;
}

void * DiscrimWContainer::discrim_wild_retrieve_first(Term t, Discrim root,  Discrim_pos *ppos)
{
  Plist tp;
  Flat f;
  Discrim_pos gp;
  DiscrimContainer D;   
  tp = discrim_wild_retrieve_leaf(t, root, &f);
  if (tp == NULL)
    return NULL;
  else {
    gp = D.get_discrim_pos();
    gp->backtrack = f;
    gp->data = tp;
    *ppos = gp;
    return tp->v;
  }
} 


void * DiscrimWContainer::discrim_wild_retrieve_next(Discrim_pos pos) {
  Plist tp;
  DiscrimContainer D;  
  tp = pos->data->next;
  if (tp != NULL) {  /* if any more terms in current leaf */
    pos->data = tp;
    return tp->v;
  }
  else {  /* try for another leaf */
    tp = discrim_wild_retrieve_leaf(NULL, NULL, (Flat *) &(pos->backtrack));
					
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

void DiscrimWContainer::discrim_wild_cancel(Discrim_pos pos) {
  DiscrimContainer D;
  Flat f1 = (Flat) pos->backtrack;
  while (f1) {
    Flat f2 = f1;
    f1 = f1->prev;
    free_flat(f2);
  }
  D.free_discrim_pos(pos);
}
