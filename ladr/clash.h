#ifndef TP_CLASH_H
#define TP_CLASH_H


#include "mindex.h"
#include "parautil.h"
#include "just.h"







typedef struct clash  * Clash;

struct clash {
  bool   clashable;
  bool       clashed;
  bool       flipped;  /* Is nuc_lit or sat_lit a flipped equality? */
  Literals   nuc_lit;
  Context    nuc_subst;
  Literals   sat_lit;
  Context    sat_subst;
  Mindex     mate_index;
  Mindex_pos mate_pos;
  clash      *next;
};

typedef struct clash  * Clash;


class GlobalClash {

					private:
								unsigned Clash_gets;
								unsigned  Clash_frees;
                    public:			
                                GlobalClash();
                                ~GlobalClash();
                        
								
					friend class ClashContainer;
                    friend class LadrVGlobais;
};


class ClashContainer {
	
						private: 
								 Clash head;
								 Clash get_clash(void);
								 void free_clash(Clash);
								 int lit_position(Topform, Literals);

								 Topform resolve(Clash, Just_type);
								 void clash_recurse(Clash first, Clash p, bool(*sat_test) (Literals), Just_type rule, void (*proc_proc) (Topform));
								 int alit_position(Topform, Literals);
						
						
						public:
									ClashContainer();
									~ClashContainer();
									void fprint_clash_mem(ostream &, bool);
									void p_clash_mem();
									Clash append_clash(Clash);
									void  zap_clash(Clash);
									Literals atom_to_literal(Term);
									Literals apply_lit(Literals, Context);
									void clash(Clash c,   bool (*sat_test) (Literals),   Just_type rule,  void (*proc_proc) (Topform));
	
};

#endif
