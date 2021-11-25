
#include <iomanip>

#include "../ladr/clock.h"
#include "../ladr/fatal.h"
#include "../ladr/ioutil.h"
#include "../ladr/interp.h"
#include "../ladr/order.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "clausefilter";

static constexpr char Help_string[] =
"\nThis program takes a file of interpretations (arg 1) and a stream of\n"
"formulas (stdin).  Formulas that pass the given test (arg 2) are sent\n"
"to stdout. The tests are true_in_all, true_in_some, false_in_all,\n"
"false_in_some.\n"
"For example,\n\n"
"   clausefilter interps true_in_all < clauses.in > clauses.out\n\n";


static OrderType interp_compare(Interp a, Interp b)
{
  InterpContainer i_con;
  if (i_con.interp_size(a) < i_con.interp_size(b))
    return OrderType::LESS_THAN;
  else if (i_con.interp_size(a) > i_con.interp_size(b))
    return OrderType::GREATER_THAN;
  else
    return OrderType::SAME_AS;
}


static
void sort_interps(Plist interps)
{
  PlistContainer interps_list;
  interps_list.set_head(interps);
  int n = interps_list.plist_count();
  void **a = new void*[n];

  int i = 0;
  for (Plist p = interps; p; p = p->next)
    a[i++] = p->v;
  myOrder::merge_sort(a, n, (OrderType (*)(void*,void*)) interp_compare);
  i = 0;
  for (Plist p = interps; p; p = p->next)
    p->v = a[i++];
}

/*************
 *
 *   find_interp(clause, list-of-interpretations, models-flag)
 *
 *   Look for an interpretation that models (or doesn't model) a given clause.
 *   That is, return the first interpretation in which the clause is true
 *   (not true); return nullptr if the clause is false (true) in all the
 *   interpretations.
 *
 *************/

Interp find_interp(Topform c, Plist interps, bool models, bool check_evaluable)
{
  Plist p = interps;
  InterpContainer i_con;

  while (p != nullptr) {
    if (check_evaluable && !i_con.evaluable_topform(c, static_cast<Interp>(p->v)))
      ;  /* skip this evaluation */
    else {
      bool value = i_con.eval_topform(c, static_cast<Interp>(p->v));  /* works also for non-clauses */
      if ((models && value) || (!models && !value))
        return static_cast<Interp>(p->v);
    }
    p = p->next;
  }
  return nullptr;
}

enum {true_IN_ALL, true_IN_SOME, false_IN_ALL, false_IN_SOME};

int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  bool ignore_nonevaluable = isu::which_member_arg(argc, argv, "ignore_nonevaluable") > 0;

  int operation = -1;
  if (isu::which_member_arg(argc, argv, "true_in_all") > 0)
    operation = true_IN_ALL;
  else if (isu::which_member_arg(argc, argv, "true_in_some") > 0)
    operation = true_IN_SOME;
  else if (isu::which_member_arg(argc, argv, "false_in_all") > 0)
    operation = false_IN_ALL;
  else if (isu::which_member_arg(argc, argv, "false_in_some") > 0)
    operation = false_IN_SOME;
  else
    fatal::fatal_error("clausefilter, need argument {true,false}_in_{all,some}");

  TopInput topInput;
  topInput.init_standard_ladr();

  AttributeContainer a_con;
  a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);  /* ignore these */
  a_con.register_attribute("answer", Attribute_type::TERM_ATTRIBUTE);    /* ignore these */

  ifstream interp_fp;
  interp_fp.open(argv[1], ios::in);
  if (!interp_fp)
    fatal::fatal_error("clausefilter, interp file cannot be opened for reading");

  TermContainer   t_con;
  ParseContainer  p_con;
  Term t = p_con.read_term(interp_fp, std::cerr);  /* get first interpretation */

  Plist interps = nullptr;
  PlistContainer  interps_con;
  interps_con.set_head(interps);
  InterpContainer i_con;
  while (t != nullptr) {
    Interp interp = i_con.compile_interp(t, false);
    interps = interps_con.plist_append(interp);
    t_con.zap_term(t);
    t = p_con.read_term(interp_fp, std::cerr);
  }
  interp_fp.close();

  sort_interps(interps);  /* puts smallest ones first */

  bool commands = isu::which_member_arg(argc, argv, "commands") > 0;
  if (commands) {
    t = topInput.read_commands(std::cin, std::cout, false, (int)unknown_actions::KILL_UNKNOWN);
    if (!t_con.is_term(t, "clauses", 1) && !t_con.is_term(t, "formulas", 1))
      fatal::fatal_error("formulas(...) not found");
  }

  /* Evaluate each formula/clause on stdin. */

  Topform c = Ioutil::read_clause_or_formula(std::cin, std::cerr);
  TopformContainer tf_con;
  unsigned long int checked = 0;
  unsigned long int passed = 0;
  while (c != nullptr && !Ioutil::end_of_list_clause(c)) {

    checked++;

    Interp interp = find_interp(c, interps, operation == true_IN_SOME || operation == false_IN_ALL,
                                ignore_nonevaluable);

    if ((interp && (operation==true_IN_SOME || operation==false_IN_SOME)) ||
        (!interp && (operation==true_IN_ALL || operation==false_IN_ALL))) {
      passed++;
      Ioutil::fwrite_clause(std::cout, c, (int)Clause_print_format::CL_FORM_BARE);  /* ok for nonclausal formulas */
    }
    tf_con.zap_topform(c);
    c = Ioutil::read_clause_or_formula(std::cin, std::cerr);
  }

  std::cout << "% " << PROGRAM_NAME << " " << argv[1] << " " << argv[2] << ": checked " << checked << ", passed " << passed
            << ", user " << std::setprecision(2) << myClock::user_seconds() << ".\n";

  exit(passed > 0 ? 0 : 1);
}

