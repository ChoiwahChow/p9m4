#include "lindex.h"
#include "memory.h"
#include "fatal.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>



GlobalLindex::GlobalLindex() {
    Lindex_gets=0;
    Lindex_frees=0;
}

GlobalLindex::~GlobalLindex() {
}



LindexContainer::LindexContainer() {
lindexHead=NULL;
}
LindexContainer::~LindexContainer() {
	lindexHead=NULL;
}

Lindex LindexContainer::get_lindex(void) {
  Lindex p = (Lindex) Memory::memCNew(sizeof(lindex));
  LADR_GLOBAL_LINDEX.Lindex_gets++;
  return(p);
}

void LindexContainer::free_lindex(Lindex p) {
  Memory::memFree((void *)p, sizeof(lindex));
  LADR_GLOBAL_LINDEX.Lindex_frees++;
}

void LindexContainer::fprint_lindex_mem(ostream &o, bool heading) {
  int n;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct lindex);
  o<<"lindex      ("<<setw(4)<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_LINDEX.Lindex_gets;
  o<<setw(11)<<LADR_GLOBAL_LINDEX.Lindex_frees;
  o<<setw(11)<<LADR_GLOBAL_LINDEX.Lindex_gets-LADR_GLOBAL_LINDEX.Lindex_frees;
  o<<setw(9)<< ( (LADR_GLOBAL_LINDEX.Lindex_gets-LADR_GLOBAL_LINDEX.Lindex_frees)*n)/1024<<"K"<<endl;

}

/* PUBLIC */
void LindexContainer::p_lindex_mem() {
  fprint_lindex_mem(cout, true);
} 

Lindex LindexContainer::lindex_init(Mindextype pos_mtype, Uniftype pos_utype, int pos_fpa_depth, Mindextype neg_mtype, Uniftype neg_utype, int neg_fpa_depth)
{
  MindexContainer M;
  Lindex ldx = get_lindex();

  ldx->pos = M.mindex_init(pos_mtype, pos_utype, pos_fpa_depth);
  ldx->neg = M.mindex_init(neg_mtype, neg_utype, neg_fpa_depth);

  return ldx;
} 

void LindexContainer::lindex_destroy(Lindex ldx) {
  MindexContainer M;
  M.mindex_destroy(ldx->pos);
  M.mindex_destroy(ldx->neg);
  free_lindex(ldx);
} 

void LindexContainer::lindex_update(Lindex ldx, Topform c, Indexop op) {
  Literals lit;
  MindexContainer M;
  for (lit = c->literals; lit != NULL; lit = lit->next) {
    if (lit->sign)
      M.mindex_update(ldx->pos, lit->atom, op);
    else
      M.mindex_update(ldx->neg, lit->atom, op);
  }
} 

void LindexContainer::lindex_update_first(Lindex ldx, Topform c, Indexop op) {
  Literals lit = c->literals;
  MindexContainer M;
  if (lit) {
    if (lit->sign)
      M.mindex_update(ldx->pos, lit->atom, op);
    else
      M.mindex_update(ldx->neg, lit->atom, op);
  }
} 

bool LindexContainer::lindex_empty(Lindex idx) {
  MindexContainer M;
  return M.mindex_empty(idx->pos) && M.mindex_empty(idx->neg);
} 

bool LindexContainer::lindex_backtrack(Lindex idx) {
  return (idx->pos->unif_type == Uniftype::BACKTRACK_UNIF ||
	  idx->neg->unif_type == Uniftype::BACKTRACK_UNIF);
}
