#ifndef TP_SEMANTICS_H
#define TP_SEMANTICS_H

#include "./ladr/interp.h"
#include "./ladr/ioutil.h"


enum class False_Type{ 
						FALSE_IN_ALL,
						FALSE_IN_SOME
};



class GlobalSemantics {
                        
                        private:
                            
                            		Plist Compiled_interps;
                                    int Eval_limit;
                                    bool False_in_all;
                                    myClock Eval_clock;
                            
                        friend class Semantics;
                        friend class Prover9VGlobais;
    
};

class Semantics {

					private:
					
							
							static bool eval_limit_ok(Interp, int);


					public:
							static void init_semantics(Plist, ClockStructure,  string, int, int);
							static void eval_in_interps(Topform);
							static void set_semantics(Topform);
							static void update_semantics_new_constant(Topform);

};


#endif
