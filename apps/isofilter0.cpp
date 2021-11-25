

/* Take a stream of interpretations and remove the isomorphic ones.
 */

#include <fstream>
#include <iomanip>

#include "../ladr/clock.h"
#include "../ladr/fatal.h"
#include "../ladr/interp.h"
#include "../ladr/ioutil.h"
#include "../ladr/mystring.h"
#include "../ladr/parse.h"
#include "../ladr/term.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "isofilter0";

static constexpr char Help_string[] =
"\nThis program reads a stream of interpretations (from stdin) and removes\n"
"isomorphic ones.\n\n"
"Interpretations look like this:\n\n"
"interpretation(2, [\n"
"        function(A, [1]),\n"
"        function(e(_,_), [1,0,0,1]),\n"
"        relation(P(_), [0,1])]).\n\n"
"Argument \"ignore_constants\" is accepted.\n"
"Argument \"wrap\" is accepted.\n"
"Argument \"check '<operations>'\" is accepted.\n"
"Argument \"output '<operations>'\" is accepted.\n";


static bool iso_member(Interp a, Plist interps)
{
  InterpContainer ipc;
  if (interps == nullptr)
    return false;
  else if (ipc.isomorphic_interps(a, static_cast<Interp>(interps->v), true))
    return true;
  else
    return iso_member(a, interps->next);
}


int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  TopInput topInput;
  topInput.init_standard_ladr();

  ParseContainer pc;
  pc.simple_parse(true);

  bool ignore_constants = false;
  if (isu::which_member_arg(argc, argv, "ignore_constants") > 0 ||
      isu::which_member_arg(argc, argv, "-ignore_constants") > 0)
    ignore_constants = true;

  Plist check_strings = nullptr;
  int rc = isu::which_member_arg(argc, argv, "check");
  if (rc == -1)
    rc = isu::which_member_arg(argc, argv, "-check");
  if (rc > 0) {
    if (rc+1 >= argc)
      fatal::fatal_error("isofilter: missing \"check\" argument");
    else
      check_strings = pc.split_string(argv[rc+1]);
  }

  Plist output_strings = nullptr;
  rc = isu::which_member_arg(argc, argv, "output");
  if (rc == -1)
    rc = isu::which_member_arg(argc, argv, "-output");
  if (rc > 0) {
    if (rc+1 >= argc)
    	fatal::fatal_error("isofilter: missing \"output\" argument");
    else
      output_strings = pc.split_string(argv[rc+1]);
  }

  bool wrap = false;  /* surround output with list(interpretations) */
  if (isu::which_member_arg(argc, argv, "wrap") > 0 ||
      isu::which_member_arg(argc, argv, "-wrap") > 0)
    wrap = true;

  /* Input is a stream of interpretations. */

  if (wrap)
    std::cout << "isofilter: list(interpretations).\n\n";

  Term t = pc.read_term(std::cin, std::cerr);

  Plist interps = nullptr;
  int interps_read = 0;
  int interps_kept = 0;

  InterpContainer ipc;
  TermContainer   tc;
  while (t != nullptr) {
    interps_read++;

    if (ignore_constants)
      ipc.interp_remove_constants(t);  /* constants not checked or output */

    Term twork = tc.copy_term(t);

    if (check_strings)
      ipc.interp_remove_others(twork, check_strings);

    Interp a = ipc.compile_interp(twork, false);
    Interp c = ipc.normal_interp(a);
    if (iso_member(c, interps))
      ipc.zap_interp(c);
    else {
      /* print the original interp */

      if (output_strings)
        ipc.interp_remove_others(t, output_strings);

      Interp b = ipc.compile_interp(t, false);
      ipc.fprint_interp_standard2(std::cout, b);
      flush(std::cout);
      ipc.zap_interp(b);

      PlistContainer plc;
      plc.set_head(interps);
      interps = plc.plist_append(c);  /* keep the normal interp */
      interps_kept++;
    }
    ipc.zap_interp(a);
    tc.zap_term(t);
    tc.zap_term(twork);
    t = pc.read_term(std::cin, std::cerr);

    if (interps_read % 1000 == 0)
      std::cerr << PROGRAM_NAME << ": " << interps_read << " interps read, " << interps_kept << " kept\n";
  }

  std::cout << "% " << PROGRAM_NAME;
  for(int i = 1; i < argc; i++)
    std::cout << " " << argv[i];

  std::cout << ": input=" << interps_read << ", kept=" << interps_kept << ", ";
  std::cout << "checks=" << ipc.iso_checks() << ", perms=" << ipc.iso_perms() << ", ";
  std::cout << std::setprecision(2) << myClock::user_seconds() << " seconds.\n";

  if (wrap)
    std::cout << "\nend_of_list.\n";

  exit(0);
}  /* main */

