#include "discrim.h"
#include "memory.h"
#include "fatal.h"
#include "symbols.h"
#include "ladrvglobais.h"
#include <iostream>
#include <iomanip>



GlobalDiscrim::GlobalDiscrim() {
    Discrim_gets=0;
    Discrim_frees=0;
    Discrim_pos_gets=0;
    Discrim_pos_frees=0;
}

GlobalDiscrim::~GlobalDiscrim() {
}



DiscrimContainer::DiscrimContainer() {

}

DiscrimContainer::~DiscrimContainer() {

}

Discrim DiscrimContainer::get_discrim(void) {
  Discrim p = (Discrim) Memory::memCNew(sizeof(struct discrim));
  LADR_GLOBAL_DISCRIM.Discrim_gets++;
  return(p);
}

/* PUBLIC */
void DiscrimContainer::free_discrim(Discrim p)
{
  Memory::memFree(p, sizeof(struct discrim));
  LADR_GLOBAL_DISCRIM.Discrim_frees++;
}  /* free_discrim */


Discrim_pos DiscrimContainer::get_discrim_pos(void){
  Discrim_pos p = (Discrim_pos) Memory::memNew(sizeof(struct discrim_pos));
  LADR_GLOBAL_DISCRIM.Discrim_pos_gets++;
  return(p);
}  /* get_discrim_pos */

void DiscrimContainer::free_discrim_pos(Discrim_pos p) {
  Memory::memFree(p,sizeof(discrim_pos));
  LADR_GLOBAL_DISCRIM.Discrim_pos_frees++;
}  /* free_discrim_pos */


/* PUBLIC */
void DiscrimContainer::fprint_discrim_mem(ostream &o, const bool heading) const{
  int n;
  if (heading)
  o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;

  n = sizeof(struct discrim);
  o<<"discrim       ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_DISCRIM.Discrim_gets;
  o<<setw(11)<<LADR_GLOBAL_DISCRIM.Discrim_frees;
  o<<setw(11)<<LADR_GLOBAL_DISCRIM.Discrim_gets-LADR_GLOBAL_DISCRIM.Discrim_frees;
  o<<setw(9)<<(LADR_GLOBAL_DISCRIM.Discrim_gets-LADR_GLOBAL_DISCRIM.Discrim_frees)*n /1024<<"K"<<endl;

  n = sizeof(struct discrim_pos);
  o<<"discrim_pos   ("<<n<<")        ";
  o<<setw(11)<<LADR_GLOBAL_DISCRIM.Discrim_pos_gets;
  o<<setw(11)<<LADR_GLOBAL_DISCRIM.Discrim_pos_frees;
  o<<setw(11)<<LADR_GLOBAL_DISCRIM.Discrim_pos_gets-LADR_GLOBAL_DISCRIM.Discrim_pos_frees;
  o<<setw(9)<<(LADR_GLOBAL_DISCRIM.Discrim_pos_gets-LADR_GLOBAL_DISCRIM.Discrim_pos_frees)*n /1024<<"K"<<endl;
}  /* fprint_discrim_mem */

/* PUBLIC */
void DiscrimContainer::p_discrim_mem(void) const{
  fprint_discrim_mem(cout, true);
}  /* p_discrim_mem */

/*************
 *
 *   discrim_init()
 *
 *************/

/* DOCUMENTATION
This routine allocates and returns an empty discrimination index.
It can be used for either wild or tame indexing.
*/

/* PUBLIC */
Discrim DiscrimContainer::discrim_init(void){
  return get_discrim();
}  /* discrim_init */

/*************
 *
 *   discrim_dealloc(d)
 *
 *************/

/* DOCUMENTATION
This routine frees an empty discrimination index (wild or tame).
*/

/* PUBLIC */
void DiscrimContainer::discrim_dealloc(Discrim d){
  if (d->u.kids) {
    fatal::fatal_error("discrim_dealloc, nonempty index.");
  }
  else
    free_discrim(d);
}  /* discrim_dealloc */


/*************
 *
 *   zap_discrim_tree()
 *
 *************/


void DiscrimContainer::zap_discrim_tree(Discrim d, int n) {
  if (n == 0) {
        PlistContainer P;
        P.set_head(d->u.data);
        P.zap_plist();
  }
  else {
    int arity;
    Discrim k, prev;
    SymbolContainer S;
    k = d->u.kids;
    while (k != NULL) {
      if (k->type == DiscriminationTreeNode::AC_ARG_TYPE || k->type==DiscriminationTreeNode::AC_NV_ARG_TYPE) arity = 0;
      else if (DVAR(k))arity = 0;
      else arity = S.sn_to_arity(k->symbol);
      prev = k;
      k = k->next;
      zap_discrim_tree(prev, n+arity-1);
    }
  }
  free_discrim(d);
}  /* zap_discrim_tree */

/*************
 *
 *   destroy_discrim_tree()
 *
 *************/

/* DOCUMENTATION
This routine frees all the Memory:: associated with a discrimination
index.  It can be used with either wild or tame trees.
*/

/* PUBLIC */
void DiscrimContainer::destroy_discrim_tree(Discrim d){
  zap_discrim_tree(d, 1);
}  /* destroy_discrim_tree */

/*************
 *
 *   discrim_empty()
 *
 *************/

/* DOCUMENTATION
This Boolean function checks if a discrimination index is empty.
It can be used with either wild or tame trees.
*/

/* PUBLIC */
bool DiscrimContainer::discrim_empty(Discrim d){
  return (d == NULL ? true : (d->u.kids == NULL ? true : false));
}  /* discrim_empty */


