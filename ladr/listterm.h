#ifndef TP_LISTTERM_H
#define TP_LISTTERM_H

#include "term.h"
#include "glist.h"

/* Private definitions and types */

#define CONS_SYM  "$cons"
#define NIL_SYM   "$nil"

class ListtermContainer {

            public:
                        ListtermContainer();
                        ~ListtermContainer();
                        Term  get_nil_term();
                        Term  listterm_cons(Term, Term);
                        bool  cons_term(Term);
                        bool  nil_term(Term);
                        bool  proper_listterm(Term);
                        Term  listterm_append(Term, Term);
                        Term  listterm_i(Term, int);
                        int   listterm_length(Term);
                        bool  listterm_member(Term, Term);
                        Plist listterm_to_tlist(Term);
                        void  listterm_zap(Term);
                        Term  rev2(Term, Term);
                        Term  listterm_reverse(Term);
};



#endif
