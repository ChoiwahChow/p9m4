
#include <iomanip>

#include "../ladr/clause_eval.h"
#include "../ladr/ioutil.h"
#include "../ladr/top_input.h"
#include "../ladr/weight.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "test_clause_eval";

int main(int argc, const char **argv)
{
  TopInput topInput;
  topInput.init_standard_ladr();
  Weight::init_weight(nullptr, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0);

  AttributeContainer a_con;
  a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);  /* ignore these */
  a_con.register_attribute("answer", Attribute_type::TERM_ATTRIBUTE);    /* ignore these */

  ifstream rule_fp;
  rule_fp.open(argv[1], ios::in);
  if (!rule_fp)
    fatal::fatal_error("test_clause_eval, rule file cannot be opened for reading");

  ParseContainer  p_con;
  Term t = p_con.read_term(rule_fp, std::cerr);  /* get first rule */

  Plist rules = nullptr;
  PlistContainer  rules_con;
  rules_con.set_head(rules);

  TermContainer        t_con;
  ClauseEvalContainer  ev_con;
  while (t != nullptr) {
    Clause_eval compiled_rule = ev_con.compile_clause_eval_rule(t);
    rules = rules_con.plist_append(compiled_rule);
    p_con.fwrite_term_nl(std::cout, t);
    t_con.zap_term(t);
    t = p_con.read_term(rule_fp, std::cerr);
  }
  rule_fp.close();

  /* Evaluate each clause on std::cin. */

  Topform c = Ioutil::read_clause(std::cin, std::cerr);
  TopformContainer tf_con;
  while (c != nullptr && !Ioutil::end_of_list_clause(c)) {
    c->weight = Weight::clause_weight(c->literals);
    for (Plist p = rules; p; p = p->next) {
      Clause_eval rule = static_cast<Clause_eval>(p->v);
      bool result = ev_con.eval_clause_in_rule(c, rule);
      std::cout << result << "  (wt=" << std::setprecision(3) << c->weight << ") : ";
      Ioutil::fwrite_clause(std::cout, c, (int)Clause_print_format::CL_FORM_BARE);
    }
    std::cout << "\n";

    tf_con.zap_topform(c);
    c = Ioutil::read_clause(std::cin, std::cerr);
  }

  exit(0);
}
