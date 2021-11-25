
#include <iomanip>

#include "../ladr/clause_misc.h"
#include "../ladr/ioutil.h"
#include "../ladr/literals.h"
#include "../ladr/topform.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "mirror-flip";

static constexpr char Help_string[] =
"\nThis program ... \n";

static bool Debug = false;


static
Term mirror_term(Term t)
{
  if (ARITY(t) == 2) {
    Term alpha = ARG(t,0);
    Term beta  = ARG(t,1);
    ARG(t,0) = mirror_term(beta);
    ARG(t,1) = mirror_term(alpha);
  }
  else if (ARITY(t) == 3) {
    Term alpha = ARG(t,0);
    Term beta  = ARG(t,1);
    Term gamma = ARG(t,2);
    ARG(t,0) = mirror_term(gamma);
    ARG(t,1) = mirror_term(beta);
    ARG(t,2) = mirror_term(alpha);
  }
  return t;
}


static
Topform mirror(Topform c)
{
  TopformContainer tf_con;
  Topform m = tf_con.copy_clause(c);
  Term atom = m->literals->atom;
  ARG(atom,0) = mirror_term(ARG(atom, 0));
  ARG(atom,1) = mirror_term(ARG(atom, 1));
  tf_con.renumber_variables(m, MAX_VARS);
  return m;
}


static
Topform flip(Topform c)
{
  TopformContainer tf_con;
  Topform f = tf_con.copy_clause(c);
  Term atom = f->literals->atom;
  Term alpha = ARG(atom,0);
  ARG(atom,0) = ARG(atom,1);
  ARG(atom,1) = alpha;
  tf_con.renumber_variables(f, MAX_VARS);
  return f;
}


static
bool contains_mirror_flip(Topform c, Plist kept)
{
  Topform f = flip(c);
  Topform m = mirror(c);
  Topform fm = flip(m);

  if (Debug) {
    std::cout << "\ntesting: "; Ioutil::f_clause(c);
    std::cout << "flip:    "; Ioutil::f_clause(f);
    std::cout << "mirror:  "; Ioutil::f_clause(m);
    std::cout << "fm:      "; Ioutil::f_clause(fm);
  }

  bool found = false;
  LiteralsContainer  l_con;
  for (Plist p = kept; p && !found; p = p->next) {
    Topform k = static_cast<Topform>(p->v);
    if (l_con.clause_ident(k->literals, c->literals) ||
        l_con.clause_ident(k->literals, f->literals) ||
        l_con.clause_ident(k->literals, m->literals) ||
        l_con.clause_ident(k->literals, fm->literals))
      found = true;
  }
  ClauseMisc::delete_clause(f);
  ClauseMisc::delete_clause(m);
  ClauseMisc::delete_clause(fm);
  return found;
}


int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  TopInput topInput;
  topInput.init_standard_ladr();

  AttributeContainer a_con;
  a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);  /* ignore these */
  a_con.register_attribute("answer", Attribute_type::TERM_ATTRIBUTE);    /* ignore these */

  Topform c = Ioutil::read_clause(std::cin, std::cerr);


  int number_read = 0;
  int number_kept = 0;
  Plist kept = nullptr;
  PlistContainer  kept_con;
  kept_con.set_head(kept);
  while (c != nullptr && !Ioutil::end_of_list_clause(c)) {
    number_read++;

    if (!contains_mirror_flip(c, kept)) {
      number_kept++;
      kept = kept_con.plist_prepend(c);
      Ioutil::fwrite_clause(std::cout, c, (int)Clause_print_format::CL_FORM_BARE);
    }

    c = Ioutil::read_clause(std::cin, std::cerr);
  }

  std::cout << "% " << PROGRAM_NAME << ": read " << number_read << ", kept " << number_kept
            << " " << std::setprecision(2) << myClock::user_seconds() << ".\n";

  exit(0);
}
