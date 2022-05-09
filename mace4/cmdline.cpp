
#include "../ladr/banner.h"
#include "../ladr/glist.h"
#include "../ladr/ladrvglobais.h"
#include "../ladr/top_input.h"

#include "../VERSION_DATE.h"

#include "cmdline.h"



bool
CmdLine::member_args(int argc, char **argv, const std::string& str)
{
  for (int i = 1; i < argc; i++)
    if (str == argv[i])
      return true;
  return false;
}

void
CmdLine::command_line_parm(int id, char *optarg)
{
  int n = atoi(optarg);
  LADR_GLOBAL_OPTIONS.assign_parm(id, n, true);
  std::cout << "\n% From the command line: assign(" << LADR_GLOBAL_OPTIONS.parm_id_to_str(id) << ", " << n << ").\n";
}

void
CmdLine::command_line_flag(int id, char *optarg)
{
  if (!optarg || strcmp(optarg, "1") == 0) {
    LADR_GLOBAL_OPTIONS.set_flag(id, true);
    std::cout << "\n% From the command line: set(" << LADR_GLOBAL_OPTIONS.flag_id_to_str(id) << ").\n";
  }
  else if (strcmp(optarg, "0") == 0) {
    LADR_GLOBAL_OPTIONS.clear_flag(id, true);
    std::cout << "\n% From the command line: clear(" << LADR_GLOBAL_OPTIONS.flag_id_to_str(id) << ").\n";
  }
  else
    fatal::fatal_error("Command-line flag must have 0 or 1 as the value");
}

void
CmdLine::process_command_line_args(int argc, char **argv, Mace_options opt)
{
  extern char *optarg;
  int c;

  /* No colons:  no argument.
     One colon:  argument required.
     Two colons: argument optional.  (GNU extension!  Don't use it!)
  */
  while ((c = getopt(argc, argv,
         "n:N:m:t:s:b:O:M:p:P:A:C:d:v:L:G:H:I:J:K:T:R:i:q:Q:S:cf:g")) != EOF) {
    switch (c) {
    case 'n':
      command_line_parm(opt->domain_size, optarg);
      break;
    case 'N':
      command_line_parm(opt->end_size, optarg);
      break;
    case 'm':
      command_line_parm(opt->max_models, optarg);
      break;
    case 't':
      command_line_parm(opt->max_seconds, optarg);
      break;
    case 's':
      command_line_parm(opt->max_seconds_per, optarg);
      break;
    case 'b':
      command_line_parm(opt->max_megs, optarg);
      break;
    case 'O':
      command_line_parm(opt->selection_order, optarg);
      break;
    case 'M':
      command_line_parm(opt->selection_measure, optarg);
      break;

    case 'A':
      command_line_flag(opt->print_models_interp, optarg);
      break;
    case 'C':
      command_line_parm(opt->print_cubes, optarg);
      break;
    case 'P':
      command_line_flag(opt->print_models, optarg);
      break;
    case 'p':
      command_line_flag(opt->print_models_tabular, optarg);
      break;
    case 'd':
      command_line_parm(opt->cubes_options, optarg);
      std::cout << "debug cubes options as read in ************************* " << opt->cubes_options << std::endl;
      break;
#if 0 /* Prolog-style variables cannot be set from the command line. */
    case 'V':
      command_line_flag(prolog_style_variables_id(), optarg);
      break;
#endif
    case 'v':
      command_line_flag(opt->verbose, optarg);
      break;
    case 'L':
      command_line_flag(opt->lnh, optarg);
      break;
    case 'G':
      command_line_flag(opt->negprop, optarg);
      break;
    case 'H':
      command_line_flag(opt->neg_assign, optarg);
      break;
    case 'I':
      command_line_flag(opt->neg_assign_near, optarg);
      break;
    case 'J':
      command_line_flag(opt->neg_elim, optarg);
      break;
    case 'K':
      command_line_flag(opt->neg_elim_near, optarg);
      break;
    case 'T':
      command_line_flag(opt->trace, optarg);
      break;
    case 'R':
      command_line_flag(opt->integer_ring, optarg);
      break;
    case 'i':
      command_line_parm(opt->increment, optarg);
      break;
    case 'q':
      command_line_flag(opt->iterate_primes, optarg);
      break;
    case 'Q':
      command_line_flag(opt->iterate_nonprimes, optarg);
      break;
    case 'S':
      command_line_flag(opt->skolems_last, optarg);
      break;
    case 'c':  /* prover compatability mode */
    case 'f':  /* input files */
    case 'g':  /* tptp syntax */
      /* do nothing---these are handled elsewhere */
      break;

    case '?':
    default:
      usage_message(std::cerr, opt);
      fatal::fatal_error("unrecognized command-line option or missing value (flags take 0 or 1)");
    }
  }
}

void
CmdLine::usage_message(std::ostream& fp, Mace_options opt)
{
  fp << CmdLine::PROGRAM_NAME << " " << PROGRAM_VERSION << " (" << PROGRAM_DATE << ") -- Search for finite models.\n"
     << "\n"
     << "Input commands, clauses, and formulas are taken from standard input.\n"
     << "\n"
     << "Command-line options override any settings in the input file.\n"
     << "To set or clear a flag, you must give 1 or 0 as the value.\n"
     << "\n"
     << "Basic Options\n"
     << "\n"
     << "  -n n : (lower case) parm domain_size (" << LADR_GLOBAL_OPTIONS.parm(opt->domain_size) << ").\n"
     << "  -N n : (upper case) parm end_size (" << LADR_GLOBAL_OPTIONS.parm(opt->iterate_up_to) << ").\n"
     << "  -i n : (lower case) parm increment (" << LADR_GLOBAL_OPTIONS.parm(opt->increment) << ").\n"
     << "  -A n : (upper case) flag print_models_interp (" << (LADR_GLOBAL_OPTIONS.flag(opt->print_models_interp) ? "set" : "clear") << ").\n"
     << "  -C n : (upper case) parm print_cubes (" << LADR_GLOBAL_OPTIONS.parm(opt->print_cubes) << ").\n"
     << "  -P n : (upper case) flag print_models (" << (LADR_GLOBAL_OPTIONS.flag(opt->print_models) ? "set" : "clear") << ").\n"
     << "  -p n : (lower case) flag print_models_tabular (" << (LADR_GLOBAL_OPTIONS.flag(opt->print_models_tabular) ? "set" : "clear") << ").\n"
     << "  -m n : parm max_models (" << LADR_GLOBAL_OPTIONS.parm(opt->max_models) << ").\n"
     << "  -t n : parm max_seconds (" << LADR_GLOBAL_OPTIONS.parm(opt->max_seconds) << ").\n"
     << "  -s n : parm max_seconds_per (" << LADR_GLOBAL_OPTIONS.parm(opt->max_seconds_per) << ").\n"
     << "  -b n : parm max_megs (" << LADR_GLOBAL_OPTIONS.parm(opt->max_megs) << ").\n"
     << "  -V n : (upper case) flag prolog_style_variables (" << (LADR_GLOBAL_OPTIONS.flag(LADR_GLOBAL_STD_OPTIONS.prolog_style_variables_id()) ? "set" : "clear") << ").\n"
     << "  -v n : (lower case) flag verbose (" << (LADR_GLOBAL_OPTIONS.flag(opt->verbose) ? "set" : "clear") << ").\n"
     << "\n"
     << "Advanced Options\n"
     << "\n"
     << "  -L n : flag lnh (" << (LADR_GLOBAL_OPTIONS.flag(opt->lnh) ? "set" : "clear") << ").\n"
     << "  -O n : parm selection_order (" << LADR_GLOBAL_OPTIONS.parm(opt->selection_order) << ").\n"
     << "  -M n : parm Selection_measure (" << LADR_GLOBAL_OPTIONS.parm(opt->selection_measure) << ").\n"
     << "  -G n : flag negprop (" << (LADR_GLOBAL_OPTIONS.flag(opt->negprop) ? "set" : "clear") << ").\n"
     << "  -H n : flag neg_assign (" << (LADR_GLOBAL_OPTIONS.flag(opt->neg_assign) ? "set" : "clear") << ").\n"
     << "  -I n : flag neg_assign_near (" << (LADR_GLOBAL_OPTIONS.flag(opt->neg_assign_near) ? "set" : "clear") << ").\n"
     << "  -J n : flag neg_elim (" << (LADR_GLOBAL_OPTIONS.flag(opt->neg_elim) ? "set" : "clear") << ").\n"
     << "  -K n : flag neg_elim_near (" << (LADR_GLOBAL_OPTIONS.flag(opt->neg_elim_near) ? "set" : "clear") << ").\n"
     << "  -T n : flag trace (" << (LADR_GLOBAL_OPTIONS.flag(opt->trace) ? "set" : "clear") << ").\n"
     << "  -R n : flag integer_ring (" <<  (LADR_GLOBAL_OPTIONS.flag(opt->integer_ring) ? "set" : "clear") << ").\n"
     << "  -q n : flag iterate_primes (" <<  (LADR_GLOBAL_OPTIONS.flag(opt->iterate_primes) ? "set" : "clear") << ").\n"
     << "  -S n : flag skolems_last (" <<  (LADR_GLOBAL_OPTIONS.flag(opt->skolems_last) ? "set" : "clear") << ").\n"
     << "\n"
     << "Special Flags (not corresponding to set/clear/assign commands)\n"
     << "\n"
     << "  -c   : Ignore unrecognized set/clear/assign commands in the input\n"
     << "         file.  This is useful for running MACE4 on an input file\n"
     << "         designed for another program such as a theorem prover.\n"
     << "\n";
}

Plist
CmdLine::process_distinct_terms(Plist distinct)
{
/* TODO [choiwah] complete documentation
 * It is taking a list of terms, and returns a list of Formula's, each from a different term.
 */
  Plist forms = nullptr;
  ListtermContainer L;
  TermContainer T;
  SymbolContainer S;
  FormulaContainer F;
  PlistContainer FORMS;
  for (Plist p = distinct; p; p = p->next) {
    Term list = static_cast<Term>(p->v);
    if (!L.proper_listterm(list))
      fatal::fatal_error("process_distinct_terms: lists must be proper, e.g., [a,b,c].\n");
    while (!L.nil_term(list)) {
      Term a = ARG(list,0);
      Term rest = ARG(list,1);
      while (!L.nil_term(rest)) {
        Term b = ARG(rest,0);
        Term neq = T.build_unary_term_safe(S.not_sym(),
                                           T.build_binary_term_safe(S.eq_sym(), T.copy_term(a), T.copy_term(b)));
        Formula f = F.term_to_formula(neq);
        T.zap_term(neq);
        FORMS.set_head(forms);
        forms = FORMS.plist_append(f);
        rest = ARG(rest, 1);
      }
      list = ARG(list, 1);
    }
  }
  return forms;
}

Plist
CmdLine::read_mace4_input(int argc, char **argv, bool allow_unknown_things, Mace_options opt)
{
  /*
   * Reads the inputs, and returns a Plist of Topform's
   * Term's are first converted to Formula's.  Then all Formula's are converted to Topform's.
   * wild_terms and hints are not used and are ignored in Mace4.
   */

  // Tell the top_input package what lists to accept and where to put them.

  // Accept hints, but they will not be used.
  Plist hints = nullptr;
  LADR_GLOBAL_TOP_INPUT.accept_list("hints", (int)Top_input_type::FORMULAS, true, &hints);

  // Accept goals; these are negated individually (each must be falsified)
  Plist goals = nullptr;
  LADR_GLOBAL_TOP_INPUT.accept_list("goals", (int)Top_input_type::FORMULAS, false, &goals);

  // Accept lists of distinct items
  Plist distinct_lists = nullptr;
  LADR_GLOBAL_TOP_INPUT.accept_list("distinct", (int)Top_input_type::TERMS, false, &distinct_lists);

  // Accept any other clauses and formulas.  Each must be true.
  Plist wild_formulas = nullptr;
  LADR_GLOBAL_TOP_INPUT.accept_list("",    (int)Top_input_type::FORMULAS, false, &wild_formulas);

  // Accept any terms.  These will not be used.
  Plist wild_terms = nullptr;
  LADR_GLOBAL_TOP_INPUT.accept_list("",      (int)Top_input_type::TERMS,    false, &wild_terms);

  // Read commands such as set, clear, op, lex.
  // Read lists, filling in variables given to the accept_list calls.

  banner::print_separator(std::cout, "INPUT", true);

  LADR_GLOBAL_TOP_INPUT.read_all_input(argc, argv, std::cout, true,
                                      (int)(allow_unknown_things ? Unknown_type::WARN_UNKNOWN : Unknown_type::KILL_UNKNOWN));

  if (wild_terms)
    std::cout << "%   term list(s) ignored\n";
  if (hints)
    std::cout << "%   hints list(s) ignored\n";

  process_command_line_args(argc, argv, opt);

  banner::print_separator(std::cout, "end of input", true);

  if (!LADR_GLOBAL_OPTIONS.option_dependencies_state()) {
    /* This might be needed in the future. */
    std::cout << "\n% Enabling option dependencies (ignore applies only on input).\n";
    LADR_GLOBAL_OPTIONS.enable_option_dependencies();
  }

  Plist distinct_forms = process_distinct_terms(distinct_lists);
  PlistContainer  p0;
  p0.set_head(wild_formulas);
  wild_formulas = p0.plist_cat(distinct_forms);

  wild_formulas = LADR_GLOBAL_TOP_INPUT.embed_formulas_in_topforms(wild_formulas, true);
  goals = LADR_GLOBAL_TOP_INPUT.embed_formulas_in_topforms(goals, false);

  // Clausify

  banner::print_separator(std::cout, "PROCESS NON-CLAUSAL FORMULAS", true);
  std::cout << "\n% Formulas that are not ordinary clauses:\n";

  wild_formulas = LADR_GLOBAL_TOP_INPUT.process_input_formulas(wild_formulas, true);
  goals = LADR_GLOBAL_TOP_INPUT.process_goal_formulas(goals, true);  /* negates goals */

  banner::print_separator(std::cout, "end of process non-clausal formulas", true);

  p0.set_head(wild_formulas);
  wild_formulas = p0.plist_cat(goals);

  return wild_formulas;
}

