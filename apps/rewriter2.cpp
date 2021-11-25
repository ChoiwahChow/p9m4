
/*
 * This program is probably in-complete.  It is not part of original 11A Prover9 apps.src/Makefile
 */

#include "../ladr/dollar.h"
#include "../ladr/ioutil.h"
#include "../ladr/parse.h"
#include "../ladr/term.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "rewriter2";

static char Help_string[] =
"\nThis program takes a file of rewrite rules (arg 1) and a\n"
"stream of terms (stdin).  Rewritten terms are sent to std::cout.\n"
"The rewrite rules are not checked for termination.\n"
"For example,\n\n"
"   rewriter demods < terms.in > terms.out\n\n"
"The file of demodulators contains optional commands\n"
"then a list of demodulators.  The commands can be used to\n"
"declare infix operations.\n"
"Example file of rewrite rules:\n\n"
"    op(400, infix, ^).\n"
"    op(400, infix, v).\n"
"    formulas(demodulators).\n"
"    end_of_list.\n\n";

int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  bool verbose = isu::which_member_arg(argc, argv, "verbose") > 0;

  TopInput topInput;
  topInput.init_standard_ladr();
  Dollar::init_dollar_eval();

  ifstream head_fp;
  head_fp.open(argv[1], ios::in);
  if (!head_fp)
    fatal::fatal_error("demodulator file can't be opened for reading");

  Term t = topInput.read_commands(head_fp, std::cerr, verbose, (int)unknown_actions::KILL_UNKNOWN);

  TermContainer   t_con;
  if (!t_con.is_term(t, "list", 1))
    fatal::fatal_error("list(rewrite_rules) not found");

  /* Read list of rules. */

  Plist rules = Ioutil::read_term_list(head_fp, std::cerr);

  head_fp.close();

  ParseContainer  p_con;
  for (Plist p = rules; p != nullptr; p = p->next) {
    Term rule = static_cast<Term>(p->v);
    if (t_con.is_term(rule, "=", 2) || t_con.is_term(rule, "<->", 2) ||
        (t_con.is_term(rule, "->", 2) && t_con.is_term(ARG(rule,1), "=", 2))) {
      t_con.term_set_variables(rule, MAX_VARS);
    }
    else {
      std::cout << "bad rewrite rule: ";
      p_con.fwrite_term_nl(std::cout, rule);
      std::cerr << "bad rewrite rule: ";
      p_con.fwrite_term_nl(std::cerr, rule);
      fatal::fatal_error("bad rewrite rule");
    }
  }

  if (verbose)
	  Ioutil::fwrite_term_list(std::cout, rules, "rewrite_rules");

  /* Read and demodulate terms. */

  t = p_con.read_term(std::cin, std::cerr);

  while (t != nullptr) {
    if (verbose) {
      std::cout << "\nBefore:   ";
      p_con.fwrite_term_nl(std::cout, t);
    }

    t = programmed_rewrite(t, rules);

    if (verbose)
    	std::cout << "After:    ";

    p_con.fwrite_term_nl(std::cout, t);
    std::flush(std::cout);

    t_con.zap_term(t);
    t = p_con.read_term(std::cin, std::cerr);
  }

  exit(0);

}  /* main */

