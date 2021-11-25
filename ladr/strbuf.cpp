#include "ladrvglobais.h"
#include "strbuf.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>
#include "mystring.h"
#include <iomanip>




GlobalStrbuf::GlobalStrbuf() {
    String_buf_frees=0;
    String_buf_gets=0;
    Chunk_gets=0;
    Chunk_frees=0;
}

GlobalStrbuf::~GlobalStrbuf() {
}


StrbufContainer::StrbufContainer() {
  
}

StrbufContainer::~StrbufContainer() {
  
}

Chunk StrbufContainer::get_chunk(void) {
  Chunk p =(Chunk) Memory::memCNew(sizeof(struct chunk));
  LADR_GLOBAL_STR_BUF.Chunk_gets++;
  return(p);
}

void StrbufContainer::free_chunk(Chunk p){
  Memory::memFree((void *)p, sizeof(struct chunk));
  LADR_GLOBAL_STR_BUF.Chunk_frees++;
}  /* free_chunk */


String_buf StrbufContainer::private_get_string_buf(void){
  String_buf p = (String_buf) Memory::memCNew(sizeof(struct string_buf));
  p->size=0;
  LADR_GLOBAL_STR_BUF.String_buf_gets++;
  return(p);
}  /* get_string_buf */


void StrbufContainer::free_string_buf(String_buf s) {
  Memory::memFree((void *)s, sizeof(struct string_buf));
  LADR_GLOBAL_STR_BUF.String_buf_frees++;
}  /* free_string_buf */




void StrbufContainer::zap_string_buf(String_buf sb) {
  Chunk curr, prev;
  curr = sb->first;
  while (curr != NULL) {
    prev = curr;
    curr = curr->next;
    free_chunk(prev);
  }
  free_string_buf(sb);
}

void StrbufContainer::fprint_strbuf_mem(ostream &o, bool heading) {
  int n;
  if (heading) o<<"  type (bytes each)               gets      frees      in use      bytes"<<endl;
  n = sizeof(struct chunk);
  o<<"chunk       ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_STR_BUF.Chunk_gets;
  o<<setw(11)<<LADR_GLOBAL_STR_BUF.Chunk_frees;
  o<<setw(11)<<LADR_GLOBAL_STR_BUF.Chunk_gets-LADR_GLOBAL_STR_BUF.Chunk_frees;
  o<<setw(9)<<((LADR_GLOBAL_STR_BUF.Chunk_gets-LADR_GLOBAL_STR_BUF.Chunk_frees)*n)/1024<<"K"<<endl;

  n = sizeof(struct string_buf);
  o<<"string_buf  ("<<setw(4)<<n<<")        "<<setw(11)<<LADR_GLOBAL_STR_BUF.String_buf_gets;
  o<<setw(11)<<LADR_GLOBAL_STR_BUF.String_buf_frees;
  o<<setw(11)<<LADR_GLOBAL_STR_BUF.String_buf_gets-LADR_GLOBAL_STR_BUF.String_buf_frees;
  o<<setw(9)<<((LADR_GLOBAL_STR_BUF.String_buf_gets-LADR_GLOBAL_STR_BUF.String_buf_frees)*n)/1024<<"K"<<endl;

}  /* fprint_strbuf_mem */

/*************
 *
 *   p_strbuf_mem()
 *
 *************/

/* DOCUMENTATION
This routine prints (to stdout) Memory:: usage statistics for data types
associated with the strbuf package.
*/

/* PUBLIC */
void StrbufContainer::p_strbuf_mem() {
  fprint_strbuf_mem(cout, 1);
}  /* p_strbuf_mem */




/* DOCUMENTATION
This routine allocates and returns a String_buf, initialized
to string s.  Don't forget to call zap_string_buf(sb) when
finished with it.
Also see get_string_buf().
*/

/* PUBLIC */

String_buf StrbufContainer::private_init_string_buf(char *aux) {
  String_buf sb = private_get_string_buf();
  sb_append(sb,aux);
  return sb;
}  /* init_string_buf*/


String_buf StrbufContainer::private_init_string_buf(string aux) {
 String_buf sb = private_get_string_buf();
 sb_append(sb, aux);
 return sb;
}





/*************
 *
 *   fprint_sb()
 *
 *************/

/* DOCUMENTATION
This routine prints String_buf sb to FILE *fp.
*/

/* PUBLIC */
void StrbufContainer::fprint_sb(ostream &o, String_buf sb){
  Chunk h = sb->first;
  int i = 0;
  while (i < sb->size) {
    char c = h->s[i % CHUNK_SIZE];
    if (c != '\0') o<<c;
    i++;
    if (i % CHUNK_SIZE == 0)  h = h->next;
  }
}  /* fprint_sb */

void StrbufContainer::p_sb(String_buf sb) {
    fprint_sb(cout,sb);
    cout<<endl;
}




void StrbufContainer::sb_append(String_buf sb, char *s) {
  int i;
  int n = sb->size;
  Chunk last = sb->first;

  while (last != NULL && last->next != NULL)
    last = last->next;

  for (i = 0; s[i] != '\0'; i++) {
    if (n % CHUNK_SIZE == 0) {
      Chunk novo = get_chunk();
      if (last != NULL) last->next = novo;
      else sb->first = novo;
      last = novo;
    }
    last->s[n % CHUNK_SIZE] = s[i];
    n++;
  }
  sb->size = n;
}  /* sb_append */


void StrbufContainer::sb_append(String_buf sb, string s) {
    sb_append(sb, (char *)s.c_str());
}

void StrbufContainer::sb_append_char(String_buf sb, char c) {
 /* In the first version of this routine, we simply built a string "c",
     and then called sb_append.  This causes a problem, however, if
     we use this package for sequences of small integers.  In particular,
     if we want to append the char 0, that won't work.
  */
  int n = sb->size;
  Chunk last = sb->first;

  while (last != NULL && last->next != NULL)
    last = last->next;

  if (n % CHUNK_SIZE == 0) {
    Chunk novo = get_chunk();
    if (last != NULL)
      last->next = novo;
    else sb->first = novo;
    last = novo;
  }
  last->s[n % CHUNK_SIZE] = c;
  n++;
  sb->size = n;
}


void StrbufContainer::sb_replace_char(String_buf sb, int i, char c) {
if (i < 0 || i >= sb->size)    return;
  else {
    Chunk h = sb->first;
    int j;
    for (j = 0; j < i / CHUNK_SIZE; j++)
      h = h->next;
    h->s[i % CHUNK_SIZE] = c;
  }
}


char StrbufContainer::sb_char(String_buf sb, int n) {
if (n < 0 || n >= sb->size) return '\0';
  else {
    Chunk h = sb->first;
    int i;
    for (i = 0; i < n / CHUNK_SIZE; i++)
      h = h->next;
    return h->s[n % CHUNK_SIZE];
 }
}


void StrbufContainer::sb_cat_copy(String_buf sb1, String_buf sb2) {
  /* Note that this is inefficient if there are many chunks in either sb. */
  int i;
  char c;
 for (i = 0; (c = sb_char(sb2, i)); i++)
    sb_append_char(sb1, c);
}



void StrbufContainer::sb_cat(String_buf sb1, String_buf sb2) {
  sb_cat_copy(sb1, sb2);
  zap_string_buf(sb2);
}


void StrbufContainer::sb_cat(String_buf sb2) {
    sb_cat(strbuf,sb2);
}

int StrbufContainer::sb_size(String_buf sb) {
    return sb->size;
}


char *StrbufContainer::sb_to_malloc_string(String_buf sb) {
char *s = (char *) malloc (sb->size + 1);
  if (s == NULL) return NULL;
  else {
    int j = 0;  /* index for new string */
    int i;      /* index for Str_buf */
    for (i = 0; i < sb->size; i++) {
      char c = sb_char(sb,i);
      if (c != '\0') s[j++] = c;
    }
    s[j] = '\0';
    return s;
  }
}


char * StrbufContainer::sb_to_malloc_char_array(String_buf sb) {
  char *s =(char *) malloc(sb->size + 1);
  if (s == NULL)  return NULL;
  else {
    int i;
    for (i = 0; i < sb->size; i++)
      s[i] = sb_char(sb,i);
    s[i] = '\0';
    return s;
  }
}

char * StrbufContainer::sb_to_malloc_char_array(void) {
    return sb_to_malloc_char_array(strbuf);
}


void StrbufContainer::sb_append_int(String_buf sb, int i)
{
  string s;
  sb_append(sb,myString::int_to_str(i, s, 25));
}


void StrbufContainer::sb_append_int(int i) {
    sb_append_int(strbuf,i);
}

void StrbufContainer::sb_append(string s) {
    sb_append(strbuf,s);
}

void StrbufContainer::new_string_buf(string s)  {
    strbuf=private_init_string_buf(s);
}


void StrbufContainer::zap_string_buf() {
    zap_string_buf(strbuf);
    strbuf=NULL;
}

bool StrbufContainer::null() {
    return strbuf==NULL;
}

int StrbufContainer::sb_size() {
    return strbuf->size;
}

char * StrbufContainer::sb_to_malloc_string(void) {
        return sb_to_malloc_string(strbuf);
}
 
char StrbufContainer::sb_char(int i) {
    return sb_char(strbuf,i);
}
 

void StrbufContainer::sb_append_char(char c) {
    sb_append_char(strbuf,c);
}
 
void StrbufContainer::new_string_buf(void) {
    strbuf=private_get_string_buf();
}

void StrbufContainer::p_sb(void) {
    p_sb(strbuf);
}


void StrbufContainer::set_string_buf(String_buf sb) {
strbuf=sb;
}

String_buf StrbufContainer::get_string_buf(void) {
        return strbuf;
}

void StrbufContainer::sb_replace_char(int i , char c) {
    sb_replace_char(strbuf,i,c);
}

void StrbufContainer::fprint_sb(ostream &o) {
    fprint_sb(o,strbuf);
}
