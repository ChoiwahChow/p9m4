#include "ladrvglobais.h"
#include "avltree.h"
#include "fatal.h"


#include <math.h>
#include <iostream>
#include <iomanip>
#include <climits>




using namespace std;





GlobalAvltree::GlobalAvltree(){
    Debug=false;
    Avl_node_gets=Avl_node_frees=0;
}

GlobalAvltree::~GlobalAvltree() {

    
}



void GlobalAvltree::fprint_avltree_mem(ostream &fp, bool heading){
  int n;
  if (heading)
    fp<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct avl_node);
  fp<<"avl_node      ("<< n <<")        ";
  fp<<setw(11)<<Avl_node_gets;
  fp<<setw(11)<<Avl_node_frees;
  fp<<setw(11)<<Avl_node_gets-Avl_node_frees;
  fp<<setw(9)<<((Avl_node_gets-Avl_node_frees) *n /1024)<<"K"<<endl;
}

void GlobalAvltree::p_avltree_mem(void){
    fprint_avltree_mem(cout, true);
}


AvltreeContainer::AvltreeContainer() {
    root=NULL;
}

AvltreeContainer::~AvltreeContainer() {
    root=NULL;
}


Avl_node AvltreeContainer::get_avl_node(void) {
  Avl_node p = (Avl_node) Memory::memNew(sizeof(struct avl_node));
  p->left=NULL;
  p->right=NULL;
  LADR_GLOBAL_AVL_TREE.Avl_node_gets++;
  return(p);
}

void AvltreeContainer::free_avl_node(Avl_node p) {
   LADR_GLOBAL_AVL_TREE.Avl_node_frees++;
   Memory::memFree(p, sizeof(struct avl_node));
}

void AvltreeContainer::avl_zap(Avl_node p) {
  if (p != NULL) {
    avl_zap(p->left);
    avl_zap(p->right);
    free_avl_node(p);
    p=NULL;
  }
}

void AvltreeContainer::avl_zap(void) {
    avl_zap(root);
    root=NULL;
}

int AvltreeContainer::avl_height(Avl_node p) {
  return (p == NULL ? 0 : p->height);
}


int AvltreeContainer::avl_size(Avl_node p) {
  return (p == NULL ? 0 : p->size);
}


void AvltreeContainer::set_height_and_size(Avl_node p) {
  p->height = IMAX(avl_height(p->left), avl_height(p->right)) + 1;
  p->size = (avl_size(p->left) + avl_size(p->right)) + 1;
}



Avl_node AvltreeContainer::rotate_left(Avl_node p) {
  Avl_node b = p;
  Avl_node c = p->right->left;
  Avl_node d = p->right;
  if (LADR_GLOBAL_AVL_TREE.Debug) cout << "rotate left"<<endl;
  b->right = c; set_height_and_size(b);
  d->left  = b; set_height_and_size(d);
  return d;
}



Avl_node AvltreeContainer::rotate_right(Avl_node p) {
  Avl_node b = p->left;
  Avl_node c = p->left->right;
  Avl_node d = p;
  if (LADR_GLOBAL_AVL_TREE.Debug) cout << "rotate right"<<endl;
  d->left  = c; set_height_and_size(d);
  b->right = d; set_height_and_size(b);
  return b;
}


bool AvltreeContainer::balance_ok(Avl_node p) {
    return (abs(avl_height(p->left) - avl_height(p->right)) <= 1);
}





Avl_node AvltreeContainer::avl_fix(Avl_node p) {
  if (balance_ok(p)) {
    set_height_and_size(p);
    return p;
  }
  else {
    if (avl_height(p->left) < avl_height(p->right)) {
      if (avl_height(p->right->left) <= avl_height(p->right->right)) return rotate_left(p);
      else {
            p->right = rotate_right(p->right);
            return rotate_left(p);
      }
    }
    else {
            if (avl_height(p->left->right) <= avl_height(p->left->left)) 	return rotate_right(p);
            else {
                    p->left = rotate_left(p->left);
                    return rotate_right(p);
            }
    }
  }
}

Avl_node AvltreeContainer::avl_insert(Avl_node p, void *item,  OrderType (*compare) (void *, void *))  {
  if (p == NULL) {
    Avl_node n = get_avl_node();
    n->item = item;
    n->size = 1;
    n->height = 1;
    n->left = NULL;
    n->right = NULL;
    return n;
  }
  else {
            OrderType relation = (*compare)(item, p->item);
    switch (relation) {
            case OrderType::LESS_THAN:    p->left  = avl_insert(p->left,  item, compare); break;
        
            case OrderType::GREATER_THAN: p->right = avl_insert(p->right, item, compare); break;
       
            case OrderType::SAME_AS: fatal::fatal_error("avl_insert, item already there"); break;
        
            default:      fatal::fatal_error("avl_insert, not comparable"); break;
    }

    p = avl_fix(p);  /* rebalance if necessary; set height, size */
    return p;
  }
} 

Avl_node AvltreeContainer::avl_insert(void *item,  OrderType (*compare) (void *, void *)) {
    root=avl_insert(root,item, compare);
    return root;
}

Avl_node AvltreeContainer::remove_and_return_largest(Avl_node p,Avl_node *removed) {
  if (p->right == NULL) {
            *removed = p;
            return p->left;
  }
  else {
            p->right = remove_and_return_largest(p->right, removed);
            p = avl_fix(p);  /* rebalance if necessary; set height, size */
            return p;
  }
}


Avl_node AvltreeContainer::remove_and_return_smallest(Avl_node p, Avl_node *removed) {
  if (p->left == NULL) {
        *removed = p;
        return p->right;
  }
  else {
        p->left = remove_and_return_smallest(p->left, removed);
        p = avl_fix(p);  /* rebalance if necessary; set height, size */
        return p;
  }
}


Avl_node AvltreeContainer::avl_delete(Avl_node p, void *item,OrderType (*compare) (void *, void *)) {
  if (p == NULL)  fatal::fatal_error("avl_delete, item not found");
  else {
        OrderType relation = (*compare)(item, p->item);
        if (relation == OrderType::LESS_THAN) p->left  = avl_delete(p->left,item, compare);
        else if (relation == OrderType::GREATER_THAN) p->right =avl_delete(p->right, item, compare);
        else if (relation != OrderType::SAME_AS) fatal::fatal_error("avl_find:not comparable");
        else {
                Avl_node left  = p->left;
                Avl_node right = p->right;
                free_avl_node(p);
                p = NULL;
                if (left == NULL && right == NULL) return NULL;
                else if (avl_height(left) < avl_height(right)) right = remove_and_return_smallest(right, &p);
                else left = remove_and_return_largest(left, &p);
                p->left = left;
                p->right = right;
        }
        p = avl_fix(p);
  }
  return p;
}


Avl_node AvltreeContainer::avl_delete(void *item , OrderType (*compare) (void *, void *)) {
    root=avl_delete(root,item,compare);
    return root;
}


Avl_node AvltreeContainer::avl_lookup(Avl_node p, void *item, OrderType (*compare) (void *, void *)) {
    if (p == NULL) return NULL;
    else {
            OrderType relation = (*compare)(item, p->item);
            switch (relation) {
                    case OrderType::SAME_AS:       return p;
                    case OrderType::LESS_THAN:     return avl_lookup(p->left,  item, compare);
                    case OrderType::GREATER_THAN:  return avl_lookup(p->right, item, compare);
                    default: fatal::fatal_error("avl_lookup: not comparable");
            }
    return NULL;  /* won't happen */
  }
}



void * AvltreeContainer::avl_find(Avl_node p, void *item, OrderType (*compare) (void *, void *)) {
    Avl_node n = avl_lookup(p, item, compare);
    return (n == NULL ? NULL : n->item);
}

void * AvltreeContainer::avl_find(void *item, OrderType (*compare)(void *, void *)) {
    Avl_node n = avl_lookup(root, item, compare);
    return (n == NULL ? NULL : n->item);
}



void * AvltreeContainer::avl_smallest(Avl_node p) {
 if (p == NULL) return NULL;  /* happens at top only */
 else if (p->left == NULL) return p->item;
 else return avl_smallest(p->left);
}

void * AvltreeContainer::avl_largest(Avl_node p) {
 if (p == NULL)   return NULL;  /* happens at top only */
 else if (p->right == NULL) return p->item;
 else  return avl_largest(p->right);
}

void * AvltreeContainer::avl_smallest(void) {
    return avl_smallest(root);
}

void * AvltreeContainer::avl_largest(void) {
    return avl_largest(root);
}


int AvltreeContainer::avl_place(Avl_node p, void *item,   OrderType (*compare) (void *, void *)) {
  if (p == NULL)  return 1;
  else {
        OrderType relation = (*compare)(item, p->item);
        switch (relation) {
            case OrderType::LESS_THAN:  return avl_place(p->left, item, compare);
            case OrderType::SAME_AS:    return avl_size(p->left) + 1;
            case OrderType::GREATER_THAN:  return avl_size(p->left) + 1 + avl_place(p->right, item, compare);
            default: fatal::fatal_error("avl_place, not comparable");
        }
      return INT_MAX;  
    }
}

int AvltreeContainer::avl_place(void *item, OrderType (*compare) (void *, void *)) {
    return avl_place(root, item, compare);
}



double AvltreeContainer::avl_position(Avl_node p, void *item,  OrderType (*compare) (void *, void *)) {
    int place = avl_place(p, item, compare);
    return place / (double) avl_size(p);
}

double AvltreeContainer::avl_position(void *item,  OrderType (*compare) (void *, void *)) {
        return avl_position(root, item, compare);
}

void * AvltreeContainer::avl_nth_item(Avl_node p, int n) {
    if (p == NULL || n < 1 || n > p->size)  return NULL;
    else if (n <= avl_size(p->left))  return avl_nth_item(p->left, n);
    else if (n == avl_size(p->left)+1)  return p->item;
    else  return avl_nth_item(p->right, n - (avl_size(p->left)+1));  
}

void * AvltreeContainer::avl_nth_item(int n) {
    return avl_nth_item(root,n);
}

void * AvltreeContainer::avl_item_at_position(Avl_node p, double pos) {
  if (p == NULL || pos <= 0.0 || pos > 1.0) return NULL;
  else {
    int n = (int) ceil(pos * p->size);
    /* It should be, but make sure that 1 <= n <= p->size. */
    n = (n < 1 ? 1 : (n > p->size ? p->size : n));
    return avl_nth_item(p, n);
  }
}

void * AvltreeContainer::avl_item_at_position(double pos) {
    return avl_item_at_position(root,pos);
}


void AvltreeContainer::avl_check(Avl_node p, OrderType (*compare) (void *, void *)) {
    if (p != NULL) {
        avl_check(p->left, compare);
        avl_check(p->right, compare);

    if (p->left && (*compare)(p->left->item, p->item) != OrderType::LESS_THAN) {
            cout<<"error: left not smaller:"<<p->item<<endl;
            cerr<<"error: left not smaller:"<<p->item<<endl;
        }
    if (p->right && (*compare)(p->right->item, p->item) != OrderType::GREATER_THAN) {
            cout<<"error: right not greater:"<<p->item<<endl;
            cerr<<"error: right not greater:"<<p->item<<endl;
        }
    if (p->height != IMAX(avl_height(p->left), avl_height(p->right)) + 1) {
        printf("error: height wrong: %p\n", p->item);
        fprintf(stderr, "error: height wrong: %p\n", p->item);
    }
    if (p->size != (avl_size(p->left) + avl_size(p->right)) + 1) {
      printf("error: size wrong: %p\n", p->item);
      fprintf(stderr, "error: size wrong: %p\n", p->item);
    }
    if (!balance_ok(p)) {
      printf("error: unbalanced: %p\n", p->item);
      fprintf(stderr, "error: unbalanced: %p\n", p->item);
    }
  }
    
}


void AvltreeContainer::avl_check(OrderType (*compare) (void *, void *)) {
        return avl_check(root,compare);
}


void AvltreeContainer::p_avl(Avl_node p, int level) {
  int i;
  if (p == NULL) {
    for (i = 0; i < level; i++)   cout<<"    ";
    cout<< "----"<<endl;
  }
  else {
    p_avl(p->right, level+1);
    for (i = 0; i < level; i++) cout<<"    ";
    int *x=(int *) (p->item);
    cout <<setw(4)<<*x<<endl;
    p_avl(p->left, level+1);
  }
    
}

void AvltreeContainer::p_avl(int level) {
        p_avl(root,level);
}
