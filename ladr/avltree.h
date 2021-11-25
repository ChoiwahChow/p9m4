
#ifndef TP_AVLTREE_H
#define TP_AVLTREE_H

#include "memory.h"
#include "order.h"
#include <iostream>

#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ( (a) < (b) ? (a) :(b))

using namespace std;


#define MAX_NAME      51

/* parse/print properties of symbols */


#define MIN_PRECEDENCE      1
#define MAX_PRECEDENCE    999
             


struct avl_node {
  void *item;             /* data (including key) */
  int size;               /* number of nodes */
  int height;             /* leaf has height 1 */
  avl_node *left, *right;   /* subtrees */
};

typedef struct avl_node * Avl_node;



class GlobalAvltree {
    private:
            
            bool Debug;
            unsigned Avl_node_gets;
            unsigned Avl_node_frees;
            void fprint_avltree_mem(ostream &, bool);
            void p_avltree_mem(void);
            
    public:
            GlobalAvltree();
            ~GlobalAvltree();
        
        
            
            friend class Utilities;
            friend class AvltreeContainer;
            friend class LadrVGlobais;
};





class AvltreeContainer {

                    private: 
                        
                        Avl_node root;
                        Avl_node get_avl_node(void);/* maximum number of chars in string part of symbol (includes '\0') */



                        void free_avl_node(Avl_node);
                        void avl_zap(Avl_node);
                        int avl_height(Avl_node);
                        int avl_size(Avl_node);
                        void set_height_and_size(Avl_node);
                        Avl_node rotate_left(Avl_node);
                        Avl_node rotate_right(Avl_node);
                        bool balance_ok(Avl_node);
                        Avl_node avl_fix(Avl_node);
                        Avl_node avl_insert(Avl_node p, void *item,  OrderType (*compare) (void *, void *));
                        
                        
                        Avl_node remove_and_return_largest(Avl_node, Avl_node *);
                        Avl_node remove_and_return_smallest(Avl_node p, Avl_node* removed);
                        Avl_node avl_delete(Avl_node p, void *item,OrderType (*compare) (void*, void *));
                        Avl_node avl_lookup(Avl_node p, void *item, OrderType (*compare) (void*, void *));
                        void *avl_find(Avl_node p, void *item, OrderType (*compare) (void *, void *));
                        void *avl_smallest(Avl_node);
                        void *avl_largest(Avl_node);
                        int avl_place(Avl_node p, void *item,   OrderType (*compare) (void *, void *));
                        double avl_position(Avl_node p, void *item,  OrderType (*compare) (void *, void *));
                        void *avl_nth_item(Avl_node p, int n);
                        void * avl_item_at_position(Avl_node p, double pos);
                        void avl_check(Avl_node p, OrderType (*compare) (void *, void *));
                        void p_avl(Avl_node, int);
                      
 
                    
                    public:
                        
                            AvltreeContainer();
                            ~AvltreeContainer();
                            
                            Avl_node avl_insert(void *item,  OrderType (*compare) (void *, void *));
                            Avl_node avl_delete(void *item,OrderType (*compare) (void *, void *));
                            void *avl_find(void *item, OrderType (*compare) (void *, void *));
                            void * avl_smallest(void);
                            void * avl_largest(void);
                            int avl_place(void* item, OrderType (*compare)(void *, void *));
                            double avl_position(void *item,  OrderType (*compare) (void *, void *));
                            void * avl_nth_item(int);
                            void * avl_item_at_position(double);
                            void avl_check(OrderType (*compare) (void *, void *));
                            void p_avl(int);
                            void avl_zap(void);
                            
                            
                            friend class GivSelect;
                            friend class GlobalGivSelect;
    

};



#endif
