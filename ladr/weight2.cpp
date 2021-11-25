#include "weight2.h"
#include "parse.h"
#include "mystring.h"
#include "fatal.h"




double Weight2::call_weight(string func, Term t) {
  if (myString::str_ident(func, "test1"))    return test1(t);
  else {
    fatal::fatal_error("call_weight, unknown function");
    return 0;  /* to appease the compiler */
  }
}


double Weight2::test1(Term t)
{
  TermContainer T;	
  string s = T.term_to_string(t);  /* remember to free(s) below */
  /* The complexity function returns a double in the range 0.0--1.0. */
  char *aux=new char[s.length()+1];
  aux[s.length()]='\0'; //terminar a string
  double c = complexity_1(aux);
  delete [] aux;
  /* printf("s=%s, complexity=%d:  ", s, c); p_term(t); */
  return c;
} 


int Weight2::char_array_ident(char *s, char *t, int n) {
  int i;
  for (i = 0; i < n; i++)
    if (s[i] != t[i])  return 0;
  return 1;
}


int Weight2::char_array_find(char *b, int nb, char *a, int na) {
  if (nb <= na) {
    int i;
    for (i = 0; i <= na - nb; i++) {
      if (char_array_ident(a+i, b, nb)) 	return i;  /* b is a subsequence of a at position i */
    }
  }
  return -1;  /* b is not a subsequence */
}


double Weight2::complexity_1(char * s) {
  /* This is based on Zac's original perl code.  It's not quite right. */
  int length = strlen(s);
  int min = 2;
  int index = min;
  int total = 0;

  if (length < 3)
    return 0.0;
  
  while (index <= length-2) {  /* index of where the search begins */
    int flen = 0;  /* length of found redundancy */
    int maxlook = (length - index);
    int window;
    /* don't check for occurences longer than head of string */
    if (maxlook > index)
      maxlook = index;
    /* printf("checking at index %d\n", index); */
    /* printf("will chack strings of length %d through %d.\n", min, maxlook); */
    for (window = min; window < maxlook+1; window++) {
      char *tocheck = s + index;
      int ind = char_array_find(tocheck, window, s, length);
       
      /* printf("Want to know if %s occurs in pos 0 until %d.\n", tocheck,index); */
      /* ind = string.find(tocheck); */

      if (ind < index) flen = window;
    }

    if (flen == 0)
      index += 1;
    else {
      index += flen;
      total += flen;
    }
  }
  /* printf("total=%d,index=%d,length=%d,MIN=%d\n",total,index,length,min); */
  /* printf("TOTAL REDUNDANCY: %d\n", total); */
  return ((double) total) / (length-2);

}



