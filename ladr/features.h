#ifndef TP_FEATURES_H
#define TP_FEATURES_H


#include "literals.h"



class GlobalFeatures {

                            private:
                                        Ilist Feature_symbols;  /* list of featured symbols (symnums)*/

                                        /* The following are work arrays, indexed by symnum, used for calculating
                                        the features of a clause.  They are allocated by init_features() and
                                        left in place throughout the process.
                                        */
                                        int Work_size;         /* size of following arrays */
                                        int *Pos_occurrences;
                                        int *Neg_occurrences;
                                        int *Pos_maxdepth;
                                        int *Neg_maxdepth;
                                        
                                        void fill_in_arrays(Term , bool, int);
                                        
                                        
                            public:
                                        GlobalFeatures();
                                        ~GlobalFeatures();
                                
                                        void Free_Mem(void);
                                        
                                        void init_features(Ilist, Ilist);

                                        Ilist features(Literals);

                                        int feature_length(void);

                                        bool features_less_or_equal(Ilist, Ilist);

                                        void p_features(Ilist);
                                        
                                        friend class LadrVGlobais;

};





#endif  
