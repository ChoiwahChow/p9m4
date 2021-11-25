#ifndef TP_SUBSUME_H
#define TP_SUBSUME_H

#include "parautil.h"
#include "lindex.h"
#include "features.h"
#include "ladrvglobais.h"




class Subsume{
				
                private:
                        
						static int Nonunit_subsumption_tests;
						static bool subsume_literals(Literals, Context, Topform, Trail *);
						static bool subsume_bt_literals(Literals, Context,Topform, Plist *);
						static void atom_conflict(bool flipped, Topform c, bool sign,  Term a, Lindex idx, void (*empty_proc) (Topform));
                        static bool subsume_literals_anyctx(Literals, Context, Topform , Trail *, int *, Ilist *);
						
				public:
						static int nonunit_subsumption_tests(void);
						static bool subsumes(Topform, Topform);
						static bool subsumes_bt(Topform, Topform);
						static Topform forward_subsume(Topform, Lindex);
						static Plist back_subsume(Topform, Lindex);
						static Topform back_subsume_one(Topform, Lindex);
						
						static void unit_conflict_by_index(Topform c, Lindex idx, void (*empty_proc) (Topform));
						static Topform try_unit_conflict(Topform, Topform);
						static void unit_delete(Topform, Lindex);
						static Plist back_unit_del_by_index(Topform, Lindex);
						static void simplify_literals(Topform);
						static bool eq_removable_literal(Topform, Literals);
						static void simplify_literals2(Topform);

};


#endif
