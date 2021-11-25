#ifndef TP_FORWARD_SUBSUME_H
#define TP_FORWARD_SUBSUME_H

#include "./ladr/subsume.h"
#include "./ladr/clock.h"


#ifndef FEATURES
	#define NUM_INDEXES 10  /* Must be >= 2 */
#endif




class GlobalForwardSubsume {
    
                    
                    private:
                                #ifdef FEATURES
									Di_tree Nonunit_index;
									Lindex Unit_index;
								#else
									Lindex Idx[NUM_INDEXES];
								#endif
                                    
                                    
                    
                    friend class ForwardSubsume;
                    friend class Prover9VGlobais;
                    
};




class ForwardSubsume {

						public:
							static void init_fsub_index(Mindextype,Uniftype,int);
							static void fsub_destroy_index(void);
							static void index_fsub(Topform , Indexop , ClockStructure);
							static Topform forward_subsumption_old(Topform);
							static void unit_deletion_old(Topform);

};


#endif
