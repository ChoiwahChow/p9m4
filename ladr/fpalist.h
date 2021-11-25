#ifndef TP_FPALIST_H
#define TP_FPALIST_H
#include <iostream>
#include "term.h"


#define F_INITIAL_SIZE 4  /* initial size of chunks  (they double as needed) */
#define F_MAX_SIZE 512    /* maximum size of chunks */

#define FPA_ID_TYPE unsigned
#define FPA_ID_MAX UINT_MAX
#define FPA_ID(t) (((Term) t)->u.id)

#define FLT(x,y) (FPA_ID(x) <  FPA_ID(y))
#define FGT(x,y) (FPA_ID(x) >  FPA_ID(y))
#define FLE(x,y) (FPA_ID(x) <= FPA_ID(y))
#define FGE(x,y) (FPA_ID(x) >= FPA_ID(y))

#define FTERM(p) ((p).f == NULL ? NULL : (p).f->d[(p).i])


#define FLAST(f) (f)->d[(f)->size - 1]
#define FFIRST(f) (f)->d[(f)->size - (f)->n]




struct fpa_list; //forward declaration
struct fpa_chunk; //forward declaration


typedef struct fpa_chunk *Fpa_chunk;
typedef struct fpa_list  *Fpa_list;


struct fpa_chunk {
  int size;         /* size of array */
  Term *d;          /* array for chunk */
  int n;            /* current number of items in chunk (right justified) */
  Fpa_list head;    /* beginning of list to which this chunk belongs */
  Fpa_chunk next;   /* list of chunks is singly-linked */
};


struct fpa_list {
  Fpa_chunk chunks;
  int num_chunks;
  int chunksize;
  int num_terms;
};

/* to maintain a position in an FPA list while traversing for set operations */

struct fposition {
  Fpa_chunk f;
  int i;
};




class GlobalFpaList {
                private:
                            unsigned Fpa_chunk_gets, Fpa_chunk_frees;
                            unsigned Fpa_list_gets, Fpa_list_frees;
                            unsigned Chunk_mem;  /* keep track of Memory:: (pointers) for chunks */
                
                public:     GlobalFpaList();
                            ~GlobalFpaList();
                            

                            friend class LadrVGlobais;
                            friend class FpalistContainer;

};



class FpalistContainer {
                                private: 
                                        Fpa_list head;
                                            
                                        Fpa_chunk get_fpa_chunk(int);
                                        void free_fpa_chunk(Fpa_chunk);
                                        void free_fpa_list(Fpa_list);
                                        Fpa_chunk double_chunksize(Fpa_chunk);
                                        Fpa_chunk flist_insert(Fpa_chunk, Term , Fpa_list); 
                                        Fpa_chunk consolidate(Fpa_chunk f, Fpa_list);
                                        Fpa_chunk flist_delete(Fpa_chunk, Term, Fpa_list);

                                public:
                                        FpalistContainer();
                                        ~FpalistContainer();
                                        Fpa_list get_fpa_list(void);
                                        void fprint_fpalist_mem(ostream &, bool);
                                        void p_fpalist_mem(void);
                                        void fpalist_insert(Fpa_list, Term);
                                        void fpalist_delete(Fpa_list, Term);
                                        fposition first_fpos(Fpa_list);
                                        struct fposition next_fpos(struct fposition);
                                        void zap_fpa_chunks(Fpa_chunk);
                                        void zap_fpalist(Fpa_list);
                                        bool fpalist_empty(Fpa_list);
                                        void p_fpa_list(Fpa_chunk);
                                        void flag_fpa_leaf_clauses(Fpa_chunk);
};




#endif
