#ifndef TP_CLAUSEID_H
#define TP_CLAUSEID_H

#include "topform.h"




#define CLAUSE_ID_TAB_SIZE  50000


class GlobalClauseId {
    
                        private:
                                    Plist     Topform_id_tab[CLAUSE_ID_TAB_SIZE];
                                    unsigned  Topform_id_count;
                        public:                                    
                                    GlobalClauseId();
                                    ~GlobalClauseId();
                                    
                                    void destroy_topform_id_tab(void);
                            
                                    friend class ClauseidContainer;
                                    friend class LadrVGlobais;
                                    
};

class ClauseidContainer {
    
                        private:
                                    int next_clause_id(void);
                                  
                                    
                        public:
                                    int clause_ids_assigned(void);
                                    void unassign_clause_id(Topform);
                                    Topform find_clause_by_id(int);
                                    void fprint_clause_id_tab(ostream &);
                                    void p_clause_id_tab();
                                    Plist insert_clause_into_plist(Plist, Topform, bool);
                                    bool clause_plist_member(Plist, Topform ,bool);
                                    void assign_clause_id(Topform);
    
    
};
                                    



#endif
