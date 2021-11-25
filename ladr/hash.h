#ifndef _HASH_H
#define _HASH_H


#include <iostream>
using namespace std;



class GlobalHash {
                    private:
                        unsigned HashTab_gets, HashTab_frees;    
                        unsigned HashNode_gets, HashNode_frees;
                        
                    public:    
                         GlobalHash();
                         ~GlobalHash();
                        
                        
                        friend class LadrVGlobais;
                        friend class HashtabContainer;
    
    
};



struct hashnode {
                        void      *v;
                        hashnode *next;
};

typedef struct hashnode * Hashnode;


struct hashtab {
  int      size;
  Hashnode *table; //array de pointers para hasnode
};

typedef struct hashtab * Hashtab;


class HashtabContainer {
                private:
                                Hashtab headTab;;
                                
                                
                                Hashtab get_hashtab(void);
                                void free_hashtab(Hashtab);
                                
                                Hashnode get_hashnode(void);
                                void free_hashnode(Hashnode);
                                
                                
                               
                                
                                Hashtab private_hash_init(int);
                                
                                void hash_insert(void *, unsigned, Hashtab);
                                
                                void *hash_lookup(void *v, unsigned hashval, Hashtab h,bool (*id_func) (void *, void *));
                                void hash_delete(void *v, unsigned hashval, Hashtab h,bool (*id_func) (void *, void *));
                                
                                void hash_destroy(Hashtab); 
                                void hash_info(Hashtab);
                                
                                
                                
                                
                            
                public: 
                            HashtabContainer();
                            ~HashtabContainer();
                            
                            void set_head(Hashtab h);
                            Hashtab get_head(void);
                            void hash_init(int);
                            
                            void fprint_hash_mem(ostream &, bool);
                            void p_hash_mem(void);
                            
                            void hash_insert(void *, unsigned);
                            void *hash_lookup(void *v, unsigned hashval, bool (*id_func) (void *, void *));
                            void hash_delete(void *v, unsigned hashval,  bool (*id_func) (void *, void *));
                                
                            void hash_destroy(void); 
                            void hash_info();
                            
                            
                            
                            
};








#endif

