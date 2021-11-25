
#include <iomanip>

#include "../ladr/clause_misc.h"
#include "../ladr/ioutil.h"
#include "../ladr/term.h"
#include "../ladr/topform.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "perm3";

static constexpr char Help_string[] =
"\nThis program ... \n";

static bool Debug = true;


static
Term perm3_term(Term t, int *p)
{

  if (ARITY(t) == 3) {
    Term alpha = ARG(t,0);
    Term beta  = ARG(t,1);
    Term gamma = ARG(t,2);
    ARG(t,p[0]) = perm3_term(alpha, p);
    ARG(t,p[1]) = perm3_term(beta, p);
    ARG(t,p[2]) = perm3_term(gamma, p);
  }
  return t;
}


static
Topform perm3(Topform c, int *p)
{
  TopformContainer tf_con;
  Topform m = tf_con.copy_clause(c);
  Term atom = m->literals->atom;
  ARG(atom,0) = perm3_term(ARG(atom, 0), p);
  ARG(atom,1) = perm3_term(ARG(atom, 1), p);
  tf_con.renumber_variables(m, MAX_VARS);
  return m;
}


static
bool contains_perm3(Topform c, Plist kept)
{
  int p[3];
  Topform p1, p2, p3, p4, p5, p6;

  p[0] = 0; p[1] = 1; p[2] = 2; p1 = perm3(c, p);
  p[0] = 0; p[1] = 2; p[2] = 1; p2 = perm3(c, p);
  p[0] = 1; p[1] = 0; p[2] = 2; p3 = perm3(c, p);
  p[0] = 1; p[1] = 2; p[2] = 0; p4 = perm3(c, p);
  p[0] = 2; p[1] = 0; p[2] = 1; p5 = perm3(c, p);
  p[0] = 2; p[1] = 1; p[2] = 0; p6 = perm3(c, p);

  if (Debug) {
    std::cout << "\ntesting: "; Ioutil::f_clause(c);
    std::cout << "p1:    "; Ioutil::f_clause(p1);
    std::cout << "p2:    "; Ioutil::f_clause(p2);
    std::cout << "p3:    "; Ioutil::f_clause(p3);
    std::cout << "p4:    "; Ioutil::f_clause(p4);
    std::cout << "p5:    "; Ioutil::f_clause(p5);
    std::cout << "p6:    "; Ioutil::f_clause(p6);
  }

  LiteralsContainer  l_con;
  bool found = false;
  for (Plist a = kept; a && !found; a = a->next) {
    Topform k = static_cast<Topform>(a->v);
    if (l_con.clause_ident(k->literals, p1->literals) ||
        l_con.clause_ident(k->literals, p2->literals) ||
		l_con.clause_ident(k->literals, p3->literals) ||
		l_con.clause_ident(k->literals, p4->literals) ||
		l_con.clause_ident(k->literals, p5->literals) ||
		l_con.clause_ident(k->literals, p6->literals))
      found = true;
  }
  ClauseMisc::delete_clause(p1);
  ClauseMisc::delete_clause(p2);
  ClauseMisc::delete_clause(p3);
  ClauseMisc::delete_clause(p4);
  ClauseMisc::delete_clause(p5);
  ClauseMisc::delete_clause(p6);
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

    if (!contains_perm3(c, kept)) {
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

