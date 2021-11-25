
#include <iomanip>

#include "../ladr/clock.h"
#include "../ladr/fastparse.h"
#include "../ladr/ioutil.h"
#include "../ladr/symbols.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "latfilter";


static constexpr char Help_string[] =
"\nThis program takes a stream of meet/join equations (from stdin)\n"
"and writes (to stdout) those that are lattice identities.\n"
"Whitman's procedure is used.  An optional argument 'fast' says\n"
"to read and write the clauses in fastparse form (e.g., =mxxx.).\n"
"The base terms can be either constants or variables.\n\n"
"Example LT identities.\n\n"
"  ordinary:   'x ^ (x v y) = x.'\n"
"  fastparse:  '=mxjxyx.' (exactly one per line, without spaces)\n\n"
"Another optional argument 'x' says to output the equations that are\n"
"*not* OL identities.\n\n";

/* We cache the important symbol numbers to avoid symbol table lookups. */

int Meet_sym;  /* ^ */
int Join_sym;  /* v */
int M_sym;     /* m */
int J_sym;     /* j */

bool meet_term(Term t)
{
  return (SYMNUM(t) == Meet_sym || SYMNUM(t) == M_sym);
}


bool join_term(Term t)
{
  return (SYMNUM(t) == Join_sym || SYMNUM(t) == J_sym);
}


/* DOCUMENTATION
Given lattice terms S and T, this routine checks if S <= T.
It is assumed that S and T are in terms of binary operations
meet ("^", "meet", or "m") and join ("v", "join", or "j").
<P>
Whitman's algorithm (from the 1930s) is used, as described in
"Free Lattices", by Freese, Jezek, and Nation, AMS Mathematical
Surveys and Monographs, Vol. 42 (1991).
<P>
Solutions to subproblems are not cached, so the behavior of
this implementation can be exponential.
*/

bool lattice_leq(Term s, Term t)
{
  if (VARIABLE(s) && (VARIABLE(t)))        /* (1) */
    return (VARNUM(s) == VARNUM(t));

  else if (CONSTANT(s) && (CONSTANT(t)))   /* (1) */
    return (SYMNUM(s) == SYMNUM(t));

  else if (join_term(s))                   /* (2) */
    return (lattice_leq(ARG(s,0), t) &&
	    lattice_leq(ARG(s,1), t));

  else if (meet_term(t))                   /* (3) */
    return (lattice_leq(s, ARG(t,0)) &&
	    lattice_leq(s, ARG(t,1)));

  else if ((VARIABLE(s) || CONSTANT(s)) && join_term(t))    /* (4) */
    return (lattice_leq(s, ARG(t,0)) ||
	    lattice_leq(s, ARG(t,1)));

  else if (meet_term(s) && (VARIABLE(t) || CONSTANT(t)))    /* (5) */
    return (lattice_leq(ARG(s,0), t) ||
	    lattice_leq(ARG(s,1), t));

  else if (meet_term(s) && join_term(t))   /* (6) */
    return (lattice_leq(s, ARG(t,0)) ||
	    lattice_leq(s, ARG(t,1)) ||
	    lattice_leq(ARG(s,0), t) ||
	    lattice_leq(ARG(s,1), t));

  else
    return false;

}


int lattice_identity(Term atom)
{
  SymbolContainer s_con;
  return (atom != nullptr && s_con.is_symbol(SYMNUM(atom), "=", 2) &&
          lattice_leq(ARG(atom,0), ARG(atom,1)) && lattice_leq(ARG(atom,1), ARG(atom,0)));
}


int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  bool fast_parse = isu::which_member_arg(argc, argv, "fast") > 0;
  bool output_non_identities = isu::which_member_arg(argc, argv, "x") > 0;

  /* Assume stdin contains equality units.  For each, if it is a
     lattice identity, print it to stdout.

     Note that if we're not using fastparse, we use read_term
     which does not "set_variables"; that is,
     the terms that you expect to be variables are still constants.
     That's okay, because the lattice identity checker doesn't care.
  */

  if (fast_parse) {
    /* Declare the symbols for fastparse. */
    Fastparse::fast_set_defaults();
  }
  else {
    TopInput topInput;
    topInput.init_standard_ladr();
  }

  /* Cache the symbol numbers for decision procedure lattice_leq. */

  SymbolContainer s_con;
  M_sym = s_con.str_to_sn("m", 2);
  J_sym = s_con.str_to_sn("j", 2);
  Meet_sym = s_con.str_to_sn("^", 2);  /* not used for fastparse */
  Join_sym = s_con.str_to_sn("v", 2);  /* not used for fastparse */

  unsigned long int checked = 0;
  unsigned long int passed = 0;
  TermContainer   t_con;
  Term t = Ioutil::term_reader(fast_parse);

  while (t != nullptr) {
    bool ident = lattice_identity(t);
    checked++;
    if ((!output_non_identities && ident) || (output_non_identities && !ident)) {
      passed++;
      Ioutil::term_writer(t, fast_parse);
      std::flush(std::cout);
    }
    t_con.zap_term(t);
    t = Ioutil::term_reader(fast_parse);
  }

  std::cout << "% latfilter" << (output_non_identities ? " x" : "") << ": checked " << checked << ", passed " << passed
            << ", user " << std::setprecision(2) << myClock::user_seconds() << ", system "
            << std::setprecision(2) << myClock::system_seconds() << ".\n";

  exit(0);
}
