
#include <iomanip>

#include "../ladr/fatal.h"
#include "../ladr/top_input.h"
#include "../ladr/interp.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "sigtest";

static constexpr char Help_string[] =
"\nHelp\n";


int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  ifstream clause_fp;
  clause_fp.open(argv[1], ios::in);
  if (!clause_fp)
    fatal::fatal_error("sigtest: clause file cannot be opened for reading");

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
      fatal::fatal_error("sigtest: formulas(...) not found");
  }

  Plist topforms = Ioutil::read_clause_or_formula_list(clause_fp, std::cerr);
  clause_fp.close();

  /* Print each interpretation on stdin that satisfies the query. */

  InterpContainer i_con;
  ParseContainer  p_con;
  Term t = p_con.read_term(std::cin, std::cerr);
  int checked = 0;
  while (t != nullptr) {
    checked++;
    Interp interp = i_con.compile_interp(t, false);

    /* std::cout << "Interp" << checked << ": ";*/
    int i = 0;
    for (Plist p = topforms; p; p = p->next, i++) {
      Topform c = static_cast<Topform>(p->v);
      int n = i_con.eval_literals_false_instances(c->literals, interp);
      std::cout << " c" << i << ": " << setw(3) << n << ",";
    }
    std::cout << "\n";
    i_con.zap_interp(interp);
    t_con. zap_term(t);
    t = p_con.read_term(std::cin, std::cerr);
  }

  exit(0);
}
