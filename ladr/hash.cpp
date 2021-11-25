#include "hash.h"
#include "memory.h"
#include "fatal.h"
#include "ladrvglobais.h"
#include <iomanip>
#include <iostream>

using namespace std;




GlobalHash::GlobalHash() {
    HashTab_gets= 0;
    HashTab_frees=0; 
    HashNode_gets=0;
    HashNode_frees=0;
}

GlobalHash::~GlobalHash() {

    
}



HashtabContainer::HashtabContainer() {
    headTab=NULL;
}


HashtabContainer::~HashtabContainer() {
    headTab=NULL;
}

Hashtab HashtabContainer::get_hashtab(void) {
  Hashtab p = (Hashtab) Memory::memCNew(sizeof(struct hashtab));
  LADR_GLOBAL_HASH.HashTab_gets++;
  return(p);
}  /* get_hashtab */

/*************
 *
 *    free_hashtab()
 *
 *************/


void HashtabContainer::free_hashtab(Hashtab p) {
  Memory::memFree(p, sizeof(struct hashtab));
  LADR_GLOBAL_HASH.HashTab_frees++;
}  /* free_hashtab */



void HashtabContainer::fprint_hash_mem(ostream &o, bool heading){
int n=0;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  
  
    n=sizeof(hashtab);
    
    o<<"hashtab     ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_HASH.HashTab_gets<<setw(11)<<LADR_GLOBAL_HASH.HashTab_frees<<setw(11)<<LADR_GLOBAL_HASH.HashTab_gets - LADR_GLOBAL_HASH.HashTab_frees;
    o<<setw(9)<<( (LADR_GLOBAL_HASH.HashTab_gets - LADR_GLOBAL_HASH.HashTab_frees) * n) / 1024 <<"K"<<endl;
    
    n = sizeof(struct hashnode);
    o<<"hashnode    ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_HASH.HashNode_gets<<setw(11)<<LADR_GLOBAL_HASH.HashNode_frees<<setw(11)<<LADR_GLOBAL_HASH.HashNode_gets - LADR_GLOBAL_HASH.HashNode_frees;
    o<<setw(9)<<((LADR_GLOBAL_HASH.HashNode_gets - LADR_GLOBAL_HASH.HashNode_frees) * n) / 1024<<"K"<<endl;
}
  
  
  
void HashtabContainer::p_hash_mem(void){
  fprint_hash_mem(cout, true);
}  




Hashnode HashtabContainer::get_hashnode(void){
  Hashnode p = (Hashnode) Memory::memCNew(sizeof(struct hashnode));
  LADR_GLOBAL_HASH.HashNode_gets++;
  return(p);
}  /* get_hashnode */

/*************
 *
 *    free_hashnode()
 *
 *************/


void HashtabContainer::free_hashnode(Hashnode p){
  Memory::memFree(p, sizeof(struct hashnode));
  LADR_GLOBAL_HASH.HashNode_frees++;
}  /* free_hashnode */



Hashtab HashtabContainer::private_hash_init(int size) {
  Hashtab p = get_hashtab();
  p->size = size;
  p->table = (Hashnode *) calloc(size, sizeof(struct hashnode));
  return p;
}  /* hash_init */


void HashtabContainer::hash_init(int size) {
    headTab=private_hash_init(size);
}



Hashtab HashtabContainer::get_head(void) {
    return headTab;
}

void HashtabContainer::set_head(Hashtab h) {
    headTab=h;
}




void HashtabContainer::hash_insert(void *v, unsigned hashval, Hashtab h) {
  int i = hashval % h->size;
  Hashnode novo = get_hashnode();
  novo->v = v;
  novo->next = h->table[i];
  h->table[i] = novo;
}


void HashtabContainer::hash_insert(void *v, unsigned hashval) {
    hash_insert(v,hashval, headTab);
}



void *HashtabContainer::hash_lookup(void *v, unsigned hashval, Hashtab h,bool (*id_func) (void *, void *)) {
  Hashnode p = h->table[hashval % h->size];
  while (p && !(*id_func)(v, p->v))
    p = p->next;
  return (p ? p->v : NULL);
} 


void HashtabContainer::hash_delete(void *v, unsigned hashval, Hashtab h,bool (*id_func) (void *, void *)) {
  
  int i = hashval % h->size;
  Hashnode p = h->table[i];
  Hashnode prev = NULL;
  
  while (p && !(*id_func)(v, p->v)) {
    prev = p;
    p = p->next;
  }

  if (!p)
    fatal::fatal_error("hash_delete, object not found");
  
  if (prev)
    prev->next = p->next;
  else
    h->table[i] = p->next;
  free_hashnode(p);
}  /* hash_delete */



void HashtabContainer::hash_destroy(Hashtab h) {
  int i;
  for (i = 0; i < h->size; i++) {
    Hashnode p = h->table[i];
    while (p != NULL) {
      Hashnode p2 = p;
      p = p->next;
      free_hashnode(p2);
    }
  }
  free(h->table);
  free_hashtab(h);
}  /* hash_destroy */



void HashtabContainer::hash_info(Hashtab h) {
  int i;
  cout<<endl<<"Hash info size="<<h->size<<endl;
  for (i = 0; i < h->size; i++) {
    Hashnode p;
    int n = 0;
    for (p = h->table[i]; p; p = p->next)
      n++;
    if (n > 0)
      cout<<"    bucket "<<i<<" has "<<n<<" objects"<<endl;
      
  }
  cout<<endl;
}  

void * HashtabContainer::hash_lookup(void *v, unsigned hashval, bool (*id_func) (void *, void *)) {
    return hash_lookup(v,hashval, headTab, id_func);
}

void HashtabContainer::hash_delete(void *v, unsigned hashval,  bool (*id_func) (void *, void *)) {
    hash_delete(v,hashval,headTab,id_func);
}

                            
                               
void HashtabContainer::hash_destroy(void) {
    hash_destroy(headTab);
}


void HashtabContainer::hash_info(void) {
    hash_info(headTab);
}
