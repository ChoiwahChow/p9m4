#ifndef TP_INDEX_LITS_H
#define TP_INDEX_LITS_H

#include "./ladr/clock.h"
#include "./ladr/subsume.h"
#include "./ladr/di_tree.h"





class GlobalIndexLits {

                    private:
                            Lindex  Unit_fpa_idx;          /* unit bsub, unit conflict */
                            Lindex  Nonunit_fpa_idx;       /* back unit del */
    
                            Lindex  Unit_discrim_idx;      /* unit fsub, unit del */
                            Di_tree Nonunit_features_idx;  /* nonunit fsub, nonunit bsub */
                        
                            
                    friend class IndexLits;
                    friend class Prover9VGlobais;
};      

class IndexLits {

                private:
            
                
        

                public:
                    
                    void init_literals_index(void);
                    void destroy_literals_index(void);
                    void index_literals(Topform, Indexop , ClockStructure clock, bool);
                    void index_denial(Topform, Indexop , ClockStructure clock);
                    void unit_conflict(Topform c, void (*empty_proc) (Topform));
                    void unit_deletion(Topform);
                    Plist back_unit_deletable(Topform);
                    Topform forward_subsumption(Topform);
                    Plist back_subsumption(Topform);
                    void lits_idx_report(void);
    
    
};

#endif
