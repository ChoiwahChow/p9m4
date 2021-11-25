
#include "../ladr/cnf.h"
#include "../ladr/formula.h"
#include "../ladr/ioutil.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "miniscope";

int main(int argc, const char **argv)
{
  TopInput topInput;
  topInput.init_standard_ladr();

  Plist formulas = Ioutil::read_formula_list(std::cin, std::cerr);

  FormulaContainer f_con;
  Formula f = f_con.formulas_to_conjunction(formulas);

  std::cout << "\n------------------------\n";

  std::cout << "\nbefore (denial): ";
  f_con.p_formula(f);

  f = f_con.nnf(f_con.negate(f));
  std::cout << "\nnnf (positive):    ";
  f_con.p_formula(f);

  Cnf  cnf;
  f = cnf.miniscope_formula(f, 0);

  if (f->type == Ftype::AND_FORM) {
    std::cout << "result is " << f->arity << "subproblems\n";
    for (int i = 0; i < f->arity; i++) {
      std::cout << "\nproblem " << i+1 << ":  ";
      f_con.p_formula(f_con.negate(f->kids[i]));
    }
  }
  else {
    std::cout << "\nresult indivisible\n\n";
    f_con.p_formula(f_con.negate(f));
  }

  std::cout << "exiting " << PROGRAM_NAME << ">\n";
  exit(0);
}
