#include "glist.h"
#include "memory.h"
#include "fatal.h"
#include "mystring.h"
#include "ladrvglobais.h"
#include <string.h>
#include <climits>
#include <iostream>
#include <iomanip>

using namespace std;

//Iniciar as variáveis da glasse globalGlist------------------------




GlobalGlist::GlobalGlist() {
    Ilist_gets=0;
    Ilist_frees=0;
    Plist_gets=0;
    Plist_frees=0;
    I2list_gets=0; 
    I2list_frees=0;
    I3list_gets=0; 
    I3list_frees=0;
}

GlobalGlist::~GlobalGlist() {

    
}

void GlobalGlist::fprint_glist_mem(ostream &o , bool heading) {
  int n;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;

  n = sizeof(struct ilist);
 
  o<<    "ilist       ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_G_LIST.Ilist_gets<<setw(11)<<LADR_GLOBAL_G_LIST.Ilist_frees<<setw(11)<<LADR_GLOBAL_G_LIST.Ilist_gets-LADR_GLOBAL_G_LIST.Ilist_frees;
  o<<setw(9)<<( (LADR_GLOBAL_G_LIST.Ilist_gets - LADR_GLOBAL_G_LIST.Ilist_frees) * n) / 1024 <<"K"<<endl;

  n = sizeof(struct plist);
 
  o<<    "plist       ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_G_LIST.Plist_gets<<setw(11)<<LADR_GLOBAL_G_LIST.Plist_frees<<setw(11)<<LADR_GLOBAL_G_LIST.Plist_gets-LADR_GLOBAL_G_LIST.Plist_frees;
  o<<setw(9)<<( (LADR_GLOBAL_G_LIST.Plist_gets - LADR_GLOBAL_G_LIST.Plist_frees) * n) / 1024 <<"K"<<endl;
  
  
  n = sizeof(struct i2list);
 
  o<<    "i2list      ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_G_LIST.I2list_gets<<setw(11)<<LADR_GLOBAL_G_LIST.I2list_frees<<setw(11)<<LADR_GLOBAL_G_LIST.I2list_gets-LADR_GLOBAL_G_LIST.I2list_frees;
  o<<setw(9)<<( (LADR_GLOBAL_G_LIST.I2list_gets - LADR_GLOBAL_G_LIST.I2list_frees) * n) / 1024 << "K"<<endl;
  
}	


void GlobalGlist::p_glist_mem(void){
	fprint_glist_mem(cout, true);
}


//Classe IlistContainer

IlistContainer::IlistContainer() {
	head=NULL;
}

IlistContainer::IlistContainer(Ilist i) {
    head=i;
}    

IlistContainer::~IlistContainer() {
	head=NULL;
}

Ilist IlistContainer::get_ilist(void) {
  Ilist p = (Ilist) Memory::memNew(sizeof(struct ilist));
  p->next = NULL;
  LADR_GLOBAL_G_LIST.Ilist_gets++;
  return(p);
}

void IlistContainer::free_ilist(Ilist p) {
  LADR_GLOBAL_G_LIST.Ilist_frees++;
  Memory::memFree(p, sizeof( struct ilist));
} 

Ilist IlistContainer::get_head() {
	return head;
}

void IlistContainer::set_head(Ilist p) {
    head=p;
}


Ilist IlistContainer::ilist_append(int i) {
  Ilist aux;
  Ilist g=get_ilist();
  g->i=i;
  g->next=NULL;
  if (head == NULL) head=g;
  else {
			aux=head;
			while (aux->next!=NULL) aux=aux->next;
			aux->next=g;
  }
  return head;
}



void IlistContainer::zap_ilist(Ilist p) {
    free_ilist(p);
    p=NULL;
}

void IlistContainer::zap_ilist(void) {
  Ilist curr, prev;
  curr = head;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    zap_ilist(prev);
    prev=NULL;
  }
  head=NULL;
}




Ilist IlistContainer::ilist_pop(void) {
  Ilist aux =head;
  head = head->next;
  free_ilist(aux);
  return head;
}

int IlistContainer::ilist_count(void) {
  Ilist aux=head;
  int n;
  for (n = 0; aux; aux = aux->next, n++);
  return(n);
}



Ilist IlistContainer::ilist_cat(Ilist i1, Ilist i2) {

    if(i1==NULL) return i2;
    else if (i2==NULL) return i1;
    else {
            Ilist aux=i1;
            while(aux->next!=NULL) aux=aux->next;
            aux->next=i2;
            return i1;
    }
}


Ilist IlistContainer::ilist_cat(IlistContainer &p)
{
  if (head == NULL) {
      head=p.get_head();  
      return p.get_head();
  }    
  else 
      if (p.get_head() == NULL) return head;
  else {
    Ilist aux = head;
    while (aux->next != NULL) aux = aux->next;
    aux->next = p.get_head();
    return head;
  }
 
}




Ilist IlistContainer::ilist_cat2(IlistContainer &p) {
  if (head == NULL) {
                        Ilist aux=p.copy_ilist();
                        head=aux;
                        return aux;
  }
  else {
           Ilist aux=head;
           while(aux->next) aux=aux->next;
           Ilist aux1=p.copy_ilist();
           aux->next=aux1;
           return head;
    }
}



Ilist IlistContainer::copy_ilist(void) 
{
  Ilist start, p1, p2,aux;
  start = NULL;
  p2 = NULL;
  aux=head;
  for ( ; aux; aux = aux->next) {
    p1 = get_ilist();
    p1->i = aux->i;
    p1->next = NULL;
    if (p2) p2->next = p1;
    else start = p1;
    p2 = p1;
  }
  return(start);
} 


Ilist IlistContainer::ilist_prepend(Ilist lst, int i){
  Ilist g = get_ilist();
  g->i = i;
  g->next = lst;
  return g;
}


Ilist IlistContainer::ilist_prepend(int i)
{
  Ilist g = get_ilist();
  g->i = i;
  g->next = head;
  head=g;
  return head;
}  
                          
                          
Ilist IlistContainer::ilist_last(void){
  if (head == NULL) return NULL;
  else if (head->next == NULL) return head;
  else {
            Ilist aux=head;
            while(aux->next) aux=aux->next;
            return aux;
    }
}

bool IlistContainer::ilist_member(Ilist p, int i) {
    Ilist aux=p;
    bool achou=false;
    while(aux && !achou)
        if (aux->i==i) achou=true;
        else aux=aux->next;
    return achou;    
}


bool IlistContainer::ilist_member(int i) {
    return ilist_member(head,i);
}



Ilist IlistContainer::ilist_subtract(Ilist p1, Ilist p2) { //retorna uma lista com os elementos de p1 que não estão em p2
   
    if(p1==NULL) return NULL;
    else {
            Ilist r=ilist_subtract(p1->next, p2);
            if (ilist_member(p2, p1->i)) return r;
            else return ilist_prepend(r, p1->i);
    }
}

Ilist IlistContainer::ilist_subtract(IlistContainer &p2) {
    return ilist_subtract(head, p2.get_head()); //retorna uma lista com os elementos deste container que não estão no container p2
}


Ilist IlistContainer::ilist_removeall(Ilist p, int i) {
    if (p==NULL) return NULL;
    else 
        if(p->i==i) {
           Ilist r=p->next;
           free_ilist(p);
           p=NULL;
           return ilist_removeall(r,i);
        }
        else {
                p->next=ilist_removeall(p->next,i);
                return p;
        }
}


Ilist IlistContainer::ilist_removeall(int i) {
    Ilist aux=ilist_removeall(head,i);
    head=aux;
    return aux;
}


Ilist IlistContainer::ilist_intersect(Ilist a, Ilist b) {
   if (a == NULL) return NULL;
   else {
    Ilist r = ilist_intersect(a->next, b);
    if (ilist_member(b, a->i)) return ilist_prepend(r, a->i);
    else return r;
  }
}

void IlistContainer::ilist_intersect(IlistContainer &a, IlistContainer &b) {
    head=ilist_intersect(a.get_head(), b.get_head());
}

Ilist IlistContainer::ilist_union(Ilist a, Ilist b) {
  if (a == NULL) return _ilist_set(b);  /* copies members of b (does not change b) */
  else if (ilist_member(b, a->i)) return ilist_union(a->next, b);
  else   return ilist_prepend(ilist_union(a->next, b), a->i);
}

void IlistContainer::ilist_union(IlistContainer &p1, IlistContainer &p2) {
    head=ilist_union(p1.get_head(), p2.get_head());
}

Ilist IlistContainer::_ilist_set(Ilist m) {
     if (m == NULL) return NULL;
     else {
        Ilist s = _ilist_set(m->next);
        if (ilist_member(s, m->i))  return s;
        else return ilist_prepend(s, m->i);
  } 
}

Ilist IlistContainer::ilist_set(Ilist m) {
    head=_ilist_set(m);
    return head;
}


bool IlistContainer::ilist_is_set(Ilist a) {
  if (a == NULL)  return true;
  else if (ilist_member(a->next, a->i)) return false;
  else   return ilist_is_set(a->next);
}

bool IlistContainer::ilist_is_set(void) {
    return ilist_is_set(head);
}

bool IlistContainer::ilist_subset(Ilist a, Ilist b){
  if (a == NULL) return true;
  else if (!ilist_member(b, a->i)) return false;
  else  return ilist_subset(a->next, b);
}


bool IlistContainer::ilist_subset(IlistContainer &p){
    return ilist_subset(head, p.get_head());
}

void IlistContainer::f_printf_ilist(ostream &o, Ilist p) {
  Ilist aux=p;  
  o<<"(";
  for (; aux!=NULL; aux=aux->next) {
        o<<aux->i;
        o<< (aux->next ? " " : "");
  }
  o<<")"<<endl;
}

void IlistContainer::f_printf_ilist(ostream &o) {
    f_printf_ilist(o,head);
}

void IlistContainer::p_ilist(Ilist p) {
    f_printf_ilist(cout, p);
}

void IlistContainer::p_ilist(void) {
        f_printf_ilist(cout);
}


Ilist IlistContainer::ilist_copy(Ilist p) {
    if (p == NULL)  return NULL;
    else {
        Ilist n = get_ilist();
        n->i = p->i;
        n->next = ilist_copy(p->next);
        return n;
  }
}

Ilist IlistContainer::ilist_copy(void) {
    return ilist_copy(head);
}

Ilist IlistContainer::ilist_remove_last(Ilist p) {
  if (p == NULL) return NULL;
  else 
    if (p->next == NULL) {
            free_ilist(p);
            return NULL;
  }
  else {
        p->next = ilist_remove_last(p->next);
        return p;
  }
    
}


Ilist IlistContainer::ilist_remove_last(void) {
    return ilist_remove_last(head);
}

int IlistContainer::ilist_occurrences(Ilist p, int i) {
    if (p == NULL) return 0;
    else  return ilist_occurrences(p->next, i) + (p->i == i ? 1 : 0);
}

int IlistContainer::ilist_occurrences(int i) {
    return ilist_occurrences(head, i);
}

Ilist IlistContainer::ilist_insert_up(Ilist p, int i){
  if (p == NULL || p->i >= i) {
                                    Ilist g = get_ilist();
                                    g->i = i;
                                    g->next = p;
                                    return g;
  }
  else {
        p->next = ilist_insert_up(p->next, i);
        return p;
  }
}


Ilist IlistContainer::ilist_insert_up(int i) {
    head=ilist_insert_up(head,i);
    return head;
        
}

int IlistContainer::position_in_ilist(int i , Ilist p) {
  if (p == NULL) return -1;
  else if (p->i == i) return 1;
  else {
        int pos = position_in_ilist(i, p->next);
        if (pos == -1) return -1;
        else  return pos+1;
 }
}

int IlistContainer::position_in_ilist(int i) {
    return position_in_ilist(i, head);
}

Ilist IlistContainer::rev_app_ilist(Ilist p1, Ilist p2) {
  Ilist p3;
  if (p1 == NULL) return(p2);
  else {
    p3 = p1->next;
    p1->next = p2;
    return(rev_app_ilist(p3, p1));
  }
}


Ilist IlistContainer::reverse_ilist(Ilist p) {
    return rev_app_ilist(p, NULL);
} 

Ilist IlistContainer::reverse_ilist(void) {
    Ilist aux=head;
    head=reverse_ilist(aux);
    return head;
}


I2listContainer::I2listContainer() {
	head=NULL;
}

I2listContainer::~I2listContainer() {
	head=NULL;
}

void I2listContainer::free_i2list(I2list p){
  LADR_GLOBAL_G_LIST.I2list_frees++;
  Memory::memFree(p, sizeof(struct i2list));
}

I2list I2listContainer::get_i2list(void) {
  I2list p = (I2list) Memory::memNew(sizeof( struct i2list));
  p->next = NULL;
  LADR_GLOBAL_G_LIST.I2list_gets++;
  return(p);
}


void I2listContainer::zap_i2list(I2list p) {
  free_i2list(p);
  p=NULL;
}

void I2listContainer::zap_i2list(void) {
  I2list curr, prev;
  curr = head;
    while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    zap_i2list(prev);
    prev=NULL;
  }
  head=NULL;
}




I2list I2listContainer::i2list_append(I2list lst, int i, int j) {
    if (lst == NULL) {
                        I2list g = get_i2list();
                        g->i = i;
                        g->j = j;
                        g->next = NULL;
                        return g;
    }
    else {
            lst->next = i2list_append(lst->next, i, j);
            return lst;
    }
}


I2list I2listContainer::i2list_append(int i, int j) {
    head= i2list_append(head, i,j);
    return head;
}

I2list I2listContainer::i2list_prepend(I2list lst, int i, int j) {
  I2list g = get_i2list();
  g->i = i;
  g->j = j;
  g->next = lst;
  return g;
}

I2list I2listContainer::i2list_prepend(int i, int j) {
    head= i2list_prepend(head,i,j);
    return head;
}


void I2listContainer::f_printf_i2list(ostream &o, I2list p) {
  I2list aux=p;  
  o<<"(";
  for (; aux!=NULL; aux=aux->next) {
        o<<"("<<aux->i <<","<<aux->j<<")";
        o<< (aux->next ? " " : "");
  }
  o<<")"<<endl;
}

void I2listContainer::f_printf_i2list(ostream &o) {
    f_printf_i2list(o,head);
}


void  I2listContainer::p_i2list(void) {
    f_printf_i2list(cout); 
}


I2list I2listContainer::i2list_removeall(I2list p, int i) {
  if (p == NULL) return NULL;
  else if (p->i == i) {
                        I2list r = p->next;
                        free_i2list(p);
                        p=NULL;
                        return i2list_removeall(r, i);
  }
  else {
            p->next = i2list_removeall(p->next, i);
            return p;
  }
}

I2list I2listContainer::i2list_removeall(int i) {
    I2list aux=head;
    aux=i2list_removeall(aux,i);
    head=aux;
    return aux;
}



I2list I2listContainer::i2list_member(I2list lst, int i) {
  if (lst == NULL) return NULL;
  else if (lst->i == i) return lst;
  else return i2list_member(lst->next, i);
}


I2list I2listContainer::i2list_member(int i) {
    return i2list_member(head,i);
}


int I2listContainer::i2list_count(I2list lst) {
  I2list p=lst;
  int n;
  for (n = 0; p; p = p->next, n++);
  return(n);
}

int I2listContainer::i2list_count(void) {
    return i2list_count(head);
}

I2list I2listContainer::alist_insert(I2list p, int key, int val) {
  return i2list_prepend(p, key, val);
}


I2list I2listContainer::alist_insert(int key, int val) {
    return i2list_prepend(key,val);
}


int I2listContainer::assoc(I2list p, int key) {
  if (p == NULL) return INT_MIN;
  else if (p->i == key) return p->j;
  else return assoc(p->next, key);
}

int I2listContainer::assoc(int key) {
    return assoc(head, key);
}

bool I2listContainer::i2list_multimember(I2list b, int i, int n) {
    if (b == NULL) return false;
    else if (i == b->i)  return n <= b->j;
    else return i2list_multimember(b->next, i, n);
}

bool I2listContainer::i2list_multimember(int i, int n) {
    return i2list_multimember(head, i,n);    
}

bool I2listContainer::i2list_multisubset(I2list a, I2list b) {
    if (a == NULL)  return true;
    else if (!i2list_multimember(b, a->i, a->j)) return false;
    else  return i2list_multisubset(a->next, b);
}




bool I2listContainer::i2list_multisubset(I2listContainer &lst) {
    I2list b=lst.head;
    I2list a=head;
    return i2list_multisubset(a,b);
}

I2list I2listContainer::multiset_add_n(I2list a, int i, int n) {
if (a == NULL) {
    a = get_i2list();
    a->i = i;
    a->j = n;
  }
  else if (a->i == i)
    a->j += n;
  else
    a->next = multiset_add_n(a->next, i, n);
  return a;
}
                                        
I2list I2listContainer::multiset_add_n(int i, int n) {
    head=multiset_add_n(head, i, n);
    return head;
}
 
 
 
I2list I2listContainer::multiset_add(I2list a, int i) {
     return multiset_add_n(a, i, 1);
}
 
I2list I2listContainer::multiset_add(int i) {
    head=multiset_add(head,i);
    return head;
}


I2list I2listContainer::multiset_union(I2list a , I2list b) {
I2list p;
  for (p = b; p; p = p->next)
    a = multiset_add_n(a, p->i, p->j);
  zap_i2list(b);
  return a; 
}

I2list I2listContainer::multiset_union(I2list b) {
    head= multiset_union(head,b);
    return head;    
}

Ilist IlistContainer::multiset_to_set(I2list m) {
  if (m == NULL) return NULL;
  else {
    Ilist p = get_ilist();
    p->i = m->i;
    p->next = multiset_to_set(m->next);
    return p;
  }
}

int I2listContainer::multiset_occurrences (I2list m, int i) {
    I2list a = i2list_member(m, i);
    return (a == NULL ? 0 : a->j);
}


int  I2listContainer::multiset_occurrences(int i) {
    return multiset_occurrences(head,i);
}
    
    
    
    
    
I2list I2listContainer::get_head(void){
        return head;
}

    
void I2listContainer::set_head(I2list l) {
    head=l;
}



I3list I3listContainer::get_head(void){
        return head;
}

    
void I3listContainer::set_head(I3list l) {
    head=l;
}

I3list I3listContainer::get_i3list(void) {

  I3list p = (I3list) Memory::memNew(sizeof( struct i3list));
  p->next = NULL;
  LADR_GLOBAL_G_LIST.I3list_gets++;
  return(p);  
}


void I3listContainer::free_i3list(I3list p) {
  LADR_GLOBAL_G_LIST.I3list_frees++;
  Memory::memFree(p, sizeof( struct i3list));
}
                                                

I3listContainer::I3listContainer() {
    head=NULL;
}


I3listContainer::~I3listContainer() {
    head=NULL;
}
    
    
bool I3listContainer::i3list_member(I3list lst, int i, int j, int k) {
  if (lst == NULL) return false;
  else if (lst->i == i && lst->j == j && lst->k == k) return true;
  else return i3list_member(lst->next, i, j, k);
}

bool I3listContainer::i3list_member(int i, int j, int k) {
    return i3list_member(head,i,j,k);
}


I3list I3listContainer::i3list_append(I3list lst, int i, int j, int k) {
  if (lst == NULL) {
        I3list g = get_i3list();
        g->i = i;
        g->j = j;
        g->k = k;
        g->next = NULL;
        return g;
  }
  else {
        lst->next = i3list_append(lst->next, i, j, k);
        return lst;
  }
    
}

I3list I3listContainer::i3list_append(int i, int j, int k) {
    I3list aux=head;
    head= i3list_append(aux,i,j,k);
    return head;
}


void I3listContainer::zap_i3list(void) {
    zap_i3list(head);
    head=NULL;
}

void I3listContainer::zap_i3list(I3list p) {
I3list curr, prev;

  curr = p;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    free_i3list(prev);
  }
    
}


I3list I3listContainer::i3list_prepend(I3list lst, int i, int j, int k){
  I3list g = get_i3list();
  g->i = i;
  g->j = j;
  g->k = k;
  g->next = lst;
  return g;
}

I3list I3listContainer::i3list_prepend(int i, int j, int k) {
    I3list aux=i3list_prepend(head, i,j,k);
    head=aux;
    return aux;    
}



I3list I3listContainer::rev_app_i3list(I3list p1, I3list p2) {

  I3list p3;
 if (p1 == NULL) return(p2);
  else {
    p3 = p1->next;
    p1->next = p2;
    return(rev_app_i3list(p3, p1));
  }
} 

I3list I3listContainer::reverse_i3list (I3list p) {
    return rev_app_i3list(p, NULL);
}

I3list I3listContainer::reverse_i3list(void) {
    I3list aux=head;
    head= reverse_i3list(aux);
    return head;
}

int I3listContainer::i3list_count(I3list p) {
  int n;
  for (n = 0; p; p = p->next, n++);
  return(n);
}

int I3listContainer::i3list_count(void) {
    I3list aux=head;
    return i3list_count(aux);
}


I3list I3listContainer::copy_i3list(I3list p){
      I3list start, p1, p2;
      start = NULL;
      p2 = NULL;
      for ( ; p; p = p->next) {
                    p1 = get_i3list();
                    p1->i = p->i;
                    p1->j = p->j;
                    p1->k = p->k;
                    p1->next = NULL;
                    if (p2)  p2->next = p1;
                    else start = p1;
                    p2 = p1;
  }
  return(start);
}

I3list I3listContainer::copy_i3list(void) {
    I3list aux=copy_i3list(head);
    return aux;
}


I3list I3listContainer::alist2_insert(I3list p, int key, int a, int b) {
  return i3list_prepend(p, key, a, b);
}

I3list I3listContainer::alist2_insert(int key, int a, int b) {
    I3list aux=alist2_insert(head, key,a,b);
    return aux;
}

int I3listContainer::assoc2a(I3list p, int key) {
  if (p == NULL) return INT_MIN;
  else if (p->i == key)  return p->j;
  else return assoc2a(p->next, key);
}

int I3listContainer::assoc2b(I3list p, int key) {
     if (p == NULL)  return INT_MIN;
     else if (p->i == key) return p->k;
     else return assoc2b(p->next, key); 
}

int I3listContainer::assoc2a(int key) {

    return assoc2a(head, key);
}

int I3listContainer::assoc2b(int key) {

    return assoc2b(head,key);
}


I3list I3listContainer::alist2_remove(I3list p, int key){

  /*
  if (p == NULL) return NULL;
  else {
        p->next= alist2_remove(p->next, key);
        if (p->i == key) {
                            I3list a = p;
                            p = p->next;
                            free_i3list(a);
        }
    return p;
  }*/
  
 //BV- 2016-nov-20 - non recursive way
 /* remove key members from beginning and remember new head */
  while ((p != NULL) && (p->i == key)) {
    I3list del = p;
    p = p->next;
    free_i3list(del);
  }
  if (p == NULL) {
    return NULL;
  }
  
  /* remove key members from the rest of the list */
  I3list prev = p; /* previous non-key member */
  I3list curr = p->next;
  while (curr != NULL) {
    I3list next = curr->next;
    if (curr->i == key) {
      prev->next = curr->next;
      free_i3list(curr);
    }
    else {
      prev = curr;
    }
    curr = next;
  }
  return p; /* return new head */
}



I3list I3listContainer::alist2_remove(int key) {
    I3list aux=alist2_remove(head, key);
    head=aux;
    return aux;
}


PlistContainer::PlistContainer() {
    head=NULL;
}


PlistContainer::~PlistContainer() {
    head=NULL;
}

Plist PlistContainer::get_plist(void) {
  Plist p = (Plist) Memory::memNew(sizeof (struct plist));
  p->next = NULL;
  LADR_GLOBAL_G_LIST.Plist_gets++;
  return(p);
}

void PlistContainer::free_plist(Plist p)
{
     Memory::memFree(p, sizeof(struct plist));
     LADR_GLOBAL_G_LIST.Plist_frees++;
     p=NULL;
 
}

void PlistContainer::zap_plist(Plist p)
{
  Plist curr, prev;
  curr = p;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    
    //no no no mr peter
    //p zap_plist não é responsável por libertar o seu conteúdo
    //apenas liberta os seus elementos
    /*
    if(prev->v) {
        free(prev->v);
        prev->v=NULL;
    }
    */
    free_plist(prev);
    prev=NULL;
  }  
}

void PlistContainer::zap_plist() {
    zap_plist(head);
    head=NULL;
}


Plist PlistContainer::plist_append(Plist lst, void *v) {
    if (lst == NULL) {
        Plist g = get_plist();
        g->v = v;
        g->next = NULL;
        return g;
    }
    else {
    lst->next = plist_append(lst->next, v);
    return lst;
  }
    
}

Plist PlistContainer::plist_append( void *v) {
    head=plist_append(head, v);
    return head;
}


Plist PlistContainer::get_head(void) {
    return head;
}

void  PlistContainer::set_head(Plist p) {
    head=p;
}

Plist PlistContainer::plist_prepend(Plist lst, void *v) {
  Plist g = get_plist();
  g->v = v;
  g->next = lst;
  return g;
}

Plist PlistContainer::plist_prepend(void *v) {
    head=plist_prepend(head,v);
    return head;
}



Plist PlistContainer::plist_pop(Plist p) {
  Plist q = p;
  p = p->next;
  free_plist(q);
  return p;
}

Plist PlistContainer::plist_pop(void) {
Plist aux=plist_pop(head);
head=aux;
return head;
}

int PlistContainer::plist_count(Plist p) {
  int n;
  for (n = 0; p; p = p->next, n++);
  return(n);
}

int PlistContainer::plist_count(void) {
Plist aux=head;
return plist_count(aux);
}

Plist PlistContainer::copy_plist(Plist p) {
  Plist start, p1, p2;
  start = NULL;
  p2 = NULL;
  for ( ; p; p = p->next) {
    p1 = get_plist();
    p1->v = p->v;
    p1->next = NULL;
    if (p2)
      p2->next = p1;
    else
      start = p1;
    p2 = p1;
  }
  return(start);
}

Plist PlistContainer::copy_plist(void) {
Plist aux=head;
return copy_plist(aux);
}


Plist PlistContainer::plist_cat(Plist p1, Plist p2) {
  if (p1 == NULL) return p2;
  else if (p2 == NULL) return p1;
  else {
    Plist p = p1;
    while (p->next != NULL) p = p->next;
    p->next = p2;
    return p1;
  }
}

Plist PlistContainer::plist_cat(PlistContainer &p) {
    Plist aux=head;
    Plist aux1=p.get_head();
    head=plist_cat(aux,aux1);
    return head;
}

Plist PlistContainer::plist_cat(Plist p1) {
Plist aux=head;
head=plist_cat(aux, p1);
return head;
}


Plist PlistContainer::plist_cat2(Plist p1, Plist p2) {
    return plist_cat(p1, copy_plist(p2));
}

Plist PlistContainer::plist_cat2(Plist p) {
    Plist aux=head;
    head=plist_cat2(aux,p);
    return head;
}


Plist PlistContainer::plist_cat2(PlistContainer &p1) {
    Plist aux=head;
    Plist aux1=p1.get_head();
    head=plist_cat2(aux,aux1);
    return head;
}


Plist PlistContainer::rev_app_plist(Plist p1, Plist p2) {
  Plist p3;
  if (p1 == NULL) return(p2);
  else {
    p3 = p1->next;
    p1->next = p2;
    return(rev_app_plist(p3, p1));
  }
}



Plist PlistContainer::reverse_plist(Plist p) {
    //return rev_app_plist(p,NULL); //old way
   //BV - 2016-oct-24
   Plist prev, current, next;
   prev = NULL;
   current = p;
   while (current != NULL) {
     next = current->next;
     current->next = prev;
     prev = current;
     current = next;
  }
  return prev;
}

Plist PlistContainer::reverse_plist(void) {
Plist aux=head;
head=reverse_plist(aux);
return head;
}



bool PlistContainer::plist_member(Plist lst, void *v) {
  if (lst == NULL) return false;
  else if (lst->v == v)  return true;
  else return plist_member(lst->next, v);
}

bool PlistContainer::plist_member(void *v) {
return plist_member(head,v);
}



Plist PlistContainer::plist_subtract(Plist p1, Plist p2){
  if (p1 == NULL) return NULL;
  else {
            Plist r = plist_subtract(p1->next, p2);
            if (plist_member(p2, p1->v)) return r;
            else return plist_prepend(r, p1->v);
  }
}

bool PlistContainer::plist_subset(Plist a, Plist b){
  if (a == NULL) return true;
  else if (!plist_member(b, a->v)) return false;
  else return plist_subset(a->next, b);
}

Plist PlistContainer::plist_remove(Plist p, void *v){
    if (p == NULL) fatal::fatal_error("plist_remove: pointer not found");
    if (p->v == v) {
                    Plist x = p;
                    p = p->next;
                    free_plist(x);
                    return p;
    }
    else {
            p->next = plist_remove(p->next, v);
            return p;
    }
}


Plist PlistContainer::plist_subtract(Plist p1) {
    return plist_subtract(head,p1);
}

bool PlistContainer::plist_subset(Plist p) {
    return plist_subset(p,head);
}


Plist PlistContainer::plist_remove(void *v) {
    Plist aux= plist_remove(head, v);
    head=aux;
    return head;
}



Plist PlistContainer::plist_remove_string(Plist p, char *s) {
  if (p == NULL) fatal::fatal_error("plist_remove_string: pointer not found");
  if (myString::str_ident((char *)p->v, s)) {
    Plist x = p;
    p = p->next;
    free_plist(x);
    return p;
  }
  else {
        p->next = plist_remove_string(p->next, s);
        return p;
  }
}

Plist PlistContainer::plist_remove_string(char *s) {
    head=plist_remove_string(head,s);
    return head;
}

Plist PlistContainer::sort_plist(Plist objects, OrderType (*comp_proc)(void *, void *)) {
  int n = plist_count(objects);
  void **a;
  a= (void **) malloc(n * sizeof(void *));
  int i;
  Plist p;
  for (p = objects, i = 0; p; p = p->next, i++)    a[i] = p->v;
  myOrder::merge_sort(a, n, comp_proc);
  for (p = objects, i = 0; p; p = p->next, i++)    p->v = a[i];
  free(a);
  return objects;
}

Plist PlistContainer::sort_plist(OrderType (*comp_proc)(void *, void*)) {
  int n = plist_count(head);
  void **a;
  a= (void **)malloc(n * sizeof(void *));
  int i;
  Plist p;
  for (p = head, i = 0; p; p = p->next, i++)    a[i] = p->v;
  myOrder::merge_sort(a, n, comp_proc);
  for (p = head, i = 0; p; p = p->next, i++)    p->v = a[i];
  free(a);
  return head;
}

Plist PlistContainer::plist_remove_last(Plist p) {
  if (p == NULL) return NULL;
  else
  if (p->next == NULL) {
                        free_plist(p);
                        return NULL;
    }
    else {
        p->next = plist_remove_last(p->next);
        return p;
    }
}

Plist PlistContainer::plist_remove_last(void) {
    head=plist_remove_last(head);
    return head;
}


int PlistContainer::position_of_string_in_plist(char *s, Plist p){
  if (p == NULL) return -1;
   else if (myString::str_ident(s, (char *) p->v)) return 1;
   else {
            int pos = position_of_string_in_plist(s, p->next);
            if (pos == -1) return -1;
            else return pos+1;
  }
}

int PlistContainer::position_of_string_in_plist(char *s) {
    return position_of_string_in_plist(s,head);
}

bool PlistContainer::string_member_plist(char *s, Plist p) {
    return position_of_string_in_plist(s, p) >= 0;
}

bool PlistContainer::string_member_plist(char *s) {
    return string_member_plist(s,head);
}

bool PlistContainer::string_member_plist(string s) {
    return string_member_plist(s.c_str());    
}

int PlistContainer::longest_string_in_plist(Plist p){
  if (p == NULL) return -1;
   else {
    int n1 = strlen((char *)p->v);
    int n2 = longest_string_in_plist(p->next);
    return (n1>n2 ? n1:n2);
  }
}

int PlistContainer::longest_string_in_plist(void) {
    return longest_string_in_plist(head);
}


void * PlistContainer::ith_in_plist(Plist p, int i) {
 if (p == NULL || i <= 0) return NULL;
 else   if (i == 1) return p->v;
        else return ith_in_plist(p->next, i-1);
}

void * PlistContainer::plist_last(Plist p) {
if (p == NULL) return NULL;
    else if (p->next == NULL) return p->v;
    else return plist_last(p->next);
}


void * PlistContainer::ith_in_plist(int i){
    return ith_in_plist(head,i);
}


void * PlistContainer::plist_last(void) {
    return plist_last(head);
}

void PlistContainer::plist_string_print() {
    Plist aux=head;
    
    cout<<"Plist elements----------------------"<<endl;
    while (aux!=NULL) {
        cout<<  (char *)  aux->v   <<endl;
        aux=aux->next;
    }
    cout<<"------------------------------------"<<endl;
}

