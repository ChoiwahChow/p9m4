#include "termflag.h"
#include "fatal.h"
#include "ladrvglobais.h"







GlobalTermflag::GlobalTermflag()
{
    for(int i=0; i<TERM_BITS; i++)
        bits_in_use[i]=0;
    
}

GlobalTermflag::~GlobalTermflag()
{

    
}






/* DOCUMENTATION
This routine returns an available flag number for marking terms.
If the use will be temporary, make sure to call release_term_flag()
when you are finished using the marks.
A fatal error occurs if no more flags are available.
*/

/* PUBLIC */

int TermflagContainer::claim_term_flag(void){

  int i = 0;
  while (i < TERM_BITS && LADR_GLOBAL_TERM_FLAG.bits_in_use[i])  i++;
  if (i < TERM_BITS) {
        LADR_GLOBAL_TERM_FLAG.bits_in_use[i] = 1;
        return i;
  }
  else {
    fatal::fatal_error("claim_term_flag, no more flags are available");
    return -1;  
  }

} 
/* claim_term_flag */

/* DOCUMENTATION
This routine frees a flag number for future use.
*/

/* PUBLIC */
void TermflagContainer::release_term_flag(const int bit) {
  LADR_GLOBAL_TERM_FLAG.bits_in_use[bit] = 0;
}  /* release_term_flag */


/* DOCUMENTATION
This routine sets a flag on a term.  The flag argument is
a small integer in the range [0 .. n-1], where n is the
number of bits available for flags.  If n is out of range,
none of the flags will change.
<P>
Term flags are stored as bits in the field private_flags.
(Look at the definition of Term to find out big private_flags
is.)  If you need more flags, you can simply change the
type of private_flags to unsigned short (usually 16 bits)
or unsigned int (usually 32 bits).  It is your responsibility
to make sure that all of the flags you use are in range.
*/

/* PUBLIC */
void TermflagContainer::term_flag_set(const Term t, const int flag) {
  if (flag >= 0 && flag < TERM_BITS)
    SET_BIT(t->private_flags, 1 << flag);
}  /* term_flag_set */


/* DOCUMENTATION
This routine clears a flag on a term.  The flag argument is
a small integer in the range [0 .. n-1], where n is the
number of bits available for flags.  If n is out of range,
none of the flags will change.
*/

/* PUBLIC */
void TermflagContainer::term_flag_clear(const Term t, const int flag) {
  if (flag >= 0 && flag < TERM_BITS)
    CLEAR_BIT(t->private_flags, 1 << flag);
}  /* term_flag_clear */



/* DOCUMENTATION
This function gets the value of a flag on a term.  The flag
argument is a small integer in the range [0 ... n-1], where n is the
number of bits available for flags.  If n is out of range, FALSE is
returned.
*/

/* PUBLIC */
bool TermflagContainer::term_flag(const Term t, const int flag) {
  if (flag < 0 || flag >= TERM_BITS)   return false;
  else if (TP_BIT(t->private_flags, 1 << flag)) return true;
  else  return false;
}  /* term_flag */


/* DOCUMENTATION
This routine returns the number of bits available for term flags.
The value should always be at least 8.  If the value is n,
you can use flags numbered [0 ... n-1].
*/

/* PUBLIC */
int TermflagContainer::term_flags(void) const{
  return TERM_BITS;
}  /* term_flags */


/* DOCUMENTATION
This routine copies a term, including <I>all of the flags</I>
of the term and its subterms.  Any other extra fields are not
copied.
*/

/* PUBLIC */
Term TermflagContainer::copy_term_with_flags(const Term t) {
  TermContainer T;
  Term t2;
  if (VARIABLE(t)) t2 = T.get_variable_term(VARNUM(t));
  else {
        int i;
        t2 = T.get_rigid_term_like(t);
        for (i = 0; i < ARITY(t); i++) ARG(t2,i) = copy_term_with_flags(ARG(t,i));
       }
  t2->private_flags = t->private_flags;
  return t2;
}  /* copy_term_with_flags */


/* DOCUMENTATION
This routine copies a term, including <I>one specified flag</I>
of the term and its subterms.  Any other flags or extra fields are not
copied.
*/

/* PUBLIC */
Term TermflagContainer::copy_term_with_flag(const Term t, const int flag) {
  Term t2;
  TermContainer T;
  if (VARIABLE(t))
    t2 = T.get_variable_term(VARNUM(t));
  else {
    int i;
    t2 = T.get_rigid_term_like(t);
    for (i = 0; i < ARITY(t); i++)
      ARG(t2,i) = copy_term_with_flag(ARG(t,i), flag);
  }
  if (term_flag(t, flag))
    term_flag_set(t2, flag);
  return t2;
}  /* copy_term_with_flag */


/* DOCUMENTATION
*/

/* PUBLIC */
void TermflagContainer::term_flag_clear_recursively(const Term t, const int flag){
  int i;
  term_flag_clear(t, flag);
  for (i = 0; i < ARITY(t); i++)
    term_flag_clear_recursively(ARG(t,i), flag);
}  /* clear_term_flag_recursively */
