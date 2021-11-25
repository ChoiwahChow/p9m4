

#include "../ladr/fastparse.h"
#include "../ladr/ioutil.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "unfast";

static constexpr char Help_string[] =
"\nThis program takes a stream of fastparse terms and writes them\n"
"as ordinary terms.  The following symbols/arities are recognized.\n"
"arity 2: [=mjfd+*/]; arity 1: [cgi-~']; variables are r--z.  Other symbols\n"
"are constants.  Input should be one term per line, terminated\n"
"with a period, without spaces.  Lines starting with % are echoed.\n\n";


int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  TopInput topInput;
  topInput.init_standard_ladr();

  Fastparse::fast_set_defaults();

  TopformContainer tf_con;
  Topform c = Fastparse::fast_read_clause(std::cin, std::cerr);
  while (c != nullptr) {
    Ioutil::fwrite_clause(std::cout, c, (int)Clause_print_format::CL_FORM_BARE);
    std::flush(std::cout);
    tf_con.zap_topform(c);
    c = Fastparse::fast_read_clause(std::cin, std::cerr);
  }
  exit(0);
}
