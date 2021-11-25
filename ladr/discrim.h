#ifndef TP_DISCRIM_H
#define TP_DISCRIM_H

#include "unify.h"


enum class DiscriminationTreeNode   { 
                                        DVARIABLE, 
                                        DRIGID, 
                                        AC_ARG_TYPE, 
                                        AC_NV_ARG_TYPE
};


struct discrim {       /* node in a discrimination tree */
                discrim   *next;      /* sibling */
                union {
                        discrim *kids;      /* for internal nodes */
                        Plist data;        /* for leaves */
                     } u;
                //short symbol;        /* variable number or symbol number */
                int symbol;
                DiscriminationTreeNode type;           /* term type and for ac indexing type */
};


typedef struct discrim * Discrim;




struct discrim_pos {  /* to save position in set of answers */
  void    *query;
  Context subst;        /* substitution */
  Plist   data;         /* identical terms from leaf of discrim tree */
  void    *backtrack;   /* data for backtracking */
};

typedef struct discrim_pos * Discrim_pos;

/* type of discrimination tree node */



#define DVAR(d)  ((d)->type == DiscriminationTreeNode::DVARIABLE)
#define PTRS_DISCRIM PTRS(sizeof(struct discrim))
#define PTRS_DISCRIM_POS PTRS(sizeof(struct discrim_pos))

class GlobalDiscrim {
                    
                    private:
                            unsigned Discrim_gets, Discrim_frees;
                            unsigned Discrim_pos_gets, Discrim_pos_frees;
                            
                            
                    public:
                            GlobalDiscrim();
                            ~GlobalDiscrim();
    

                            friend class DiscrimContainer;
                            friend class LadrVglobais;
};

class DiscrimContainer {

                        private:
                                void zap_discrim_tree(Discrim, int);
    
                        public:
                                DiscrimContainer();
                                ~DiscrimContainer();

                                Discrim get_discrim(void);
                                void free_discrim(Discrim p);
                                Discrim_pos get_discrim_pos(void);
                                void free_discrim_pos(Discrim_pos);
                                void fprint_discrim_mem(ostream &,const bool) const;
                                void p_discrim_mem(void) const;
                                Discrim discrim_init(void);
                                void discrim_dealloc(Discrim);
                                void destroy_discrim_tree(Discrim);
                                bool discrim_empty(Discrim);


};


#endif
