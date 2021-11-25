
#ifndef MACE4_MSTATE_H
#define MACE4_MSTATE_H

#include <cmath>
#include <ostream>
#include "../ladr/term.h"

#include "estack.h"

/* Public definitions */

typedef struct jnode*  Jnode;
typedef struct mstate* Mstate;

// TODO: [choiwah] initialize
struct mstate {
  Estack stack;
  Jnode  first_job;
  Jnode  last_job;
  bool   ok;
};

struct jnode {
  int type;
  int id;
  Term alpha;
  Term beta;
  int pos;
  Jnode prev;
  Jnode next;
};

class MstateContainer {
public:
  unsigned Mstate_gets;
  unsigned Mstate_frees;
  unsigned Jnode_gets;
  unsigned Jnode_frees;

  static constexpr int PTRS_MSTATE = std::round(ceil(sizeof(struct mstate)/sizeof(void*)));
  static constexpr int PTRS_JNODE = std::round(ceil(sizeof(struct jnode)/sizeof(void*)));

  MstateContainer(): Mstate_gets(0), Mstate_frees(0), Jnode_gets(0), Jnode_frees(0) {}

  Mstate get_mstate(void);
  void   free_mstate(Mstate p);
  Jnode  get_jnode(void);
  void   free_jnode(Jnode p);
  void   fprint_mstate_mem(std::ostream& fp, bool heading);
  void   job_append(Mstate s, int type, int id, Term alpha, Term beta, int pos);
  void   job_prepend(Mstate s, int type, int id, Term alpha, Term beta, int pos);
  void   job_pop(Mstate s);
  void   zap_jobs(Mstate ms);
};

/* End of public definitions */

#endif

