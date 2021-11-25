
#include <iomanip>
#include "estack.h"



Estack
EstackContainer::get_estack(void)
{
  Estack p;

  Estack_gets++;
  if (Estack_avail == nullptr)
    p = new estack();
  else {
    Estack_avails--;
    p = Estack_avail;
    Estack_avail = Estack_avail->next;
  }

  /* no initialization */

  return(p);
}

void
EstackContainer::free_estack(Estack p)
{
  Estack_frees++;
  Estack_avails++;
  p->next = Estack_avail;
  Estack_avail = p;
}

void
EstackContainer::fprint_estack_mem(std::ostream& fp, bool heading)
{
  if (heading)
    fp << "  type (bytes each)        gets      frees     in use      bytes\n";

  int n = sizeof(struct estack);

  fp << "estack (" << std::setw(4) << n << ")       " << std::setw(11) << Estack_gets << std::setw(11) << Estack_frees
     << std::setw(11) << Estack_gets - Estack_frees << std::setw(9) << std::setprecision(1) << ((Estack_gets - Estack_frees) * n) / 1024.0
	 << " K (avail=" << Estack_avails << ", " << std::setprecision(1) << (Estack_avails * n) / 1024.0 << " K)\n";
}

void
EstackContainer::free_estack_memory(void)
{
  Estack a = Estack_avail;
  while (a != nullptr) {
    Estack b = a;
    a = a->next;
    delete b;
    Estack_avails--;
  }
  Estack_avail = nullptr;
}

int
EstackContainer::estack_bytes(void)
{
  return ((Estack_gets - Estack_frees) + Estack_avails) * sizeof(estack);
}

Estack
EstackContainer::update_and_push(void **p, void *new_ptr, Estack stack)
{
  if (stack == nullptr || stack->n == estack::ESTACK_SIZE) {
    Estack s = get_estack();  /* allocate a stack entry */
    s->n = 0;
    s->next = stack;
    stack = s;
  }

  stack->p[stack->n] = p;              /* record the location */
  stack->v[stack->n] = *p;             /* record the old value */
  *p = new_ptr;                        /* make the assignment */
  stack->n++;
  return stack;                        /* return the updated stack */
}

void
EstackContainer::restore_from_stack(Estack stack)
{
  while (stack != nullptr) {
    Estack s = stack;
    stack = stack->next;
    for (int i = s->n-1; i >= 0; i--) {
      *(s->p[i]) = s->v[i];  /* restore */
    }
    free_estack(s);
  }
}

void
EstackContainer::zap_estack(Estack stack)
{
  while (stack != nullptr) {
    Estack s = stack;
    stack = stack->next;
    free_estack(s);
  }
}
