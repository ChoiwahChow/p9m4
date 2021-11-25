#include "multiset.h"
#include "ladrvglobais.h"



int MultiSet::num_occurrences(const Term t, const Term a[], const int n) {
  int i, count;
  TermContainer T;
  for (i = 0, count = 0; i < n; i++)
    if (T.term_ident(a[i], t))    count++;
  return count;
}  /* num_occurrences */



Plist MultiSet::set_of_more_occurrences(const Term a1[], const int n1, const Term a2[], const int n2){
  PlistContainer answer;
  int i, j;
  for (i = 0; i < n1; i++) {
    Term e1 = a1[i];
    /* Check if this is the first occurrence of e1. */
    TermContainer T;
    for (j = 0; j != i && !T.term_ident(e1, a1[j]); j++);
    if (i == j && num_occurrences(e1,a1,n1) > num_occurrences(e1,a2,n2))   answer.plist_prepend(e1);
  }
  return answer.get_head();
}  /* set_of_more_occurrences */



bool MultiSet::greater_multiset(const Term a1[], const int n1, const Term a2[], const int n2,  bool (*comp_proc) (Term, Term, bool), const bool lex_order_vars)
{
  PlistContainer P;
  Plist s1, s2, p1, p2;
  bool ok;
  s1 = set_of_more_occurrences(a1, n1, a2, n2);  /* more occurrences in a1 */
  s2 = set_of_more_occurrences(a2, n2, a1, n1);  /* more occurrences in a2 */
  /*
   * return (s1 not empty and foreach p2 in s2
   * there is an p1 in s1 such that p1 > p2).
   */
  if (s1 == NULL)    ok = false;
  else {
    for (p2 = s2, ok = true; p2 && ok; p2 = p2->next)
      for (p1 = s1, ok = false; p1 && !ok; p1 = p1->next)
        ok = (*comp_proc)((Term)p1->v, (Term)p2->v, lex_order_vars);
  }
  P.set_head(s1);
  P.zap_plist();
  P.set_head(s2);
  P.zap_plist();
  return ok;
}  /* greater_multiset */

