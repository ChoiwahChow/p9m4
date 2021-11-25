#include "fpalist.h"
#include "ladrvglobais.h"
#include <iomanip>
#include "fpalist.h"
#include "memory.h"
#include "term.h"
#include "fatal.h"



GlobalFpaList::GlobalFpaList() {
    Fpa_chunk_gets=0;
    Fpa_chunk_frees=0;
    Fpa_list_gets=0;
    Fpa_list_frees=0;
    Chunk_mem=0;
}

GlobalFpaList::~GlobalFpaList() {
}



FpalistContainer::FpalistContainer() {  head=NULL;}
FpalistContainer::~FpalistContainer() { head=NULL;}



Fpa_chunk FpalistContainer::get_fpa_chunk(int n){
  Fpa_chunk p = (Fpa_chunk) Memory::memCNew(sizeof(struct fpa_chunk));
  p->d = (Term *) Memory::memCNew(n * sizeof(Term *)); //term array
  LADR_GLOBAL_FPA_LIST.Chunk_mem += n;
  p->size = n;
  LADR_GLOBAL_FPA_LIST.Fpa_chunk_gets++;
  return(p);
}




Fpa_list FpalistContainer::get_fpa_list(void) {
  Fpa_list p = (Fpa_list) Memory::memCNew(sizeof(struct fpa_list));
  p->chunksize = F_INITIAL_SIZE;
  LADR_GLOBAL_FPA_LIST.Fpa_list_gets++;
  return(p);
}



void FpalistContainer::free_fpa_chunk(Fpa_chunk p) {
  LADR_GLOBAL_FPA_LIST.Chunk_mem -= p->size;
  Memory::memFree((Term *) p->d, p->size);
  Memory::memFree((Fpa_chunk)p, sizeof(struct fpa_chunk));
  LADR_GLOBAL_FPA_LIST.Fpa_chunk_frees++;
}


void FpalistContainer::free_fpa_list(Fpa_list p) {
  Memory::memFree((Fpa_list) p, sizeof(struct fpa_list));
  LADR_GLOBAL_FPA_LIST.Fpa_list_frees++;
}

void FpalistContainer::flag_fpa_leaf_clauses(Fpa_chunk c) {
  Topform clause;

  for (; c; c = c->next) {
    int i;
    /* terms right justified in chunk */
    for (i = c->size-c->n; i < c->size; i++) {
      Term t = c->d[i];

      clause = (Topform) t -> container;

      // for debugging
      // fprint_term(stdout, clause->literals->atom);

      // flag the clause for future identification
      clause -> used = true;
    }
  }
}


void FpalistContainer::fprint_fpalist_mem(ostream &o, bool heading) {
  int n;
  if (heading)
    o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct fpa_chunk);
  o<<"fpa_chunk   ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_FPA_LIST.Fpa_chunk_gets<<setw(11)<<LADR_GLOBAL_FPA_LIST.Fpa_chunk_frees;
  o<<setw(11)<<LADR_GLOBAL_FPA_LIST.Fpa_chunk_gets-LADR_GLOBAL_FPA_LIST.Fpa_chunk_frees;
  o<<setw(9)<<((LADR_GLOBAL_FPA_LIST.Fpa_chunk_gets - LADR_GLOBAL_FPA_LIST.Fpa_chunk_frees) * n) / 1024<<"K"<<endl;
  
  
  n = sizeof(struct fpa_list);
  o<<"fpa_list    ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_FPA_LIST.Fpa_list_gets<<setw(11)<<LADR_GLOBAL_FPA_LIST.Fpa_list_frees;
  o<<setw(11)<<LADR_GLOBAL_FPA_LIST.Fpa_list_gets-LADR_GLOBAL_FPA_LIST.Fpa_list_frees;
  o<<setw(9)<<((LADR_GLOBAL_FPA_LIST.Fpa_list_gets - LADR_GLOBAL_FPA_LIST.Fpa_list_frees) * n) / 1024<<"K"<<endl;
}

void FpalistContainer::p_fpalist_mem(void){
  fprint_fpalist_mem(cout, true);
} 

Fpa_chunk FpalistContainer::double_chunksize(Fpa_chunk f) {
  if (f == NULL)  return NULL;
  else if (f->next == NULL) {
    fatal::fatal_error("double_chunksize, parity error");
    return NULL;
  }
  else {
    int newsize = f->size * 2;
    Fpa_chunk g = f->next;
    Fpa_chunk novo = get_fpa_chunk(newsize);
    /* put f and g terms into new, free f and g, return new */
    int i = newsize - (f->n + g->n);
    int j;
    for (j = f->size - f->n; j < f->size; j++)  novo->d[i++] = f->d[j];
    for (j = g->size - g->n; j < g->size; j++)  novo->d[i++] = g->d[j];
    novo->n = f->n + g->n;
    novo->head = f->head;
    novo->next = double_chunksize(g->next);
    free_fpa_chunk(f);
    free_fpa_chunk(g);
    return novo;
  }
}

Fpa_chunk FpalistContainer::flist_insert(Fpa_chunk f, Term x, Fpa_list _head) {
  if (f == NULL) {
    f = get_fpa_chunk(_head->chunksize);
    _head->num_chunks++;
    FLAST(f) = x;
    f->n = 1;
  }
  else if (f->n == f->size) {
    /* chunk is full */
    if (FLT(x,FLAST(f)))
      f->next = flist_insert(f->next, x, _head);
    else if (x == FLAST(f))
      fatal::fatal_error("flist_insert, item already here (1)");
    else if (FGT(x,FFIRST(f))) {
      /* 
	 This special case isn't necessary.  It is to improve performance.
	 The application for which I'm writing this inserts items in
	 increasing order (most of the time), and this prevents a lot of
	 half-empty chunks in that case.
      */
      Fpa_chunk f2 = flist_insert(NULL, x, _head);
      f2->next = f;
      f = f2;
    }
    else {
      /* split this chunk in half (new chunk has same size) */
      Fpa_chunk f2 = get_fpa_chunk(f->size);
      int move = f2->size / 2;
      int i, j;
      _head->num_chunks++;
      for (i = 0, j = f->size - move; i < move; i++, j++) {
        f2->d[j] = f->d[i];
        f->d[i] = NULL;
      }
      f2->n = move;
      f->n = f->size - move;
      f2->next = f;
      f = flist_insert(f2, x, _head);
    }
  }
  else {
    /* chunk not full */
    if (f->next && FLE(x,FFIRST(f->next)))
      f->next = flist_insert(f->next, x, _head);  /* insert into next chunk */
    else {
      /* insert into this chunk */
      int n = f->n;
      int i = f->size - n;
      while (i < f->size && FLT(x,f->d[i]))	i++;
      if (i < f->size && x == f->d[i])	fatal::fatal_error("flist_insert, item already here (2)");
      if (i == f->size - n) {
        f->d[i-1] = x;
        f->n = n+1;
      }
      else {
	/* insert at i-1, shifting the rest */
            int j;
            for (j = f->size-n; j < i; j++)  f->d[j-1] = f->d[j];
            f->d[i-1] = x;
            f->n = n+1;
      }
    }
  }
  return f;
} 



void FpalistContainer::fpalist_insert(Fpa_list p, Term t) {
  p->chunks = flist_insert(p->chunks, t, p);
  p->num_terms++;

#if 0
  printf("insert %p, %d chunks of size %d, %d terms\n",
	 p, p->num_chunks, p->chunksize, p->num_terms);
#endif

  if (p->num_chunks == p->chunksize && p->chunksize < F_MAX_SIZE) {
#if 0
    printf("doubling %p chunksize from %d to %d (%d chunks, density=%.2f)\n",
	   p,
	   p->chunksize, p->chunksize * 2, p->num_chunks,
	   p->num_terms / (double) (p->num_chunks * p->chunksize));
#endif
    p->chunks = double_chunksize(p->chunks);
    p->chunksize = p->chunksize * 2;
    p->num_chunks = p->num_chunks / 2;
  }
}


Fpa_chunk FpalistContainer::consolidate(Fpa_chunk f, Fpa_list _head) {
  if (f->next && f->n + f->next->n <= f->size) {
    Fpa_chunk f2 = f->next;
    int i;
    for (i = 0; i < f->n; i++)
      f2->d[f->size - (f2->n + i + 1)] = f->d[f->size - (i+1)];
    f2->n = f->n + f2->n;
    free_fpa_chunk(f);
    _head->num_chunks--;
    return f2;
  }
  else return f;
} 


Fpa_chunk FpalistContainer::flist_delete(Fpa_chunk f, Term x, Fpa_list _head) {
  if (f == NULL)    fatal::fatal_error("flist_delete, item not found (1)");

  if (FLT(x,FLAST(f)))
    f->next = flist_delete(f->next, x, _head);
  else {
    int n = f->n;
    int i = f->size - n;
    int j;
    while (i < f->size && FLT(x,f->d[i]))   i++;
    if (x != f->d[i])    fatal::fatal_error("flist_delete, item not found (2)");

    /* delete and close the hole */
    for (j = i; j > f->size-n; j--)  f->d[j] = f->d[j-1];
    f->d[j] = NULL;
    f->n = n-1;
    if (f->n == 0) {
      /* delete this chunk */
      Fpa_chunk next = f->next;
      _head->num_chunks--;
      free_fpa_chunk(f);
      f = next;
    }
    else {
      /* try to join this chunk with the next */
      f = consolidate(f, _head);
    }
  }
  return f;
} 



void FpalistContainer::fpalist_delete(Fpa_list p, Term t) {
  p->chunks = flist_delete(p->chunks, t, p);
  p->num_terms--;
#if 0
  printf("delete %p, %d chunks of size %d, %d terms\n",
	 p, p->num_chunks, p->chunksize, p->num_terms);
#endif
} 



struct fposition FpalistContainer::first_fpos(Fpa_list f) {
  return next_fpos((struct fposition) {f->chunks, -1});
}


struct fposition FpalistContainer::next_fpos(struct fposition p) {
  if (p.f == NULL)
    return (struct fposition) {NULL, 0};
  else if (p.i == -1)
    return (struct fposition) {p.f, p.f->size - p.f->n};
  else {
    int i = p.i+1;
    if (i < (p.f)->size)
      return (struct fposition) {p.f, i};
    else
      return next_fpos((struct fposition) {(p.f)->next, -1});
  }
} 

void FpalistContainer::zap_fpa_chunks(Fpa_chunk p) {
  if (p != NULL) {
    zap_fpa_chunks(p->next);
    free_fpa_chunk(p);
  }
}


void FpalistContainer::zap_fpalist(Fpa_list p) {
  zap_fpa_chunks(p->chunks);
  free_fpa_list(p);
}


bool FpalistContainer::fpalist_empty(Fpa_list p) {
  return !p || p->chunks == NULL;
} 


void FpalistContainer::p_fpa_list(Fpa_chunk c) {
  TermContainer T;
  for (; c; c = c->next) {
    int i;
    /* terms right justified in chunk */
    for (i = c->size-c->n; i < c->size; i++) {
      Term t = c->d[i];
      printf(" : ");
      T.fprint_term(cout, t);
    }
  }
} 
