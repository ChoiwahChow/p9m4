#ifndef  _ORDER_H
#define  _ORDER_H





enum class OrderType {
                            NOT_COMPARABLE,
                            SAME_AS,
                            LESS_THAN,
                            GREATER_THAN,
                            LESS_THAN_OR_SAME_AS,
                            GREATER_THAN_OR_SAME_AS,
                            NOT_LESS_THAN,
                            NOT_GREATER_THAN
};


 class  myOrder {
     
 public:
            
                  static void merge_sort_recurse(void *a[], void *w[],int start, int end, OrderType (*comp_proc) (void *, void *) );
                  static void merge_sort(void *a[], int n, OrderType (*comp_proc) (void *, void *));
                  static OrderType compare_vecs(int *, int *, int );
                  static void copy_vec(int *, int *, int );
                  
                 
};


#endif
