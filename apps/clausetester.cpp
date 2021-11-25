
#include <iomanip>

#include "../ladr/interp.h"
#include "../ladr/ioutil.h"
#include "../ladr/term.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "clausetester";

static constexpr char Help_string[] =
"\nThis program takes a file of interpretations (arg 1) and a stream of\n"
"clauses (stdin).  For each clause, it tells interpretations in which\n"
"the clause is true.\n\n"
"For example,\n\n"
"    clausetester interps < clauses.in\n\n";


int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  ifstream interp_fp;
  interp_fp.open(argv[1], ios::in);
  if (!interp_fp)
    fatal::fatal_error("interpretation file cannot be opened for reading");

  TopInput topInput;
  topInput.init_standard_ladr();

  AttributeContainer a_con;
  a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);  /* ignore these */
  a_con.register_attribute("answer", Attribute_type::TERM_ATTRIBUTE);    /* ignore these */

  ParseContainer   p_con;
  TermContainer    t_con;
  Term t = p_con.read_term(interp_fp, std::cerr);  /* get first interpretation */

  int icount = 0;
  Plist interps = nullptr;
  PlistContainer  interps_con;
  interps_con.set_head(interps);
  InterpContainer i_con;
  while (t != nullptr) {
    icount++;
    Interp interp = i_con.compile_interp(t, false);
    interps = interps_con.plist_append(interp);
    t_con.zap_term(t);
    t = p_con.read_term(interp_fp, std::cerr);
  }
  interp_fp.close();

  int* counters = new int[icount](); // initialize to zero

  bool commands = isu::which_member_arg(argc, argv, "commands") > 0;
  if (commands) {
    t = topInput.read_commands(std::cin, std::cout, false, (int)unknown_actions::KILL_UNKNOWN);
    if (!t_con.is_term(t, "clauses", 1) && !t_con.is_term(t, "formulas", 1))
      fatal::fatal_error("formulas(...) not found");
  }

  /* Evaluate each clause on stdin. */

  Topform c = Ioutil::read_clause_or_formula(std::cin, std::cerr);

  TopformContainer tf_con;
  int ccount = 0;
  while (c != nullptr && !Ioutil::end_of_list_clause(c)) {

    Plist p = interps;
    Term t = tf_con.topform_to_term(c);
    ccount++;
    p_con.fwrite_term(std::cout, t);
    t_con.zap_term(t);
    std::cout << ".  %";

    int i = 0;
    while (p != nullptr) {
      if (i_con.eval_topform(c, static_cast<Interp>(p->v))) {  /* works also for non-clauses */
        counters[i]++;
        std::cout << " " << std::setw(2) << i+1;
      }
      i++;
      p = p->next;
    }
    std::cout << "\n";
    std::flush(std::cout);
    tf_con.zap_topform(c);
    c = Ioutil::read_clause_or_formula(std::cin, std::cerr);
  }

  for (int i = 0; i < icount; i++)
    std::cout << "% interp " << i+1 << " models " << counters[i] << " of " << ccount << " clauses.\n";

  exit(0);
}

