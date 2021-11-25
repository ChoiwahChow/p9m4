#include "ladrvglobais.h"
#include "clist.h"
#include "memory.h"
#include "mystring.h"
#include "literals.h"
#include "order.h"

#include "fatal.h"
#include <iomanip>



GlobalClist::GlobalClist() {
    Clist_pos_gets=0;
    Clist_pos_frees=0;
    Clist_gets=0;
    Clist_frees=0;
   
}

GlobalClist::~GlobalClist() {
}



ClistContainer::ClistContainer() {head=NULL;}
ClistContainer::~ClistContainer() {head=NULL;}


Clist_pos ClistContainer::get_clist_pos(void) {
  Clist_pos p = (Clist_pos) Memory::memCNew(sizeof(struct clist_pos));
  LADR_GLOBAL_CLIST.Clist_pos_gets++;
  return(p);
} 

void ClistContainer::free_clist_pos(Clist_pos p) {
  Memory::memFree((void *) p,sizeof(struct clist_pos));
  LADR_GLOBAL_CLIST.Clist_pos_frees++;
}

Clist ClistContainer::get_clist(void) {
  Clist p = (Clist) Memory::memCNew(sizeof(struct clist));
  LADR_GLOBAL_CLIST.Clist_gets++;
  return(p);
} 

void ClistContainer::free_clist(Clist p) {
  Memory::memFree((void *)p, sizeof(struct clist));
  LADR_GLOBAL_CLIST.Clist_frees++;
}


void ClistContainer::fprint_clist_mem(ostream &o, bool heading)
{
  int n;
  if (heading)
	o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct clist_pos);
  o<<"clist_pos   ("<<setw(4)<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_CLIST.Clist_pos_gets;
  o<<setw(11)<<LADR_GLOBAL_CLIST.Clist_pos_frees;
  o<<setw(11)<<LADR_GLOBAL_CLIST.Clist_pos_gets-LADR_GLOBAL_CLIST.Clist_pos_frees;
  o<<setw(9)<<( (LADR_GLOBAL_CLIST.Clist_pos_gets-LADR_GLOBAL_CLIST.Clist_pos_frees)*n) /1024<<"K"<<endl;
  
  
  n = sizeof(struct clist);
  o<<"clist_pos   ("<<setw(4)<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_CLIST.Clist_gets;
  o<<setw(11)<<LADR_GLOBAL_CLIST.Clist_frees;
  o<<setw(11)<<LADR_GLOBAL_CLIST.Clist_gets-LADR_GLOBAL_CLIST.Clist_frees;
  o<<setw(9)<<( (LADR_GLOBAL_CLIST.Clist_gets-LADR_GLOBAL_CLIST.Clist_frees)*n) /1024<<"K"<<endl;

}

void ClistContainer::p_clist_mem() {
  fprint_clist_mem(cout, true);
}  /* p_clist_mem */


Clist ClistContainer::clist_init(string name) {
  Clist p = get_clist();
  if (name == myString::null_string()) p->name = NULL;
  else {
        p->name= new string(name);
  }
  head=p;
  return p;
}


void ClistContainer::name_clist(Clist p, string name) {
  if (p->name != NULL) delete (p->name);
  if (name == myString::null_string())  p->name = NULL;
  else
    p->name = new string(name);
} 


void ClistContainer::clist_free(Clist p) {
    if (p->first == NULL) {
    if (p->name != NULL) {
      delete(p->name);
      p->name = NULL;
    }
    free_clist(p);
  }
}



void ClistContainer::clist_append(Topform c, Clist l) {
  Clist_pos p;
  p = get_clist_pos();
  p->list = l;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->next = NULL;
  p->prev = l->last;
  l->last = p;
  if (p->prev) p->prev->next = p;
  else l->first = p;
  l->length++;
}


void ClistContainer::clist_prepend(Topform c, Clist l)
{
  Clist_pos p;
  p = get_clist_pos();
  p->list = l;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;
  p->prev = NULL;
  p->next = l->first;
  l->first = p;
  if (p->next) p->next->prev = p;
  else l->last = p;
  l->length++;
}


void ClistContainer::clist_insert_before(Topform c, Clist_pos pos) {
  Clist_pos p;
  p = get_clist_pos();
  p->list = pos->list;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->next = pos;
  p->prev = pos->prev;
  pos->prev = p;
  if (p->prev) p->prev->next = p;
  else  pos->list->first = p;
  pos->list->length++;
} 


void ClistContainer::clist_insert_after(Topform c, Clist_pos pos) {
  Clist_pos p;
  p = get_clist_pos();
  p->list = pos->list;
  p->c = c;
  p->nocc = c->containers;
  c->containers = p;

  p->prev = pos;
  p->next = pos->next;
  pos->next = p;
  if (p->next)  p->next->prev = p;
  else pos->list->last = p;
  pos->list->length++;
} 

void ClistContainer::clist_remove(Topform c, Clist l) {
  Clist_pos p, prev;

  /* Find position from containment list of clause. */
  for (p = c->containers, prev = NULL;
       p && p->list != l;
       prev = p, p = p->nocc);
    
  if (!p) 
    fatal::fatal_error("clist_remove: clause not in list");

  /* First update normal links. */
  if (p->prev)
    p->prev->next = p->next;
  else
    p->list->first = p->next;
  if (p->next)
    p->next->prev = p->prev;
  else
    p->list->last = p->prev;

    /* Now update containment links. */
  if (prev)
    prev->nocc = p->nocc;
  else
    c->containers = p->nocc;

  free_clist_pos(p);
  l->length--;
}



void ClistContainer::clist_remove_all_clauses(Clist l){
  while (l->first)
    clist_remove(l->first->c, l);
} 


int ClistContainer::clist_remove_all(Topform c) {
  int i = 0;
  while (c->containers) {
    clist_remove(c, c->containers->list);
    i++;
  }
  return i;
} 


int ClistContainer::clist_member(Topform c, Clist l){
  Clist_pos p;
  

  for (p = c->containers; p; p = p->nocc) {
    if (p->list == l)
      return true;
  }
  return false;
} 


void ClistContainer::fprint_clist(ostream &o, Clist l) {
  Clist_pos p;
  TopformContainer T;	
  if (l->name != NULL)
	o<<"list("<<l->name<<")."<<endl;  
  for(p = l->first; p; p = p->next)
    T.fprint_clause(o, p->c);
  o<<"end_of_list."<<endl;
}

void ClistContainer::p_clist(Clist l) {
  fprint_clist(cout, l);
} 

void ClistContainer::clist_zap(Clist l) {
  Clist_pos p;
  Topform c;
  TopformContainer T;
  p = l->first;
  while (p) {
    c = p->c;
    p = p->next;
    clist_remove(c, l);
    if (c->containers == NULL)
      T.zap_topform(c);
  }
  l->first=NULL;
  clist_free(l);
}

void ClistContainer::clist_check(Clist l) {
  Clist_pos p;
  int n = 0;

  for (p = l->first; p; p = p->next) {
    n++;
    if (p->list != l) cout<<"clist_check error0"<<endl;
      
    if (p->next) {
      if (p->next->prev != p) cout<<"clist_check error1"<<endl;
	
    }
    else if (p != l->last) cout<<"clist_check error2"<<endl;
      
    if (p->prev) {
      if (p->prev->next != p) printf("clist_check error3\n");
	
    }
    else if (p != l->first) printf("clist_check error4\n");
      
  }
  if (l->length != n) printf("clist_check error5\n");
    
}


void ClistContainer::clist_append_all(Clist l1, Clist l2) {
  Clist_pos p;
  for (p = l2->first; p != NULL; p = p->next)
    clist_append(p->c, l1);
  clist_zap(l2); /* This doesn't zap clauses, because they now occur in l1. */
} 


bool ClistContainer::clist_empty(Clist lst) {
  return lst->first == NULL;
}

int ClistContainer::clist_length(Clist l) {
  return l->length;
} 

int ClistContainer::max_wt_in_clist(Clist l) {
  int max = INT_MIN;
  Clist_pos p;
  for (p = l->first; p != NULL; p = p->next)
    if (p->c->weight > max)
      max = p->c->weight;
  return max;
}

bool ClistContainer::horn_clist(Clist l) {
  Clist_pos p;
 
  for (p = l->first; p != NULL; p = p->next)
    if (!LADRV_GLOBAIS_INST.Lit.horn_clause(p->c->literals))
      return false;
  return true;
}

bool ClistContainer::unit_clist(Clist l) {
  Clist_pos p;
  
  for (p = l->first; p != NULL; p = p->next)
    if (!LADRV_GLOBAIS_INST.Lit.unit_clause(p->c->literals))
      return false;
  return true;
} 

bool ClistContainer::equality_in_clist(Clist l) {
  Clist_pos p;

  for (p = l->first; p != NULL; p = p->next)
    if (LADRV_GLOBAIS_INST.Lit.contains_pos_eq(p->c->literals)) return true; 
  return false;
} 
bool ClistContainer::neg_nonunit_in_clist(Clist l) {
  Clist_pos p;
 
  for (p = l->first; p != NULL; p = p->next)
    if (LADRV_GLOBAIS_INST.Lit.negative_clause(p->c->literals) && LADRV_GLOBAIS_INST.Lit.number_of_literals(p->c->literals) > 1)
      return true;
  return false;
}

Plist ClistContainer::clauses_in_clist(Plist p, Clist l) {
  Plist q;
  Plist result = NULL;
  for (q = p; q; q = q->next) {
    Topform c = (Topform) q->v;
    if (clist_member(c, l)) {
		PlistContainer P;
		P.set_head(result);
		result = P.plist_append(c);
  }
 }
 return result;
}


void ClistContainer::clist_swap (Topform a, Topform b) {
  Clist_pos p;
  /* Replace refs to a with refs to b. */
  for (p = a->containers; p; p = p->nocc)
    p->c = b;
  /* Replace refs to b with refs to a. */
  for (p = b->containers; p; p = p->nocc)
    p->c = a;
  /* Swap the containment lists. */
  p = a->containers;
  a->containers = b->containers;
  b->containers = p;
}

void ClistContainer::clist_move_clauses(Clist a, Clist b) {
  while (a->first) {
    Topform c = a->first->c;
    clist_remove(c, a);
    clist_append(c, b);
  }
} 

Plist ClistContainer::move_clist_to_plist(Clist a) {
  Plist p = NULL;
  PlistContainer P;
  while (a->first) {
    Topform c = a->first->c;
    clist_remove(c, a);
	P.set_head(p);
	p = P.plist_prepend(c);
  }
  clist_free(a);
  p = P.reverse_plist();
  return p;
} 


Plist ClistContainer::copy_clist_to_plist_shallow(Clist a) {
  Clist_pos pos;
  Plist p = NULL;
  PlistContainer P;
  P.set_head(p);
  for (pos = a->first; pos; pos = pos->next) 
    P.plist_prepend((void *) pos->c);
  P.reverse_plist();
  return P.get_head();
} 

Clist ClistContainer::plist_to_clist(Plist p, string name) {
  Clist a = clist_init(name);
  Plist q;
  for (q = p; q; q = q->next) clist_append((Topform)q->v, a);
  PlistContainer P;
  P.set_head(p);
  P.zap_plist();
  return a;
}

void ClistContainer::clist_reverse(Clist l) {
  Clist_pos p = l->first;
  while (p) {
    Clist_pos next = p->next;
    p->next = p->prev;
    p->prev = next;
    p = next;
  }
  p = l->first;
  l->first = l->last;
  l->last = p;
}

Clist_pos ClistContainer::pos_in_clist(Clist lst, Topform c) {
  Clist_pos p = c->containers;
  while (p && p->list != lst)
    p = p->nocc;
  return p;
} 

void ClistContainer::clist_append_plist(Clist lst, Plist clauses) {
  Plist p;
  for (p = clauses; p; p = p->next) {
    clist_append(Topform(p->v), lst);
  }
} 

Plist ClistContainer::prepend_clist_to_plist(Plist p, Clist c) {
  if (c == NULL)
    return p;
  else {
    Clist_pos a;
	PlistContainer P;
	P.set_head(p);
    for (a = c->last; a; a = a->prev)
      P.plist_prepend(a->c);
    return P.get_head();
  }
} 


int ClistContainer::clist_number_of_weight(Clist lst, int weight) {
  if (lst == NULL)
    return 0;
  else {
    int n = 0;
    Clist_pos a;
    for (a = lst->first; a; a = a->next) {
      if (a->c->weight == weight)
	n++;
    }
    return n;
  }
} 


OrderType ClistContainer::compare_clause_ids(Topform c, Topform d) {
  if (c->id < d->id)
    return OrderType::LESS_THAN;
  else if (c->id > d->id)
    return OrderType::GREATER_THAN;
  else
    return OrderType::SAME_AS;
} 





void ClistContainer::sort_clist_by_id(Clist lst, OrderType (*comp_proc) (Topform, Topform) ) {
  int n = clist_length(lst);
  Topform *a = (Topform *) malloc(n * sizeof(Topform *));
  int i;
  Clist_pos p;

  for (p = lst->first, i = 0; p; p = p->next, i++)
    a[i] = p->c;

  clist_remove_all_clauses(lst);

  myOrder::merge_sort( (void **) a, n, (OrderType (*)(void*,void*)) comp_proc);

  for (i = 0; i < n; i++)
    clist_append(a[i], lst);
  
  free(a);
}


void ClistContainer::sort_clist_by_id(Clist lst) {
    sort_clist_by_id(lst, compare_clause_ids);
}


Plist ClistContainer::neg_clauses_in_clist(Clist a) {
  Clist_pos p;
  PlistContainer N;

  
  
  for (p = a->first; p; p = p->next) {
    if (LADRV_GLOBAIS_INST.Lit.negative_clause(p->c->literals))
      N.plist_prepend(p->c);
  }
  N.reverse_plist();
  return N.get_head();
}  /* neg_clauses_in_clist */


void ClistContainer::fprint_clause_clist(ostream &o, Clist lst) {
  Clist_pos p;
  TopformContainer T;
  if (lst->name == NULL || myString::str_ident(*lst->name, ""))
	o<<endl<<"clauses(anonymous)."<<endl;
  else
	 o<<endl<<"clauses("<<lst->name<<")."<<endl;
	  
  for (p = lst->first; p != NULL; p = p->next)
    T.fprint_clause(o, p->c);
  o<<"end_of_list."<<endl;
} 
