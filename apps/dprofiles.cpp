
/* Take a stream of interpretations and shows the profile.
 */

#include "../ladr/fatal.h"
#include "../ladr/interp.h"
#include "../ladr/ioutil.h"
#include "../ladr/parse.h"
#include "../ladr/top_input.h"
#include "../ladr/term.h"

#include "inputs_util.h"


static constexpr char PROGRAM_NAME[] = "profiles";

static constexpr char Help_string[] =
"\nThis program reads a stream of interpretations (from std::cin)\n"
"and shows the profiles."
"Argument \"discrim '<filename>'\" is accepted.\n";


int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  TopInput topInput;
  topInput.init_standard_ladr();

  Plist discriminators = nullptr;
  int rc = isu::which_member_arg(argc, argv, "discrim");
  if (rc == -1)
    rc = isu::which_member_arg(argc, argv, "-discrim");
  if (rc > 0) {
    if (rc+1 >= argc)
      fatal::fatal_error("isofilter: missing \"discrim\" argument");
    else {
      std::ifstream fp;
      fp.open(argv[rc+1], ios::in);
      if (!fp)
        fatal::fatal_error("discrim file cannot be opened for reading");
      discriminators = Ioutil::read_clause_list(fp, std::cerr, true);
    }
  }

  /* Input is a stream of interpretations. */
  TermContainer   t_con;
  InterpContainer i_con;
  ParseContainer  p_con;
  Term t = p_con.read_term(std::cin, std::cerr);

  while (t != nullptr) {
    Interp a = i_con.compile_interp(t, false);
    Interp c = i_con.normal3_interp(a, discriminators);
    i_con.p_interp_profile(c, discriminators);
    i_con.zap_interp(a);
    i_con.zap_interp(c);
    t_con.zap_term(t);
    t = p_con.read_term(std::cin, std::cerr);
  }

  exit(0);
}

