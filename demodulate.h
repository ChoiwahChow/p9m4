#ifndef TP_DEMODULATE_H
#define TP_DEMODULATE_H


#include "./ladr/clock.h"
#include "./ladr/mindex.h"
#include "./ladr/topform.h"


class GlobalDemodulate {

                    private:
                            Mindex Demod_idx;
							Mindex Back_demod_idx;
					
                            
    
    
                        friend class Prover9VGlobais;
                        friend class Demodulate;
};

class Demodulate {
					private:
							
					public:
					
							void init_demodulator_index(Mindextype, Uniftype, int);
							void init_back_demod_index(Mindextype, Uniftype, int);
							void index_demodulator(Topform, int, Indexop , ClockStructure);
							void index_back_demod(Topform, Indexop, ClockStructure, bool);
							void destroy_demodulation_index(void);
							void destroy_back_demod_index(void);
							static void demodulate_clause(Topform, int, int,bool,bool);
							Plist back_demodulatable(Topform, int, bool);
							void back_demod_idx_report(void);
};

#endif
