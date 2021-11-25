
#include <iomanip>

#include "../ladr/memory.h"
#include "../ladr/fatal.h"
#include "mstate.h"


Mstate
MstateContainer::get_mstate(void)
{
  Mstate p = static_cast<Mstate>(Memory::memCNew(PTRS_MSTATE * sizeof(void*)));
  p->ok = true;
  p->first_job = nullptr;
  p->last_job = nullptr;
  p->stack = nullptr;
  Mstate_gets++;
  return(p);
}

void
MstateContainer::free_mstate(Mstate p)
{
  Memory::memFree(p, PTRS_MSTATE*sizeof(void*));
  Mstate_frees++;
}

Jnode
MstateContainer::get_jnode(void)
{
  Jnode p = static_cast<Jnode>(Memory::memNew(PTRS_JNODE*sizeof(void*)));
  Jnode_gets++;
  return(p);
}

void
MstateContainer::free_jnode(Jnode p)
{
  Memory::memFree(p, PTRS_JNODE*sizeof(void*));
  Jnode_frees++;
}

/*
 * This routine prints (to FILE *fp) memory usage statistics for data types
 * associated with the mstate package.
 * The Boolean argument heading tells whether to print a heading on the table.
 */

void
MstateContainer::fprint_mstate_mem(std::ostream& fp, bool heading)
{
  if (heading)
    fp << "  type (bytes each)        gets      frees     in use      bytes\n";

  int n = sizeof(struct mstate);
  fp << "mstate (" << std::setw(4) << n << ")       " << std::setw(11) << Mstate_gets << std::setw(11) << Mstate_frees
     << std::setw(11) << Mstate_gets - Mstate_frees << std::setw(9) << std::setprecision(1) << ((Mstate_gets - Mstate_frees) * n) / 1024. << " K\n";

  n = sizeof(struct jnode);
  fp << "jnode (" << std::setw(4) << n << ")        " << std::setw(11) << Jnode_gets << std::setw(11) << Jnode_frees
     << std::setw(11) << Jnode_gets - Jnode_frees << std::setw(9) << std::setprecision(1) << ((Jnode_gets - Jnode_frees) * n) / 1024. << " K\n";
}


void
MstateContainer::job_append(Mstate s, int type, int id, Term alpha, Term beta, int pos)
{
  Jnode j = get_jnode();
  j->type = type;
  j->id = id;
  j->alpha = alpha;
  j->beta = beta;
  j->pos = pos;

  if (s->first_job == nullptr) {
    j->prev = nullptr;
    j->next = nullptr;
    s->first_job = j;
    s->last_job = j;
  }
  else {
    j->prev = s->last_job;
    j->next = nullptr;
    s->last_job->next = j;
    s->last_job = j;
  }
}

void
MstateContainer::job_prepend(Mstate s, int type, int id, Term alpha, Term beta, int pos)
{
  Jnode j = get_jnode();
  j->type = type;
  j->id = id;
  j->alpha = alpha;
  j->beta = beta;
  j->pos = pos;

  if (s->first_job == nullptr) {
    j->prev = nullptr;
    j->next = nullptr;
    s->first_job = j;
    s->last_job = j;
  }
  else {
    j->next = s->first_job;
    j->prev = nullptr;
    s->first_job->prev = j;
    s->first_job = j;
  }
}

void
MstateContainer::job_pop(Mstate s)
{
  if (s->first_job == nullptr)
    fatal::fatal_error("job_pop: empty list");
  else {
    Jnode p = s->first_job;
    s->first_job = p->next;
    if (s->first_job == nullptr)
      s->last_job = nullptr;
    else
      s->first_job->prev = nullptr;
    free_jnode(p);
  }
}

void
MstateContainer::zap_jobs(Mstate ms)
{
  Jnode j = ms->first_job;
  while (j != nullptr) {
    Jnode p = j;
    j = j->next;
    free_jnode(p);
  }
}

