#include "nonport.h"
#include <string.h>


#ifdef PRIMITIVE_ENVIRONMENT
/* This means that we don't have some UNIXy things */
#else
#  include <pwd.h>
#  include <unistd.h>
#endif

int nonport::get_bits(void)
{
  return sizeof(long) == 8 && sizeof(void *) == 8 ? 64 : 32;
}  


string nonport::username(void)
{
#ifdef PRIMITIVE_ENVIRONMENT
  return("an unknown user");
#else
  struct passwd *p;
  p = getpwuid(getuid());
  if(p) return p->pw_name;
  else return "???";
#endif
}  /* username */

/*************
 *
 *   hostname()
 *
 *************/

/* DOCUMENTATION
Return the hostname of the computer on which the current job is running.
*/

/* PUBLIC */
string nonport::hostname(void)
{
#ifdef PRIMITIVE_ENVIRONMENT
  return("an unknown computer");
#else
  static char host[64];
  if (gethostname(host, 64) != 0)
    strcpy(host, "???");
  return(host);
#endif
}  /* hostname */

/*************
 *
 *   my_process_id()
 *
 *************/

/* DOCUMENTATION
Return the process ID of the current process.
*/

/* PUBLIC */
int nonport::my_process_id(void)
{
#ifdef PRIMITIVE_ENVIRONMENT
  return 0;
#else
  return getpid();
#endif
}  /* my_process_id */
