
#ifndef MACE4_GROUND_H
#define MACE4_GROUND_H

#include <cmath>
#include <ostream>
#include "../ladr/term.h"
#include "../ladr/termflag.h"
#include "../ladr/topform.h"

#include "mace4globalvalues.h"
#include "arithmetic.h"
#include "cell.h"
#include "mstate.h"
#include "mstats.h"

typedef struct mclause* Mclause;

struct mclause {
  Term*   lits;
  Mclause next;
  bool    subsumed;
  int     numlits;
  /* The following union exists in case long int is smaller than pointer;
     This is because "active" is sometimes handled as a pointer.
  */
  union {
    long int active;
    void*    for_padding_only;
  } u;
};

class Ground {
public:
  static constexpr int BYTES_POINTER = sizeof(void*);
  // i.e the size of mclause but padded to pointer alignment so that what follows mclause can hold an array of pointers
  static constexpr int PTRS_MCLAUSE = std::round(ceil(sizeof(struct mclause)/BYTES_POINTER));

private:
  unsigned Mclause_gets;
  unsigned Mclause_frees;
  unsigned Mclause_use;
  unsigned Mclause_high;

  /*
   * The following are "env/global" data used by a number of cooperating objects (e.g. msearch, select etc)
   * to do searching for models. This object does not own any of them.  They are "pointers" passed in.
   */
  int                 Domain_size;
  const Term*         Domain;
  const Symbol_data*  Sn_to_mace_sn;
  const Cell          Cells;
  struct mace_stats*  Mstats;
  const arithmetic*   Arith;
  const Mace4GlobalValues* Mace4_gv;

public:
  Ground() = delete;
  Ground(const Ground&) = delete;
  Ground& operator=(const Ground&) = delete;

  Ground(int ds, const Term* t, const Symbol_data* s, const Cell c, struct mace_stats* m, const arithmetic* a, const Mace4GlobalValues* gv);

private:
  Mclause get_mclause(int numlits);
  void    free_mclause(Mclause p);
  bool    member(Term x, Term t);
  Term    merge(Term t);
  Term    simp_term(Term t);
  Plist   term_to_lits(Term t);

public:
  static inline Term LIT(Mclause c, int i) {return c->lits[i];}
  static inline Term* LIT_l(Mclause c, int i) {return &(c->lits[i]);}
  static inline void LIT(Mclause c, int i, Term atom) {c->lits[i] = atom;}
  static inline bool FALSE_TERM(Term t, const Term* Domain) {return t == Domain[0];}
  static inline bool TRUE_TERM(Term t, const Term* Domain) {return t == Domain[1];}

  static void p_mclause(Mclause c, const Mace4GlobalValues& mace4_gv);
  static int  lit_position(Mclause parent, Term child);

  Term    simp_tv(Term t);
  void    fprint_mclause_mem(std::ostream& fp, bool heading);
  void    zap_mterm(Term t);
  void    zap_mclause(Mclause c);
  Mclause containing_mclause(Term t);
  Term    containing_mliteral(Term t);
  Mclause term_to_mclause(Term t);
  void    set_parent_pointers(Term t);
  Term    subst_domain_elements_term(Term t, int *vals);
  bool    eterm(Term t, int* pid);
  void    eterm_index_term(Term t);
  bool    check_that_ground_clauses_are_true(Plist Ground_clauses);
};


#endif

