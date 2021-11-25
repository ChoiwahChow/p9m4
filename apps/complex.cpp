
#include <iomanip>

#include "../ladr/top_input.h"
#include "../ladr/term.h"
#include "../VERSION_DATE.h"

static constexpr char PROGRAM_NAME[] = "complex";


static
void complex4_compare(Term a, Term b, int* n)
{
  //std::cout << "compare:  "; TermContainer  t_con; t_con.p_term(a);
  //std::cout << "       :  "; t_con.p_term(b);
  if (VARIABLE(a) && VARIABLE(b)) {
    if (VARNUM(a) == VARNUM(b))
      (*n)++;
  }
  else if (!VARIABLE(a) && !VARIABLE(b)) {
    if (SYMNUM(a) == SYMNUM(b)) {
      (*n)++;
      for (int i = 0; i < ARITY(a); i++)
        complex4_compare(ARG(a,i), ARG(b,i), n);
    }
  }
}


static
bool complex4_p2(Term s, Term t, int* n)
{
  if (s == t)
    return false;
  else {
    bool go = true;
    for (int i = 0, go = true; i < ARITY(t) && go; i++)
      go = complex4_p2(s, ARG(t,i), n);
    complex4_compare(s, t, n);
    return go;
  }
}


static
void complex4_p1(Term s, Term t, int* n)
{
  for (int i = 0; i < ARITY(s); i++)
    complex4_p1(ARG(s,i), t, n);
  complex4_p2(s, t, n);
}


static
double complex4(Term t)
{
  TermContainer  t_con;
  int size = t_con.symbol_count(t);

  int n = 0;
  complex4_p1(t, t, &n);

  double d = ((double) n) / (size*size);
  if (d >= 1.0)
    d = 0.999;

  std::cout << "size= " << std::setw(2) << size << " n= " << std::setw(3) << n << "  d2=" << std::setprecision(3) << d << "  ";

  t_con.p_term(t);

  return d;
}


int main(int argc, const char **argv)
{
  TopInput topInput;
  topInput.init_standard_ladr();

  /* Evaluate each clause on stdin. */

  ParseContainer p_con;
  Term t = p_con.read_term(std::cin, std::cerr);

  TermContainer   t_con;
  while (t) {
    /* term_set_variables(t, MAX_VARS); */
    double d = complex4(t);
    t_con.zap_term(t);
    t = p_con.read_term(std::cin, std::cerr);
  }
  exit(0);
}

