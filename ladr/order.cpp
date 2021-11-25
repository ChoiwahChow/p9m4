#include "order.h"
#include <stdlib.h>


void myOrder::merge_sort_recurse(void *a[], void *w[], int start, int end, OrderType (*comp_proc) (void *, void *) ) {
   int mid, i, i1, i2, e1, e2;
    if (start < end) {
                      mid = (start+end)/2;
                      merge_sort_recurse(a, w, start, mid, comp_proc);
                      merge_sort_recurse(a, w, mid+1, end, comp_proc);
                      i1 = start; e1 = mid;
                      i2 = mid+1; e2 = end;
                      i = start;
                      while (i1 <= e1 && i2 <= e2) {
                                                 if ((*comp_proc)(a[i1], a[i2]) == OrderType::GREATER_THAN) w[i++] = a[i2++];
                                                 else w[i++] = a[i1++];
                      }

                      if (i2 > e2) while (i1 <= e1) w[i++] = a[i1++];
                      else   while (i2 <= e2) w[i++] = a[i2++];
                      for (i = start; i <= end; i++) a[i] = w[i];
  }
}


void myOrder::merge_sort(void *a[], int n, OrderType (*comp_proc) (void *, void *)) {
   void **work;
   work = (void **) malloc(n * sizeof( void *) );
   merge_sort_recurse((void **) a, (void **) work, 0, n-1, comp_proc);
  
  free(work);
}




OrderType myOrder:: compare_vecs(int *a, int *b, int n){

  for (int i = 0; i < n; i++) {
    if (a[i] < b[i]) return OrderType::LESS_THAN;
    else if (a[i] > b[i]) return OrderType::GREATER_THAN;
  }
  return OrderType::SAME_AS;
}


void myOrder::copy_vec(int *a, int *b, int n) {
    for (int i = 0; i < n; i++)   b[i] = a[i];
}
