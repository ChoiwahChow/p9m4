
#include <iomanip>

#include "../ladr/complex.h"
#include "../ladr/parse.h"
#include "../ladr/term.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "test_complex";

int main(int argc, const char **argv)
{
  TopInput topInput;
  topInput.init_standard_ladr();

  ParseContainer  p_con;
  /* Evaluate each clause on stdin. */
  Term t = p_con.read_term(std::cin, std::cerr);

  TermContainer   t_con;
  double total1 = 0;
  double total2 = 0;
  double total3 = 0;
  double total4 = 0;
  double total5 = 0;
  int n = 0;
  while (t != nullptr) {
    int size = t_con.symbol_count(t);
    t_con.term_set_variables(t, MAX_VARS);
    n++;
    double x1 = 1 - Complex::term_complexity(t, 2, 0);
    double x2 = 1 - Complex::term_complexity(t, 3, 1);
    double x3 = 1 - Complex::term_complexity(t, 3, 2);
    double x4 = 1 - Complex::term_complexity(t, 3, 3);
    double x5 = 1 - Complex::complex4(t);
    total1 += x1;
    total2 += x2;
    total3 += x3;
    total4 += x4;
    total5 += x5;
    std::cout << std::setw(2) << size << "  " << std::setprecision(3) << x1 << "  " << std::setprecision(3) << x2
    		<< "  " << std::setprecision(3) << x3 << "  " << std::setprecision(3) << x4 << "  " << std::setprecision(3) << x5 << "     ";
    t_con.p_term(t);
    t_con.zap_term(t);
    t = p_con.read_term(std::cin, std::cerr);
  }
  std::cout << "average complexities " << std::setprecision(3) << total1 / n << "  "
            << std::setprecision(3) << total2 / n << "  " << std::setprecision(3) << total3 / n << "  "
            << std::setprecision(3) << total4 / n << "  " << std::setprecision(3) << total5 / n << "\n";

  exit(0);
}
