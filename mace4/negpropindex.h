

#ifndef MACE4_NEGPROPINDEX_H
#define MACE4_NEGPROPINDEX_H

#include "estack.h"
#include "mstate.h"
#include "syms.h"


/*
 * negpropindex::Index
 *
 * The index is a simple discrimination tree in which each
   node is simply an array of pointers to the children.

   0:                                 Index                        void ****
   1: sign             negative                positive            void ***
   2: symbol      sym1    sym2    sym3     sym1    sym2    sym3    void **
   3: value      v1 v2   v1 v2   v1 v2    v1 v2   v1 v2   v1 v2    void *
   4:     domain element of arg 0 plus the special value "eterm"
   5:     domain element of arg 1 plus the special value "eterm"
   etc.

   After level 3, the processing is recursive, and the * on the void
   get replenished.  That's what makes the typing kludgy, but the
   compiler accepts it.

   The term space is not very big, so we construct a full tree at the start.
*/


class negpropindex {
private:
  void**** Index;

  /*
   *  The following are "env/global" data used by a number of cooperating objects (e.g. msearch, select etc)
   * to do searching for models. This object does not own any of them.  They are "pointers" passed in.
   */
  const Symbol_data  Symbols;
  const Symbol_data* Sn_to_mace_sn;
  int                Domain_size;

  void* init_recurse(int n);
  void  free_recurse(int n, void **p);
  void  p_recurse(void **p, int x, int n, int depth);
  Term  negprop_find_near_recurse(void **p, Term query, int pos, int n);

public:
  negpropindex() = delete;
  negpropindex(const negpropindex&) = delete;
  negpropindex& operator=(const negpropindex&) = delete;
  ~negpropindex();

  negpropindex(const Symbol_data s, int d, const Symbol_data* Sn);

public:
  void init_negprop_index();
  void p_negprop_index();
  void insert_recurse(void **p, Term atom, Term t, int n, Mstate state, EstackContainer& es_con);
  void insert_negprop_eq(Term atom, Term alpha, int val, Mstate state,
                         EstackContainer& es_con, const Mace4GlobalValues& mace4_gvs);
  void insert_negprop_noneq(Term atom, Mstate state,
                            EstackContainer& es_con, const Mace4GlobalValues& mace4_gv);
  Term negprop_find_near(int sign, int sym, int val, Term query, int pos);

};



#endif

