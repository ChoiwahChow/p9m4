
#include <iomanip>

#include "../ladr/glist.h"
#include "../ladr/interp.h"
#include "../ladr/topform.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "interpfilter";

static constexpr char Help_string[] =

"\nThis program takes a file of clauses/formulas (arg 1), a test to\n"
"apply (arg 2), and a stream of interpretations (from stdin).\n"
"The tests are [all_true,some_true,all_false,some_false.  We print\n"
"(to stdout) the interpretations that satisfy the test.\n\n"
"For example, if the set of claues consists of associativity \n"
"and commutativity (AC), we can remove AC models by using the\n"
"some_false test.\n"
"For example,\n\n"
"    interpfilter clauses some_false < interps.in > interps.out\n\n";

/*************
 *
 *  find_clause(topforms, interp)
 *
 *  Return the first clause that has the requested value (true or false)
 *  in the interpretation.  Return nullptr on failure.
 *
 *************/

static
Topform find_clause(Plist topforms, Interp interp, bool requested_val)
{
  InterpContainer i_con;
  Plist p = topforms;
  while (p != nullptr) {
    bool val = i_con.eval_topform(static_cast<Topform>(p->v), interp);
    if (val == requested_val)
      return static_cast<Topform>(p->v);
    else
      p = p->next;
  }
  return nullptr;
}

enum {ALL_true, SOME_true, ALL_false, SOME_false};

int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  int operation = -1;
  if (isu::which_member_arg(argc, argv, "all_true") > 0)
    operation = ALL_true;
  else if (isu::which_member_arg(argc, argv, "some_true") > 0)
    operation = SOME_true;
  else if (isu::which_member_arg(argc, argv, "all_false") > 0)
    operation = ALL_false;
  else if (isu::which_member_arg(argc, argv, "some_false") > 0)
    operation = SOME_false;
  else
    fatal::fatal_error("interpfilter: operation should be {all,some}_{true,false}");

  ifstream clause_fp;
  clause_fp.open(argv[1], ios::in);
  if (!clause_fp)
    fatal::fatal_error("interpfilter: clause file cannot be opened for reading");

  TopInput topInput;
  topInput.init_standard_ladr();

  AttributeContainer a_con;
  a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);  /* ignore these */
  a_con.register_attribute("answer", Attribute_type::TERM_ATTRIBUTE);    /* ignore these */

  TermContainer   t_con;
  bool commands = isu::which_member_arg(argc, argv, "commands") > 0;
  if (commands) {
    Term t = topInput.read_commands(clause_fp, std::cerr, false, (int)unknown_actions::KILL_UNKNOWN);
    if (!t_con.is_term(t, "topforms", 1) && !t_con.is_term(t, "formulas", 1))
      fatal::fatal_error("interpfilter: formulas(...) not found");
  }

  Plist topforms = Ioutil::read_clause_or_formula_list(clause_fp, std::cerr);
  clause_fp.close();

  /* Print each interpretation on stdin that satisfies the query. */

  ParseContainer  p_con;
  Term t = p_con.read_term(std::cin, std::cerr);

  InterpContainer i_con;
  unsigned long int checked = 0;
  unsigned long int passed = 0;
  while (t != nullptr) {
    checked++;
    Interp interp = i_con.compile_interp(t, false);
    Topform c = find_clause(topforms, interp, operation == SOME_true || operation == ALL_false);

    if ((c  && (operation == SOME_true || operation == SOME_false)) ||
	    (!c && (operation == ALL_true  || operation == ALL_false ))) {
      passed++;
      i_con.fprint_interp_standard(std::cout, interp);
    }
    i_con.zap_interp(interp);
    t_con.zap_term(t);
    t = p_con.read_term(std::cin, std::cerr);
  }

  std::cout << "% " << PROGRAM_NAME << " " << argv[1] << " " << argv[2] << ": checked " << checked << ", passed " << passed
            << ", user " << std::setprecision(2) << myClock::user_seconds() << ".\n";

  exit(passed > 0 ? 0 : 1);
}

