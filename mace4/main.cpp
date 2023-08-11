
#include <ostream>

#include "../ladr/banner.h"
#include "../ladr/ioutil.h"
#include "../ladr/ladrvglobais.h"
#include "../ladr/top_input.h"
#include "../VERSION_DATE.h"
#include "cmdline.h"
#include "mace4vglobais.h"
#include "mace4.h"


int main(int argc, char *argv[])
{
  std::cout << std::fixed;
  std::cerr << std::fixed;
  TopInput topInput;
  topInput.init_standard_ladr();

  // Construction of Macd4VGlobais includes init mace options
  Mace4VGlobais mace4vglobais;    /* We must do this before calling usage_message. */

  MACE4::init_attrs();

  if (CmdLine::member_args(argc, argv, "help") ||
      CmdLine::member_args(argc, argv, "-help")) {
      CmdLine::usage_message(std::cerr, mace4vglobais.Opt);
    exit(1);
  }
  Search  searcher(&mace4vglobais);
  MACE4::set_curr_search(&searcher);  //TODO: for thread-safety, change to setting thread-specific object

  banner::print_banner(argc, argv, CmdLine::PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_DATE, INSTITUTION, false);
  topInput.set_program_name(CmdLine::PROGRAM_NAME);   /* for conditional input */

  signal(SIGINT,  MACE4::mace4_sig_handler);
  signal(SIGUSR1, MACE4::mace4_sig_handler);
  signal(SIGSEGV, MACE4::mace4_sig_handler);

  /* Following says whether to ignore unrecognized set/clear/assigns. */
  bool prover_compatability_mode = CmdLine::member_args(argc, argv, "-c");
  Plist clauses = CmdLine::read_mace4_input(argc, argv, prover_compatability_mode, mace4vglobais.Opt, mace4vglobais.m_opts);

  banner::print_separator(std::cout, "CLAUSES FOR SEARCH", true);
  Ioutil::fwrite_clause_list(std::cout, clauses, "mace4_clauses", (int)Clause_print_format::CL_FORM_BARE);
  banner::print_separator(std::cout, "end of clauses for search", true);

  Mace_results results = searcher.mace4(clauses);

  MACE4::mace4_exit(results->return_code);  /* print messages and exit */

  exit(0);  /* won't happen */
}
