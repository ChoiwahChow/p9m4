#ifndef _GLIST_H
#define _GLIST_H

#include "order.h"

#include <iostream>

#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ( (a) < (b) ? (a) :(b))


using namespace std;

typedef struct plist * Plist;

//Pointers List node-------------------------------------------------------
struct plist {
  void *v;
  Plist next;
};



//-------------------------------------------------------------------------



//Integer List node -------------------------------------------------------
struct ilist {
  int i;
  ilist *next;
};
typedef struct ilist *Ilist;
//-------------------------------------------------------------------------



//Duple int list node------------------------------------------------------
struct i2list {
  int i;
  int j;
  i2list *next;
};

typedef struct i2list * I2list;
//-------------------------------------------------------------------------


//Tripple int List node ---------------------------------------------------
struct i3list {
  int i;
  int j;
  int k;
  i3list *next;
};
typedef struct i3list * I3list;
//--------------------------------------------------------------------------



class GlobalGlist {
					 private:
								unsigned Ilist_gets, Ilist_frees;
								unsigned Plist_gets, Plist_frees;
								unsigned I2list_gets, I2list_frees;
								unsigned I3list_gets, I3list_frees;
								
                    public:     GlobalGlist();
                                ~GlobalGlist();
                                void fprint_glist_mem(ostream &, bool);	
								void p_glist_mem(void);	
                            
                                
                                friend class IlistContainer;
                                friend class I2listContainer;
                                friend class I3listContainer;
                                friend class PlistContainer;
                                

};


//Object to contain a list of integers
class IlistContainer {
						private:
							Ilist head; //the head element of the list
							 
							 bool ilist_member(Ilist, int);
                             Ilist ilist_prepend(Ilist, int);
                             Ilist ilist_subtract(Ilist , Ilist);
                             Ilist ilist_removeall(Ilist , int);
                             Ilist ilist_intersect(Ilist a, Ilist b); //constroi a intersecção de duas Ilists
                             Ilist ilist_union(Ilist a, Ilist b);
                             Ilist  _ilist_set(Ilist);
                             bool  ilist_is_set(Ilist);
                             bool  ilist_subset(Ilist, Ilist); 
                             void  f_printf_ilist(ostream &, Ilist);
                             void  p_ilist(Ilist);
                             Ilist ilist_copy(Ilist p);
                             Ilist ilist_remove_last(Ilist p);
                             int ilist_occurrences(Ilist, int);
                             Ilist ilist_insert_up(Ilist, int);
                             int position_in_ilist(int, Ilist);
                             
                             Ilist rev_app_ilist(Ilist, Ilist); 
                             Ilist reverse_ilist(Ilist);
                             
                             Ilist get_ilist(void); //cria um novo nó ilist
							 void free_ilist(Ilist); //liberta um novo nó da lista
                             void zap_ilist(Ilist);
                             Ilist ilist_cat(Ilist, Ilist);
                             
                             
                             
        
                           
							
						public:
								IlistContainer(); //construtor
								~IlistContainer(); //destrutor
                                IlistContainer(Ilist); //construtor por cópia
								
                                Ilist get_head(); //devolv o head
                                void set_head(Ilist);
								
               
                                
                                Ilist ilist_append(int); //insere um novo inteiro na lista
								Ilist ilist_pop(void);
                                Ilist ilist_prepend(int);


                                int ilist_count(void);
                                
                                Ilist ilist_cat(IlistContainer &);  //concatena nesta lista uma lista passada, mas não cria uma cópia
                                Ilist ilist_cat2(IlistContainer &); //concatena nesta lista uma lista passada como referência, mas cria uma cópia
                                
                                Ilist copy_ilist(void);
                                Ilist ilist_last(void);                               
                                Ilist ilist_subtract(IlistContainer &);
                                bool  ilist_member(int);
                               
                                Ilist ilist_removeall(int); //remove todas as ocorrências de i
                                void  ilist_intersect(IlistContainer &, IlistContainer &); //constroi a intersecção de duas Ilists
                                void  ilist_union(IlistContainer &, IlistContainer &);
                                Ilist ilist_set(Ilist);
                                bool  ilist_is_set(void);
                                bool  ilist_subset(IlistContainer &);
                                void  f_printf_ilist(ostream &);
                                void  p_ilist(void);
                                
                                Ilist ilist_copy(void);
                                Ilist ilist_remove_last(void);
                                
                                int ilist_occurrences(int i);
                                Ilist ilist_insert_up(int);
                                int position_in_ilist(int);

                                
                              
                                void zap_ilist(void);
                                
                                
                                Ilist multiset_to_set(I2list);
                                Ilist reverse_ilist(void);
                            
                                
                                
                                friend class I2listContainer;
                                friend class FormulaContainer;
                                friend class Cnf;
                                friend class Random;
                                friend class Unfold;
                                friend class Paramodulation;
                                friend class SymbolContainer;
                                friend class Utilities;
                                friend class FpaIndexContainer;
};



class I2listContainer {

                            private: 
                                        I2list head;
                                        I2list get_i2list(void);
                                        void free_i2list(I2list);
                                        
                                        I2list i2list_append(I2list, int , int );
                                        I2list i2list_prepend(I2list , int , int );
                                        I2list i2list_removeall(I2list, int);
                                        I2list i2list_member(I2list, int);
                                        int    i2list_count(I2list);
                                        I2list alist_insert(I2list, int , int);
                                        int assoc(I2list , int );
                                        bool i2list_multimember(I2list , int , int );
                                        void f_printf_i2list(ostream &, I2list );
                                        bool i2list_multisubset(I2list, I2list);
                                        
                            
                            public:
                                
                                        I2listContainer();
                                        ~I2listContainer();
                                        void zap_i2list(I2list);
                                        void zap_i2list(void);
                                        
                                        
                                        I2list i2list_append(int, int);
                                        I2list i2list_prepend(int,int);
                                        I2list i2list_removeall(int);
                                        I2list i2list_member(int);
                                        int    i2list_count(void);
                                        I2list alist_insert(int , int ) ;
                                        int assoc(int);
                                        bool i2list_multimember(int, int);
                                        
                                        
                                        bool i2list_multisubset(I2listContainer &);
                                        I2list multiset_add_n(I2list a, int i, int n);
                                        I2list multiset_add_n(int, int); 
                                        
                                        I2list multiset_add(I2list, int );
                                        I2list multiset_add(int); 
                                        I2list multiset_union(I2list, I2list);
                                        I2list multiset_union(I2list);
                                        
                                        
                                        int multiset_occurrences (I2list m, int);
                                        int multiset_occurrences(int);
                                        
                                        void  f_printf_i2list(ostream &);
                                        void  p_i2list(void);
                                        
                                        I2list get_head(void);
                                        void set_head(I2list);
                                        
                                        
                                        friend class TermContainer;
                                    
                                        
                                    
};


class I3listContainer {
    
                        private: 
                                I3list head;
                                
                                  I3list get_i3list(void);
                                  void free_i3list(I3list);
                                  
                                  bool i3list_member(I3list, int, int, int);
                                  I3list i3list_append(I3list lst, int i, int j, int k);  
                                  I3list i3list_prepend(I3list, int, int, int);
                                  I3list reverse_i3list(I3list);
                                  I3list rev_app_i3list(I3list, I3list);
                                  int i3list_count(I3list);
                                  I3list copy_i3list(I3list);
                              
                                  I3list alist2_insert(I3list, int, int, int);
                                  int assoc2a(I3list , int);
                                  int assoc2b(I3list ,int);
                                  I3list alist2_remove(I3list, int);
                                  
                                 
                                  void zap_i3list(I3list);
                        
                        
                        public:
                                 I3listContainer();
                                 ~I3listContainer();
                            
                                I3list get_head(void);
                                void set_head(I3list);
                                bool i3list_member(int, int, int);
                                
                                I3list i3list_append(int, int, int);
                                I3list i3list_prepend(int, int, int);
                                I3list reverse_i3list(void);
                                int i3list_count(void);
                                I3list copy_i3list(void);
                                I3list alist2_insert(int,int,int);
                              
                                int assoc2a(int);
                                int assoc2b(int);
                                I3list alist2_remove(int);
                        
                                
                                void zap_i3list();
};


class PlistContainer {
                        private:
                                    Plist head;
                                    
                                   
                                    Plist plist_append(Plist, void *);
                                    Plist plist_cat2(Plist, Plist);
                                    
                                   
                                  
                                   
                                 
                                    Plist plist_prepend(Plist lst, void* v);
                                    Plist plist_pop(Plist);
                                    int plist_count(Plist);
                                    Plist copy_plist(Plist);
                                    Plist plist_cat(Plist, Plist);
                                    Plist rev_app_plist(Plist, Plist);
                                    Plist reverse_plist(Plist);
                                    bool plist_member(Plist, void *);
                                    
                                    Plist plist_subtract(Plist, Plist);
                                    bool plist_subset(Plist , Plist );
                                    Plist plist_remove(Plist , void *);
                                    
                                    
                                    Plist plist_remove_string(Plist, char *);
                                    Plist sort_plist(Plist objects, OrderType (void *, void *));
                                    Plist plist_remove_last(Plist);
                                    int   position_of_string_in_plist(char *, Plist );
                                    bool  string_member_plist(char *, Plist);
                                    int   longest_string_in_plist(Plist);
                                    void  *ith_in_plist(Plist , int );
                                    void  *plist_last(Plist);
                                    
                                    void  free_plist(Plist);
                                    Plist get_plist(void);
                                    void  zap_plist(Plist);
                                   
                                  
                                   
                        
                        
                        public:
                                    PlistContainer();
                                    ~PlistContainer();
                                    
                                   
                                    
                                    Plist get_head(void);
                                    void  set_head(Plist);
                                    
                                    Plist plist_append(void *);
                                    Plist plist_prepend(void *);
                                    Plist plist_pop(void);
                                    int   plist_count(void);
                                    Plist copy_plist(void);
                                    
                                    Plist plist_cat(PlistContainer &);
                                    Plist plist_cat(Plist);
                                    
                                  
                                    Plist plist_cat2(PlistContainer &);
                                    Plist plist_cat2(Plist);
                                    Plist reverse_plist(void);
                                    bool  plist_member(void* v);
                                    
                                    Plist plist_subtract(Plist);
                                    bool  plist_subset(Plist );
                                    Plist plist_remove(void *);
                                    
                                    Plist plist_remove_string(char *);
                                    Plist sort_plist(OrderType (*comp_proc)(void *, void*));
                                    Plist plist_remove_last(void);
                                    int   position_of_string_in_plist(char *s);
                                    bool  string_member_plist(char *);
                                    int   longest_string_in_plist(void);
                                    void  *ith_in_plist(int );
                                    void  *plist_last(void);
                                    void  zap_plist(void);
                                    void plist_string_print(void);
                                    bool string_member_plist(string);
                                    
                                    friend class DiscrimBContainer; //esta necessita de aceder à parte privada de plist
                                    friend class DiscrimWContainer;
                                    friend class MindexContainer;
                                    friend class TlistContainer;
                                    friend class FormulaContainer;
                                    friend class ClauseidContainer;
                                    friend class DefinitionsContainer;
                                    friend class Di_treeContainer;
                                    friend class ClauseMisc;
                                    friend class Tlist;
                                    friend class Ioutil;
                                    friend class Ivy;
                                    friend class TermContainer;
                                    friend class Search;
                                    
                     
};


#endif
