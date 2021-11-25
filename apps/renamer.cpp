

#include "../ladr/fastparse.h"
#include "../ladr/fatal.h"
#include "../ladr/ioutil.h"
#include "../ladr/term.h"
#include "../ladr/topform.h"

#include "../VERSION_DATE.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "renamer";


static constexpr char Help_string[] =
"\nThis program takes a stream of clauses (from stdin) and\n"
"renumbers the variables.  Optional arguments:\n"
"    fast        clauses are read and written in fastparse form.\n"
"    commands    commands can be given and clauses are in a list.\n"
"                (not compatable with \"fast\")\n\n";


int main(int argc, const char **argv)
{
  if (isu::which_member_arg(argc, argv, "help") > 0 ||
      isu::which_member_arg(argc, argv, "-help") > 0) {
    std::cout << "\n" << PROGRAM_NAME <<", version " << PROGRAM_VERSION << ", " << PROGRAM_DATE << "\n";
    std::cout << Help_string;
    exit(1);
  }

  bool fast_parse = isu::which_member_arg(argc, argv, "fast") > 0;
  bool commands   = isu::which_member_arg(argc, argv, "commands") > 0;

  if (fast_parse && commands)
    fatal::fatal_error("renamer, fast_parse incompatable with commands");

  TopInput topInput;
  topInput.init_standard_ladr();

  if (fast_parse)
    Fastparse::fast_set_defaults(); /* Declare the symbols for fastparse. */
  else {
    AttributeContainer a_con;
    a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);
    a_con.register_attribute("answer", Attribute_type::TERM_ATTRIBUTE);
  }

  if (commands) {
    TermContainer   t_con;
    Term t = topInput.read_commands(std::cin, std::cout, false, (int)Unknown_type::IGNORE_UNKNOWN);
    if (!t || (!t_con.is_term(t, "clauses", 1) && !t_con.is_term(t, "formulas", 1)))
      fatal::fatal_error("renamer, bad command or list identifier");
  }

  TopformContainer tf_con;
  Topform c = Ioutil::clause_reader(fast_parse);
  while (c != nullptr) {
    tf_con.renumber_variables(c, 100);  /* fatal if too many variables */
    Ioutil::clause_writer(c, fast_parse);
    std::flush(std::cout);
    tf_con.zap_topform(c);
    c = Ioutil::clause_reader(fast_parse);
  }
  exit(0);
}

