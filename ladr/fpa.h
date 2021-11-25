#ifndef TP_FPA_H
#define TP_FPA_H

#include "unify.h"
#include "index.h"
#include "fpalist.h"



#define BUMP_NEXT_CALLS {LADR_GLOBAL_FPA_INDEX.Next_calls++; if (LADR_GLOBAL_FPA_INDEX.Next_calls == 0) LADR_GLOBAL_FPA_INDEX.Next_calls_overflows++;}

class GlobalFpaIndex {
                    private:
                            unsigned Fpa_id_count;
                            unsigned Next_calls;
                            unsigned Next_calls_overflows;
                            unsigned Fpa_trie_gets, Fpa_trie_frees;
                            unsigned Fpa_state_gets, Fpa_state_frees;
                            unsigned Fpa_index_gets, Fpa_index_frees;
                            
                    public:       
                            GlobalFpaIndex();
                            ~GlobalFpaIndex();
                            friend class LadrVGlobais;
                            friend class FpaIndexContainer;
};




struct fpa_trie {
  fpa_trie *parent, *next, *kids;
  int        label;
  Fpa_list   terms;
    #ifdef FPA_DEBUG
        Ilist      path;
    #endif
};

typedef struct fpa_trie * Fpa_trie;


struct fpa_index {
  Fpa_trie   root;
  int        depth;
  fpa_index  *next;
};

typedef struct fpa_index * Fpa_index;



enum class Fpa_state_type   {
                                LEAF,
                                UNION,
                                INTERSECT
};  /* types of fpa_state (node in FPA tree) */

struct fpa_state {
  Fpa_state_type    type;
  fpa_state        *left, *right;
  Term             left_term, right_term;
  struct fposition fpos;
#ifdef FPA_DEBUG
  Ilist            path;
#endif
};

typedef struct fpa_state * Fpa_state;

/* A path is a sequence of integers.  We have to do some operations
   to the end of a path, and an Ilist is singly-linked; but we can
   get away with just keeping a pointer to the end of the list.
 */

struct path {
  Ilist first;
  Ilist last;
};





class FpaIndexContainer {
                                        private:
                                                    Fpa_index head;
                                                    Fpa_trie get_fpa_trie(void);
                                                    void free_fpa_trie(Fpa_trie);
                                                    Fpa_state get_fpa_state(void);
                                                    void free_fpa_state(Fpa_state);
                                                    Fpa_index get_fpa_index(void);
                                                    void free_fpa_index(Fpa_index p);
                                                    void fprint_fpa_mem(ostream &, bool);
                                                    void fprint_path(ostream &, Ilist);
                                                    void fprint_fpa_trie(ostream &, Fpa_trie, int);
                                                    void flag_fpa_leaves(ostream &, Fpa_trie, int);
                                                    
                                        public:
                                                    FpaIndexContainer();
                                                    ~FpaIndexContainer();
                                                    void p_fpa_mem();
                                                    void flag_fpa_clauses(ostream &, Fpa_index);
                                                    
                                                    void p_path(Ilist);
                                                    void p_fpa_index(Fpa_index);
                                                    Fpa_trie fpa_trie_member_insert(Fpa_trie, Ilist);
                                                    Fpa_trie fpa_trie_member(Fpa_trie, Ilist);
                                                    void fpa_trie_possible_delete(Fpa_trie);
                                                    void path_insert(Term, Ilist, Fpa_trie);
                                                    void path_delete(Term, Ilist, Fpa_trie);
                                                    Ilist path_push(struct path *, int);
                                                    void path_restore(struct path *, Ilist);
                                                    void fpa_paths(Term, Term , struct path *, int,  Indexop , Fpa_trie);
                                                    Fpa_index fpa_init_index(int);
                                                    void fpa_update(Term, Fpa_index, Indexop);
                                                    Fpa_state query_leaf_full(Ilist, Fpa_trie);
                                                    Fpa_state query_leaf(Ilist, Fpa_trie);
                                                    Fpa_state query_intersect(Fpa_state, Fpa_state);
                                                    Fpa_state query_special(Fpa_trie);
                                                    void zap_fpa_state(Fpa_state);
                                                    Fpa_state union_commuted(Fpa_state, Term, Context, Querytype , struct
                                                    path *, int , Fpa_trie);
                                                    bool var_in_context(Term, Context);
                                                    bool all_args_vars_in_context(Term, Context);
                                                    Fpa_state query_union(Fpa_state, Fpa_state);
                                                    Fpa_state build_query(Term , Context , Querytype,struct path *, int, Fpa_trie); 
                                                    
                                                    
                                                    void fprint_fpa_state(ostream &, Fpa_state , int);
                                                    void p_fpa_state(Fpa_state);
                                                    void p_fpa_query(Term, Querytype, Fpa_index);
                                                    Term next_term(Fpa_state, FPA_ID_TYPE);
                                                    Term fpa_next_answer(Fpa_state);
                                                    Term fpa_first_answer(Term, Context, Querytype, Fpa_index, Fpa_state *);
                                                    void fpa_cancel(Fpa_state);
                                                    void zap_fpa_trie(Fpa_trie);
                                                    void zap_fpa_index(Fpa_index);
                                                    bool fpa_empty(Fpa_index);
                                                    void fpa_density(Fpa_trie);
                                                    void p_fpa_density(Fpa_index);
                                                    unsigned mega_next_calls(void);
                                                    void fprint_fpa_index(ostream &, Fpa_index);
                                                    
                                                    
                                                    friend class Utilities;
                                                    
                                                    
                                                    
                                                    

};







#endif
