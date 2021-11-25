#include "discrimb.h"
#include "memory.h"
#include "symbols.h"
#include "fatal.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>




GlobalDiscrimB::GlobalDiscrimB() {
    Flat2_gets=0;
    Flat2_frees=0;
}

GlobalDiscrimB::~GlobalDiscrimB() {
}


DiscrimBContainer::DiscrimBContainer() { root=NULL; }
DiscrimBContainer::~DiscrimBContainer(){ root=NULL; }


Flat2 DiscrimBContainer::get_flat2(void) {
	Flat2 p= (Flat2) Memory::memCNew(sizeof(struct flat2));
	LADR_GLOBAL_DISCRIM_B.Flat2_gets++;
	return p;
}

void DiscrimBContainer::free_flat2(Flat2 p) {
	Memory::memFree((void *)p, sizeof(struct flat2));
	LADR_GLOBAL_DISCRIM_B.Flat2_frees++;
}

void DiscrimBContainer::fprint_discrimb_mem(ostream &o, bool heading) {
  int n;
  if (heading)
	o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct flat2);
  o<<"flat2         ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_DISCRIM_B.Flat2_gets;
  o<<setw(11)<<LADR_GLOBAL_DISCRIM_B.Flat2_frees;
  o<<setw(11)<<LADR_GLOBAL_DISCRIM_B.Flat2_gets - LADR_GLOBAL_DISCRIM_B.Flat2_frees;
  o<<setw(9)<<( (LADR_GLOBAL_DISCRIM_B.Flat2_gets - LADR_GLOBAL_DISCRIM_B.Flat2_frees)*n)/1024<<"K"<<endl;
} 


void DiscrimBContainer::p_discrimb_mem(void){
  fprint_discrimb_mem(cout, true);
} 

void DiscrimBContainer::check_discrim_bind_tree(Discrim d, int n) {
 
  if (n > 0) {
    SymbolContainer S;
	int arity;
    Discrim d1;
    for (d1 = d->u.kids; d1; d1 = d1->next) {
      if (DVAR(d1)) arity = 0;
      else	arity = S.sn_to_arity(d1->symbol);
      check_discrim_bind_tree(d1, n+arity-1);
    }
  }
} 


void DiscrimBContainer::print_discrim_bind_tree(ostream &o, Discrim d, int n, int depth) {
  int arity, i;
  SymbolContainer S;
  for (i = 0; i < depth; i++) o<<" -";
  if (depth == 0) o<<endl<<"root";
  else if (DVAR(d)) o<<d->symbol;
  else o<<S.sn_to_str(d->symbol);
  o<<"("<<d<<")";  
  if (n == 0) {
    Plist p;
    for (i = 0, p = d->u.data; p; i++, p = p->next);
	o<<": leaf has "<<i<<" objects"<<endl;
  }
  else {
    Discrim d1;
	o<<endl;
    for (d1 = d->u.kids; d1 != NULL; d1 = d1->next) {
      if (DVAR(d1))	arity = 0;
      else	arity = S.sn_to_arity(d1->symbol);
      print_discrim_bind_tree(o, d1, n+arity-1, depth+1);
    }
  }
}  

void DiscrimBContainer::fprint_discrim_bind_index(ostream &o, Discrim d) {
  print_discrim_bind_tree(o, d, 1, 0);
}

void DiscrimBContainer::p_discrim_bind_index(Discrim d) {
    fprint_discrim_bind_index(cout, d);
} 


Discrim DiscrimBContainer::discrim_bind_insert_rec(Term t, Discrim d) {
  Discrim d1, d2, prev;
  int symbol, i;
  DiscrimContainer D;

  if (VARIABLE(t)) {
    d1 = d->u.kids;
    prev = NULL;
    symbol = VARNUM(t);
    while (d1 && DVAR(d1) && d1->symbol < symbol) {
      prev = d1;
      d1 = d1->next;
    }
    if (d1 == NULL || !DVAR(d1) || d1->symbol != symbol) {
      d2 = D.get_discrim();
      d2->type = DiscriminationTreeNode::DVARIABLE;
      d2->symbol = VARNUM(t);
      d2->next = d1;
      if (prev == NULL)
	d->u.kids = d2;
      else
	prev->next = d2;
      return d2;
    }
    else  /* found node */
      return d1;
  }

  else {  /* constant || complex */
    d1 = d->u.kids;
    prev = NULL;
    /* arities fixed: handle both NAME and COMPLEX */
    symbol = SYMNUM(t);
    while (d1 && DVAR(d1)) {  /* skip variables */
      prev = d1;
      d1 = d1->next;
    }
    while (d1 && d1->symbol < symbol) {
      prev = d1;
      d1 = d1->next;
    }
    if (d1 == NULL || d1->symbol != symbol) {
      d2 = D.get_discrim();
      d2->type = DiscriminationTreeNode::DRIGID;
      d2->symbol = symbol;
      d2->next = d1;
      d1 = d2;
    }
    else
      d2 = NULL;  /* new node not required at this level */

    for (i = 0; i < ARITY(t); i++)
      d1 = discrim_bind_insert_rec(ARG(t,i), d1);

    if (d2 != NULL) {  /* link in new subtree (possibly a leaf) */
      if (prev == NULL)
	d->u.kids = d2;
      else
	prev->next = d2;
    }
	    
    return d1;  /* d1 is leaf corresp. to end of input term */
  }
} 

void DiscrimBContainer::discrim_bind_insert(Term t, Discrim root, void *object) {
  Discrim d;
  Plist gp1, gp2;
  PlistContainer P;

  d = discrim_bind_insert_rec(t, root);
  gp1 = P.get_plist();
  gp1->v = object;
  gp1->next = NULL;

  /* Install at end of list. */
  if (d->u.data == NULL)
    d->u.data = gp1;
  else {
    for (gp2 = d->u.data; gp2->next != NULL; gp2 = gp2->next);
    gp2->next = gp1;
  }
}


Discrim DiscrimBContainer::discrim_bind_end(Term t, Discrim d, Plist *path_p) {
  Discrim d1;
  Plist dp;
  PlistContainer P;
  int symbol, sym;

  /* add current node to the front of the path list. */

  dp = P.get_plist();
  dp->v = d;
  dp->next = *path_p;
  *path_p = dp;

  if (VARIABLE(t)) {
    d1 = d->u.kids;
    symbol = VARNUM(t);
    while (d1 && DVAR(d1) && d1->symbol < symbol) 
      d1 = d1->next;

    if (d1 == NULL || !DVAR(d1) || d1->symbol != symbol)
      return NULL;
    else   /* found node */
      return d1;
  }

  else {  /* constant || complex */
    d1 = d->u.kids;
    sym = SYMNUM(t);  /* arities fixed: handle both NAME and COMPLEX */
    while (d1 && DVAR(d1))  /* skip variables */
      d1 = d1->next;
    while (d1 && d1->symbol < sym)
      d1 = d1->next;

    if (d1 == NULL || d1->symbol != sym)
      return NULL;
    else {
      int i;
      for (i = 0; d1 && i < ARITY(t); i++)
	d1 = discrim_bind_end(ARG(t,i), d1, path_p);
      return d1;
    }
  }
}

void DiscrimBContainer::discrim_bind_delete(Term t, Discrim root, void *object) {
  Discrim end, d2, d3, parent;
  Plist tp1, tp2;
  Plist dp1, path;
  PlistContainer P;
  DiscrimContainer D;

    /* First find the correct leaf.  path is used to help with  */
    /* freeing nodes, because nodes don't have parent pointers. */

  path = NULL;
  end = discrim_bind_end(t, root, &path);
  if (end == NULL) {
    fatal::fatal_error("discrim_bind_delete, cannot find end.");
  }

    /* Free the pointer in the leaf-list */

  tp1 = end->u.data;
  tp2 = NULL;
  while(tp1 && tp1->v != object) {
    tp2 = tp1;
    tp1 = tp1->next;
  }
  if (tp1 == NULL) {
    fatal::fatal_error("discrim_bind_delete, cannot find term.");
  }

  if (tp2 == NULL)
    end->u.data = tp1->next;
  else
    tp2->next = tp1->next;
  P.free_plist(tp1);

  if (end->u.data == NULL) {
    /* free tree nodes from bottom up, using path to get parents */
    end->u.kids = NULL;  /* probably not necessary */
    dp1 = path;
    while (end->u.kids == NULL && end != root) {
      parent = (Discrim) dp1->v;
      dp1 = dp1->next;
      d2 = parent->u.kids;
      d3 = NULL;
      while (d2 != end) {
	d3 = d2;
	d2 = d2->next;
      }
      if (d3 == NULL)
	parent->u.kids = d2->next;
      else
	d3->next = d2->next;
      D.free_discrim(d2);
      end = parent;
    }
  }
  /* free path list */
  
  while (path) {
    dp1 = path;
    path = path->next;
    P.free_plist(dp1);
  }
} 


void DiscrimBContainer::discrim_bind_update(Term t, Discrim root, void *object, Indexop op) {
  if (op == Indexop::INSERT)
    discrim_bind_insert(t, root, object);
  else
    discrim_bind_delete(t, root, object);
}


Flat2 DiscrimBContainer::check_flat2(Flat2 f) {
  Flat2 last;
  int i, arity;

  if (f->next != NULL && f->next->prev != f)
		cerr<<"check_flat2: next-prev error"<<endl;

  if (f->place_holder)
    arity = 0;
  else
    arity = ARITY(f->t);
  
  last = f;
  for (i = 0; i < arity; i++) 
    last = check_flat2(last->next);
  if (f->last != last)
    cerr<<"check_flat2: last is wrong"<<endl;
	
  return last;
}  

void DiscrimBContainer::p_flat2(Flat2 f) {
  SymbolContainer S;
  while (f != NULL) {
    if (VARIABLE(f->t)) cout<<"v" + VARNUM(f->t);
    else cout <<S.sn_to_str(SYMNUM(f->t));
    if (f->place_holder) cout<<"[]";
    f = f->next;
    if (f != NULL) cout<<"-";
   
  }
  cout<<endl;	
}

Plist DiscrimBContainer::discrim_bind_retrieve_leaf(Term t_in, Discrim root,
			    Context subst, Flat2 *ppos)
{
  Flat2 f, f1, f2, f_save;
  Term t = NULL;
  Discrim d = NULL;
  int symbol = 0;
  int match = 0;
  int bound = 0;
  int status = 0;
  TermContainer T;

  f = *ppos;  /* Don't forget to reset before return. */
  t = t_in;
  f_save = NULL;

  if (t != NULL) {  /* if first call */
    d = root->u.kids;
    if (d != NULL) {
      f = get_flat2();
      f->t = t;
      f->last = f;
      f->prev = NULL;
      f->place_holder = (COMPLEX(t));
      status = GO;
    }
    else
      status = FAILURE;
  }
  else
    status = BACKTRACK;

  while (status == GO || status == BACKTRACK) {
    if (status == BACKTRACK) {
      while (f && !f->alternatives) {  /* clean up HERE??? */
		if (f->bound) {
			subst->terms[f->varnum] = NULL;
			f->bound = 0;
	    }
	 f_save = f;
	 f = f->prev;
    }
    if (f != NULL) {
	 if (f->bound) {
	  subst->terms[f->varnum] = NULL;
	  f->bound = 0;
	 }
 	 d = f->alternatives;
	 f->alternatives = NULL;
	 status = GO;
    }
    else status = FAILURE;
    }

    if (status == GO) {
      match = 0;
      while (!match && d && DVAR(d)) {
		symbol = d->symbol;
		if (subst->terms[symbol]) { /* if already bound */
		match = T.term_ident(subst->terms[symbol], f->t);
		bound = 0;
		}
		else { /* bind variable in discrimb tree */
		 match = 1;
		 subst->terms[symbol] = f->t;
		 bound = 1;
		}
	if (!match)	  d = d->next;
    }
    if (match) {
	/* push alternatives */
	 f->alternatives = d->next;
	 f->bound = bound;
	 f->varnum = symbol;
	 f = f->last;
    }
    else if (VARIABLE(f->t)) status = BACKTRACK;
    else {
		symbol = SYMNUM(f->t);
		while (d && d->symbol < symbol) d = d->next;
		if (!d || d->symbol != symbol) status = BACKTRACK;
		else if (f->place_holder) {
			int i;
			/* insert skeleton in place_holder */
			f1 = get_flat2();
			f1->t = f->t;
			f1->prev = f->prev;
			f1->last = f;
			f_save = f1;
			if (f1->prev)
			f1->prev->next = f1;
			t = f->t;
			for (i = 0; i < ARITY(t); i++) {
				if (i < ARITY(t)-1) f2 = get_flat2();
				else  f2 = f;
				f2->place_holder = COMPLEX(ARG(t,i));
				f2->t = ARG(t,i);
				f2->last = f2;
				f2->prev = f1;
				f1->next = f2;
				f1 = f2;
			}
			f = f_save;
	 }  /* if f->place_holder */
    }
     if (status == GO) {
		if (f->next) {
		 f = f->next;
		 d = d->u.kids;
		}
		else status = SUCCESS;
      }
    }
  }  /* while */
  if (status == SUCCESS) {
    *ppos = f;
    return d->u.data;
  }
  else {
    /* Free flat2s. */
#ifdef SPEED
    Flat2count = 0;
#else
    while (f_save) {
      f1 = f_save;
      f_save = f_save->next;
      free_flat2(f1);
    }
#endif
    return NULL;
  }
	   
}


void * DiscrimBContainer::discrim_bind_retrieve_first(Term t, Discrim root, Context subst, Discrim_pos *ppos){
  Plist tp;
  Flat2 f;
  Discrim_pos gp;
  DiscrimContainer D;

  tp = discrim_bind_retrieve_leaf(t, root, subst, &f);
  if (tp == NULL)   return NULL;
  else {
    gp = D.get_discrim_pos();
    gp->subst = subst;
    gp->backtrack = f;
    gp->data = tp;
    *ppos = gp;
    return tp->v;
  }
} 

void * DiscrimBContainer::discrim_bind_retrieve_next(Discrim_pos pos){
  Plist tp;
  DiscrimContainer D;  
  tp = pos->data->next;
  if (tp != NULL) {  /* if any more terms in current leaf */
    pos->data = tp;
    return tp->v;
  }
  else {  /* try for another leaf */
    tp = discrim_bind_retrieve_leaf(NULL, NULL,
				    pos->subst, (Flat2 *) &(pos->backtrack));
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

void DiscrimBContainer::discrim_bind_cancel(Discrim_pos pos) {
  Flat2 f1, f2;
  DiscrimContainer D;
  f1 =(Flat2) pos->backtrack;
  while (f1) {
    if (f1->bound)
      pos->subst->terms[f1->varnum] = NULL;
    f2 = f1;
    f1 = f1->prev;
#ifndef SPEED
    free_flat2(f2);
#endif
  }
#ifdef SPEED
  Flat2count = 0;
#endif
  D.free_discrim_pos(pos);
}
