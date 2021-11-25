#ifndef TP_HINTS_H
#define TP_HINTS_H

#include "subsume.h"
#include "clist.h"
#include "backdemod.h"
#include "resolve.h"





class Hints {
	
	
				private:
						
                        static Lindex Hints_idx;       /* FPA index for hints */
						static Clist Redundant_hints;  /* list of hints not indexed */
						static Mindex Back_demod_idx;        /* to index hints for back demodulation */
						static int Bsub_wt_attr;
						static bool Back_demod_hints;
						static bool Collect_labels;
						
						static int Hint_id_count;
						static int Active_hints_count;
						static int Redundant_hints_count;
						
                        static  void (*Demod_proc) (Topform, int, int, bool, bool);
                        
                        
                        static Topform find_equivalent_hint(Topform, Lindex);
						static Topform find_matching_hint(Topform, Lindex);
			
				public:
				
						static void init_hints(Uniftype utype,int bsub_wt_attr,bool collect_labels,bool back_demod_hints,void (*demod_proc) (Topform, int, int, bool, bool));
						static void done_with_hints(void);
						static int redundant_hints(void);
						static void index_hint(Topform, bool);
						static void unindex_hint(Topform);
						static void adjust_weight_with_hints(Topform,bool degrade,bool breadth_first_hints);
						static void keep_hint_matcher(Topform);
						static void back_demod_hints(Topform, int, bool);
                        // BV(2017-nov-12)
                        static void flag_indexed_hints(void);
                        static void Free_Mem(void);
};


#endif
