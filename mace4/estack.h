
#ifndef MACE4_ESTACK_H
#define MACE4_ESTACK_H

/* INTRODUCTION
Estacks can be used to manage generic pointer assignments
that must be undone at a later time.  This is typically
used in backtracking procedures that work by moving
pointers around.  Say you need to assign the value of
one pointer *a to another pointer *b, and some time later
you'll need to undo that assignment, restoring a to its
old value.  Instead of using a special-purpose data structure
to store a's old value, you can write
<pre>
    estack = update_and_push((void **) &a, b, estack);
    ...
    restore_from_stack(estack);
</pre>
The update_and_push call assigns b to a and records the
assignment in the Estack.  The restore_from_stack call undoes
a whole stack of assignments.

<P>
This mechanism uses void pointers for both a and b, so
it works for any kind of pointer.  If b is not a pointer
(for example an integral type), make sure it is the same
size as a pointer.
*/

#include <ostream>

/* Public definitions */

typedef struct estack* Estack;

struct estack {
  static constexpr int ESTACK_SIZE = 400;

  void **p[ESTACK_SIZE];
  void *v[ESTACK_SIZE];
  int n;
  Estack next;

  estack(const estack&) = delete;
  estack& operator=(const estack&) = delete;
  estack() : n(0), next(nullptr) {}
};

class EstackContainer {
public:
  unsigned Estack_gets;
  unsigned Estack_frees;
  unsigned Estack_avails;
  Estack   Estack_avail;

  EstackContainer(const EstackContainer&) = delete;
  EstackContainer& operator=(const EstackContainer&) = delete;
  EstackContainer(): Estack_gets(0), Estack_frees(0), Estack_avails(0), Estack_avail(nullptr) {}

private:
  Estack get_estack(void);
  void   free_estack(Estack p);

public:
  void   fprint_estack_mem(std::ostream& fp, bool heading);
  void   free_estack_memory(void);
  int    estack_bytes(void);
  Estack update_and_push(void **p, void *new_ptr, Estack stack);
  void   restore_from_stack(Estack stack);
  void   zap_estack(Estack stack);
};

#endif

