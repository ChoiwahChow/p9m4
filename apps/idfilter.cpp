
#include <iomanip>

#include "../ladr/accanon.h"
#include "../ladr/demod.h"
#include "../ladr/fatal.h"
#include "../ladr/glist.h"
#include "../ladr/fastparse.h"
#include "../ladr/index.h"
#include "../ladr/ioutil.h"
#include "../ladr/mindex.h"
#include "../ladr/term.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "idfilter";

static char Help_string[] =
"\nThis program takes a file of demodulators (arg 1) and a\n"
"stream of equations (stdin).  Each equation is rewritten.\n"
"If it rewrites to an instance of x=x, it is discarded;\n"
"otherwise it (in the unrewritten form) is send to stdout.\n"
"For example,\n\n"
"   idfilter demods < terms.in > terms.out\n\n"
"The file of demodulators contains optional commands\n"
"then a list of demodulators.  The commands can be used to\n"
"declare infix operations and associativity/commutativity.\n"
"Example file of demodulators:\n\n"
"    op(400, infix, ^).\n"
"    op(400, infix, v).\n"
"    assoc_comm(^).\n"
"    assoc_comm(v).\n"
"    formulas(demodulator).\n"
"    x ^ x = x.\n"
"    x ^ (x v y) = x.\n"
"    x v x = x.\n"
"    x v (x ^ y) = x.\n"
"    end_of_list.\n\n";

int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  bool fast_parse = isu::which_member_arg(argc, argv, "fast") > 0;
  bool backward = isu::which_member_arg(argc, argv, "x") > 0;
  bool verbose = isu::which_member_arg(argc, argv, "verbose") > 0;

  TopInput topInput;
  topInput.init_standard_ladr();

  if (fast_parse)
	  Fastparse::fast_set_defaults();  /* Declare the symbols for fastparse. */

  ifstream head_fp;
  head_fp.open(argv[1], ios::in);
  if (!head_fp)
    fatal::fatal_error("demodulator file can't be opened for reading");

  Term t = topInput.read_commands(head_fp, std::cerr, false, (int)unknown_actions::KILL_UNKNOWN);

  TermContainer   t_con;
  if (!t_con.is_term(t, "clauses", 1) && !t_con.is_term(t, "formulas", 1))
    fatal::fatal_error("formulas(demodulators) not found");

  Uniftype unification_type;
  SymbolContainer s_con;
  if (s_con.assoc_comm_symbols() || s_con.comm_symbols())
    unification_type = Uniftype::BACKTRACK_UNIF;
  else
    unification_type = Uniftype::ORDINARY_UNIF;

  /* Read list of demodulators. */

  Plist demodulators = Ioutil::read_clause_list(head_fp, std::cerr, true);

  head_fp.close();

  /* AC-canonicalize and index the demodulators. */

  Parautil p_util;
  MindexContainer m_con;
  Mindex idx = m_con.mindex_init(Mindextype::DISCRIM_WILD, unification_type, 0);
  for (Plist p = demodulators; p != nullptr; p = p->next) {
    /* assume positive equality unit */
    Topform d = static_cast<Topform>(p->v);
    Literals lit = d->literals;
    Term alpha = lit->atom->args[0];
    p_util.mark_oriented_eq(lit->atom);  /* don not check for termination */
    if (s_con.assoc_comm_symbols())
    	Accnon::ac_canonical(lit->atom, -1);
    m_con.mindex_update(idx, alpha, Indexop::INSERT);
  }

  if (verbose)
    Ioutil::fwrite_clause_list(std::cout, demodulators, "demodulators", (int)Clause_print_format::CL_FORM_BARE);

  /* Read and demodulate terms. */

  t = Ioutil::term_reader(fast_parse);
  Ilist just;
  int tested = 0;
  int passed = 0;
  ParseContainer  p_con;
  while (t != nullptr) {
    Term tcopy = t_con.copy_term(t);
    bool ident;
    tested++;
    if (verbose) {
      std::cout << "\nBefore:   ";
      p_con.fwrite_term_nl(std::cout, t);
    }

    if (s_con.assoc_comm_symbols())
      Accnon::ac_canonical(tcopy, -1);
    just = nullptr;
    tcopy = Demod::demodulate(tcopy, idx, &just, false);

    if (verbose) {
      std::cout << "After:    ";
      p_con.fwrite_term_nl(std::cout, tcopy);
    }

    ident = (t_con.is_term(t, "=", 2) && t_con.term_ident(ARG(tcopy,0),ARG(tcopy,1)));
    if ((ident && !backward) ||
	(!ident && backward)) {
      passed++;
      Ioutil::term_writer(t, fast_parse);
      std::flush(std::cout);
    }

    IlistContainer just_con;
    just_con.set_head(just);
    just_con.zap_ilist();

    t_con.zap_term(t);
    t_con.zap_term(tcopy);
    t = Ioutil::term_reader(fast_parse);
  }

  std::cout << "% " << PROGRAM_NAME << " " << argv[1] << " " << (backward ? "x" : "") << ": tested "
            << tested << ", passed " << passed << " in " << std::setprecision(2) << myClock::user_seconds() << " seconds.\n";
  exit(0);

}  /* main */

