#include "ladrvglobais.h"
#include "pindex.h"
#include "memory.h"
#include "fatal.h"
#include <iostream>
#include <iomanip>




GlobalPindex::GlobalPindex() {
    Pair_index_gets=0;
    Pair_index_frees=0;	
}


GlobalPindex::~GlobalPindex() {
}



PindexContainer::PindexContainer() {
	pindexHead=NULL;
}
PindexContainer::~PindexContainer() {
	pindexHead=NULL;
}

Pair_index PindexContainer::get_pair_index(void) {
  
  Pair_index p = (Pair_index) Memory::memCNew(sizeof(pair_index));
  p->min = INT_LARGE;
  p->new_min = INT_LARGE;
  LADR_GLOBAL_PINDEX.Pair_index_gets++;
  return(p);
}

void PindexContainer::free_pair_index(Pair_index p) {
  Memory::memFree((void *)p, sizeof(pair_index));
  LADR_GLOBAL_PINDEX.Pair_index_frees++;
}

void PindexContainer::fprint_pindex_mem(ostream &o, bool heading) {
  int n;
  if (heading)
	o<<"  type(bytes each)          gets      frees     in use      bytes"<<endl;
  n = sizeof(struct pair_index);
  o<<"pair_index ("<<setw(4)<<n<<")   ";
  o<<setw(11)<<LADR_GLOBAL_PINDEX.Pair_index_gets;
  o<<setw(11)<<LADR_GLOBAL_PINDEX.Pair_index_frees;
  o<<setw(11)<<LADR_GLOBAL_PINDEX.Pair_index_gets-LADR_GLOBAL_PINDEX.Pair_index_frees;
  o<<setw(9)<<( (LADR_GLOBAL_PINDEX.Pair_index_gets-LADR_GLOBAL_PINDEX.Pair_index_frees)*n)/1024<<endl;
} 

void PindexContainer::p_pindex_mem() {
  fprint_pindex_mem(cout, true);
}

Pair_index PindexContainer::init_pair_index(int n) {
  Pair_index p;
  int i, j;

  p = get_pair_index();

  p->finished = 1;
  p->n = n;
  p->i = 0;
  p->j = 0;
  p->min = INT_LARGE;
  p->new_min = INT_LARGE;

  p->lists = (Clist *) malloc(n * sizeof(Clist));
  p->top   = (Clist_pos *)malloc(n * n * sizeof(Clist_pos));
  p->curr  = (Clist_pos *)malloc(n * n * sizeof(Clist_pos));

  /* top and curr will be indexed as top[i*n+j]. */
  ClistContainer C;
  for (i = 0; i < n; i++)
    p->lists[i] = C.clist_init(string(""));
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      p->top[i*p->n+j] = NULL;
      p->curr[i*p->n+j] = NULL;
    }
  pindexHead=p;
  return p;
} 

void PindexContainer::zap_pair_index(Pair_index p) {
  int i;
  ClistContainer C;
  for (i = 0; i < p->n; i++) {
    C.clist_zap(p->lists[i]);
  }
  free(p->lists);
  free(p->top);
  free(p->curr);
  free_pair_index(p);
}

int PindexContainer::pairs_exhausted(Pair_index p) {
  return p->finished;
} 



void PindexContainer::init_pair(int i, int j, Pair_index p) {
  int n = p->n;
  Clist_pos lp_i = p->lists[i]->first;
  Clist_pos lp_j = p->lists[j]->first;

  if (lp_i && lp_j) {
    if (i == j) {
      p->top[i*n+i] = lp_i;
      p->curr[i*n+i] = NULL;
    }
    else {
      p->top[i*n+j] = lp_i;
      p->top[j*n+i] = lp_j;
      /* It doesn't matter which curr gets set to NULL. */
      p->curr[i*n+j] = lp_i;
      p->curr[j*n+i] = NULL;
    }
  }
  else {
    p->top[i*n+j] = NULL;
    p->top[j*n+i] = NULL;
    p->curr[i*n+j] = NULL;
    p->curr[j*n+i] = NULL;
  }
} 

void PindexContainer::insert_pair_index(Topform c, int wt, Pair_index p)
{
  /* If the new clause will be the only one in its list, then
   * for each nonempty list, set the top and curr.
     */
  int i, j, n;

  n = p->n;
  j = IN_RANGE(wt, 0, n-1);
  ClistContainer C;
  if (p->lists[j]->first == NULL) {
    C.clist_append(c, p->lists[j]);
    for (i = 0; i < p->n; i++)
      init_pair(i, j, p);
  }
  else
    C.clist_append(c, p->lists[j]);

  p->finished = 0;
  if (wt < p->new_min)
    p->new_min = wt;
  if (wt < p->min)
    p->min = wt;
} 

void PindexContainer::delete_pair_index(Topform c, int wt, Pair_index p) {
  int i, j;
  int n = p->n;
  Clist_pos lp;

  j = IN_RANGE(wt, 0, n-1);

  for (lp = p->lists[j]->first; lp && lp->c != c; lp = lp->next);
  if (!lp) {
    fatal::fatal_error("delete_pair_index, clause not found.");
  }

  /* We are deleting a clause from list j.  For each list i, consider the
   * pair [i,j].  Top[i,j] and curr[i,j] (say t1 and c1) point into list i,
   * and top[j,i] and curr[j,i] (say t2 anc c2) point into list j.
   */

  for (i = 0; i < n; i++) {
    Clist_pos t1 = p->top[i*n+j];
    Clist_pos c1 = p->curr[i*n+j];
    Clist_pos t2 = p->top[j*n+i];
    Clist_pos c2 = p->curr[j*n+i];

    if (i == j) {
      if (t2 == lp) {
	/* printf("HERE: i == j\n"); */
	/* This handles t2=c2, c2==NULL, c2 != NULL, singleton list. */
	if (t2->next) {
	  p->top[i*n+i] = t2->next;
	  p->curr[i*n+i] = NULL;
	}
	else {
	  p->top[i*n+i] = t2->prev;
	  p->curr[i*n+i] = t2->prev;
	}
      }
      else if (c2 == lp) {
	p->curr[i*n+i] = c2->prev;
      }
    }
    else {  /* i != j */

      if (lp == t2) {
	/* printf("HERE: i != j (B)\n"); */
	if (t2 == c2) {
	  if (t2->next) {
	    t2 = t2->next;
	    c2 = c2->next;
	    c1 = NULL;
	  }
	  else if (t2->prev) {
	    t2 = t2->prev;
	    c2 = c2->prev;
	    c1 = t1;
	  }
	  else
	    t1 = c1 = t2 = c2 = NULL;
	}
	else if (t2->prev)
	  t2 = t2->prev;
	else if (t2->next) {
	  t2 = t2->next;
	  c2 = NULL;
	  t1 = c1 = p->lists[i]->first;
	}
	else
	  t1 = c1 = t2 = c2 = NULL;
      }
      else if (lp == c2) {
	/* printf("HERE: i != j (D)\n"); */
	c2 = c2->prev;
      }

      p->top[i*n+j] = t1;
      p->curr[i*n+j] = c1;
      p->top[j*n+i] = t2;
      p->curr[j*n+i] = c2;
    }
  }
  ClistContainer C;
  C.clist_remove(c, p->lists[j]);
}


void PindexContainer::retrieve_pair(Pair_index p, Topform *cp1, Topform *cp2) {
  int i, j, k, max_k, found, n;

  /* First, find i and j, the smallest pair of weights that
   * have clauses available.  p->i and p->j are from the
   * previous retrieval, and if no clauses have been inserted
   * since then, start with them.  Otherwise, use new_min
   * (the smallest weight inserted since the previous retrieval)
   * and min (the smallest weight in the index) to decide
   * where to start looking.
   */ 
     
  if (p->min + p->new_min < p->i + p->j) {
    i = p->min;
    j = p->new_min;
  }
  else {
    i = p->i;
    j = p->j;
  }

  n = p->n;
  k = i+j;
  max_k = (n + n) - 2;
  found = 0;

  while (k <= max_k && !found) {
    i = k / 2; j = k - i;
    while (i >= 0 && j < n && !found) {
      /* This test works if (i==j). */
      found = (p->top[i*n+j] != p->curr[i*n+j] ||
	       p->top[j*n+i] != p->curr[j*n+i] ||
	       (p->top[i*n+j] && p->top[i*n+j]->next) ||
	       (p->top[j*n+i] && p->top[j*n+i]->next));
		     		     
      if (!found) {
	i--; j++;
      }
    }
    if (!found)
      k++;
  }

  if (!found) {
    *cp1 = NULL; *cp2 = NULL;
    p->finished = 1;
  }
  else {
    /* OK, there should be a pair in (i,j). */

    /* Recall that if top[i,j]=curr[i,j] and top[j,i]=top[j,i],
     * then all pairs up to those positions have been returned.
     */

    Clist_pos t1 = p->top[i*n+j];
    Clist_pos c1 = p->curr[i*n+j];
    Clist_pos t2 = p->top[j*n+i];
    Clist_pos c2 = p->curr[j*n+i];

    if (i == j) {
      if (t1 == c1) {
		p->top[i*n+i]  = t1 =  t1->next;
		p->curr[i*n+i] = c1 = NULL;
      }
      *cp1 = t1->c;
      p->curr[i*n+i] = c1 = (c1 ? c1->next : p->lists[i]->first);
      *cp2 = c1->c;
    }
    else {  /* i != j */
      if (t1 == c1 && t2 == c2) {
	/* Both tops equal their currs, so pick a top to increment. */
	if (t1->next && (t1->c->id < t2->c->id || !t2->next)) {
	  p->top[i*n+j]  = t1 = t1->next;
	  p->curr[i*n+j] = c1 = c1->next;
	  p->curr[j*n+i] = c2 = NULL;
	}
	else {
	  p->top[j*n+i]  = t2 = t2->next;
	  p->curr[j*n+i] = c2 = c2->next;
	  p->curr[i*n+j] = c1 = NULL;
	}
      }
      if (t1 == c1) {
		*cp1 = t1->c;
		p->curr[j*n+i] = c2 = (c2 ? c2->next : p->lists[j]->first);
		*cp2 = c2->c;
      }
      else if (t2 == c2) {
		*cp1 = t2->c;
		p->curr[i*n+j] = c1 = (c1 ? c1->next : p->lists[i]->first);
		*cp2 = c1->c;
      }
      else {
				fatal::fatal_error("retrieve_pair, bad state.");
      }
    }
    /* Save the "working pair" for next time. */
    p->i = i;
    p->j = j;
    p->new_min = INT_LARGE;
  }
} 

void PindexContainer::p_pair_index(Pair_index p) {
  int i, j, n;
  Clist_pos x;

  n = p->n;

  for (i = 0; i < n; i++) {
    cout<<"Clist "<<i;
	for (x = p->lists[i]->first; x; x = x->next)
		cout<<" "<<setw(3)<<x->c->id;
    cout<<"."<<endl;  
    
  }
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      cout<<" [";
	  if (p->top[i*n+j])
		cout<<setw(2)<<p->top[i*n+j]->c->id;
      else cout<<"--";
	cout<<",";
	
      if (p->curr[i*n+j])
		cout<<setw(2)<<p->curr[i*n+j]->c->id;
      else cout<<"--";
	  cout<<"] ";
    }
    cout<<endl;
  }
} 

int PindexContainer::pair_already_used(Topform c1, int weight1,Topform c2, int weight2, Pair_index p) {
  int i, j, id1, id2;
  int rc = 0;
  int n = p->n;
  Clist_pos top1, curr1, top2, curr2;

  id1 = c1->id;
  id2 = c2->id;

  i = IN_RANGE(weight1, 0, n-1);
  j = IN_RANGE(weight2, 0, n-1);

  top1 = p->top[i*n+j];
  curr1 = p->curr[i*n+j];

  top2 = p->top[j*n+i];
  curr2 = p->curr[j*n+i];

  if (!top1 || !top2) {
    /* One of the lists is empty.  If this happens, something is
       probably wrong: why would we be trying to use c1 and c2? 
       */
    fatal::fatal_error("pair_already_used, bad state (1).");
  }
  else if (i == j) {
    /* Let id2 be the greater one. */
    if (id1 > id2) {
      int tmp = id1; id1 = id2; id2 = tmp;
    }
    rc = ((id2 < top1->c->id) ||
	  (id2 == top1->c->id && curr1 && id1 <= curr1->c->id));
  }
  else {  /* i != j */

    if (top1 == curr1) {
      rc = ((id1 <  top1->c->id && id2 <= top2->c->id) ||
	    (id1 == top1->c->id && curr2 && id2 <= curr2->c->id));
    }
    else if (top2 == curr2) {
      rc = ((id2 <  top2->c->id && id1 <= top1->c->id) ||
	    (id2 == top2->c->id && curr1 && id1 <= curr1->c->id));
    }
    else {
      fatal::fatal_error("pair_already_used, bad state (2).");
    }
  }	
  return rc;
}



