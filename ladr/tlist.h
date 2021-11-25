#ifndef TP_TLIST_H
#define TP_TLIST_H

#include "term.h"
#include "glist.h"

class Tlist {

       private:
            
            static void zap_tlist(Plist);
            static Plist tlist_remove(Term , Plist);
            static Plist tlist_union(Plist, Plist);
        public: 
            
            static Ilist constants_in_term(Term, Ilist);
            static Plist tlist_copy(Plist);

            friend class FormulaContainer;
};


#endif
