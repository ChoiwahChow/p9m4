
#include <iomanip>

#include "../ladr/accanon.h"
#include "../ladr/clause_misc.h"
#include "../ladr/demod.h"
#include "../ladr/fatal.h"
#include "../ladr/index.h"
#include "../ladr/ioutil.h"
#include "../ladr/mindex.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "rewriter";

static constexpr char Help_string[] =
"\nThis program takes a file of demodulators (arg 1) and a\n"
"stream of terms (stdin).  Rewritten terms are sent to stdout.\n"
"The demodulators are not checked for termination.\n"
"For example,\n\n"
"   rewriter demods < terms.in > terms.out\n\n"
"The file of demodulators contains optional commands\n"
"then a list of demodulators.  The commands can be used to\n"
"declare infix operations and associativity/commutativity.\n"
"Example file of demodulators:\n\n"
"    op(400, infix, ^).\n"
"    op(400, infix, v).\n"
"    assoc_comm(^).\n"
"    assoc_comm(v).\n"
"    formulas(demodulators).\n"
"    x ^ x = x.\n"
"    x ^ (x v y) = x.\n"
"    x v x = x.\n"
"    x v (x ^ y) = x.\n"
"    end_of_list.\n\n";


int main(int argc, const char *argv[])
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  bool verbose = isu::which_member_arg(argc, argv, "verbose") > 0;

  TopInput topInput;
  topInput.init_standard_ladr();

  ifstream head_fp;
  head_fp.open(argv[1], ios::in);
  if (!head_fp)
    fatal::fatal_error("demodulator file can't be opened for reading");

  Term t = topInput.read_commands(head_fp, std::cerr, verbose, (int)unknown_actions::KILL_UNKNOWN);

  TermContainer   t_con;
  if (!t_con.is_term(t, "clauses", 1) && !t_con.is_term(t, "formulas", 1))
    fatal::fatal_error("formulas(demodulators) not found");

  /* Read list of demodulators. */

  Plist demodulators = Ioutil::read_clause_list(head_fp, std::cerr, true);

  head_fp.close();

  /* AC-canonicalize and index the demodulators. */
  SymbolContainer s_con;
  MindexContainer m_con;
  Mindex idx;
  if (s_con.assoc_comm_symbols() || s_con.comm_symbols())
    idx = m_con.mindex_init(Mindextype::DISCRIM_WILD, Uniftype::BACKTRACK_UNIF, 0);
  else
    idx = m_con.mindex_init(Mindextype::DISCRIM_BIND, Uniftype::ORDINARY_UNIF, 0);

  Parautil p_util;
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

  Ilist just;
  int rewritten = 0;
  ParseContainer  p_con;
  t = p_con.read_term(std::cin, std::cerr);

  while (t != nullptr) {
    rewritten++;
    if (verbose) {
      std::cout << "\nBefore:   ";
      p_con.fwrite_term_nl(std::cout, t);
    }

    if (s_con.assoc_comm_symbols())
    	Accnon::ac_canonical(t, -1);
    just = nullptr;
    t = Demod::demodulate(t, idx, &just, false);

    if (verbose)
      std::cout << "After:    ";

    p_con.fwrite_term_nl(std::cout, t);
    std::flush(std::cout);

    IlistContainer just_con;
    just_con.set_head(just);
    just_con.zap_ilist();
    t_con.zap_term(t);
    t = p_con.read_term(std::cin, std::cerr);
  }

  std::cout << "% " << PROGRAM_NAME << " " << argv[1] << ": rewrote " << rewritten << " terms with "
            << Demod::demod_rewrites() << " rewrite steps in " << std::setprecision(2) << myClock::user_seconds() << " seconds.\n";

  exit(0);

}

