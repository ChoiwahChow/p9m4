
/* Take a stream of interpretations, and for each, print the upper-covers.
 */

#include "../ladr/interp.h"
#include "../ladr/ioutil.h"
#include "../ladr/term.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "upper-covers";

static constexpr char Help_string[] =
"\nThis program reads a stream of lattices (from stdin) and\n"
"for each one, prints its upper-covers.  Each lattice is\n"
"assumed to be in \"portable format\" as produced by mace4,\n"
"and have a binary operation (meet) named \"^\", \"m\", or \"meet\"\n"
"(it can have other operations as well).\n";


#define I2(n,i,j)     ((i) * (n) + (j))


static bool adjacent(int n, int i, int j, bool *lt)
{
  for (int k = 0; k < n; k++)
    if (lt[I2(n,i,k)] && lt[I2(n,k,j)])
      return false;
  return true;
}


static void upper_covers(Interp p, int lattice_number)
{
  InterpContainer i_con;
  int* a = i_con.interp_table(p, "^", 2);
  if (a == nullptr) {
    a = i_con.interp_table(p, "meet", 2);
    if (a == nullptr) {
      a = i_con.interp_table(p, "m", 2);
      if (a == nullptr)
        fatal::fatal_error("upper_covers: meet not found");
    }
  }

  /* Construct less-than relation; initialize upper-covers to less-than. */
  int n = i_con.interp_size(p);
  bool *lt = new bool[n*n];   //malloc(n * n * sizeof(bool *));
  bool *uc = new bool[n*n];   //malloc(n * n * sizeof(bool *));
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) {
      lt[I2(n,i,j)] = (a[I2(n,i,j)] == i && i != j);
      uc[I2(n,i,j)] = lt[I2(n,i,j)];
    }

  /* Remove entries from uc if something is between the elements. */

  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) {
      if (lt[I2(n,i,j)] && !adjacent(n, i, j, lt))
        uc[I2(n,i,j)] = false;
    }

  /* Print upper-covers. */

  std::cout << "% UPPER-COVERS of Lattice " << lattice_number << "\n";
  std::cout << "(\n";
  for (int i = 0; i < n; i++) {
    bool started = false;
    std::cout << "(" << i << " (";
    for (int j = 0; j < n; j++) {
      if (uc[I2(n,i,j)]) {
        std::cout << (started ? " " : "") << i;
        started = true;
      }
    }
    std::cout << "))\n";
  }
  std::cout << ")\n";
  std::cout << "% end of upper-covers\n";

  delete lt;
  delete uc;
}


int main(int argc, const char **argv)
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  TopInput topInput;
  topInput.init_standard_ladr();

  ParseContainer  p_con;
  /* Note that we do not read any commands. */
  /* terms(interpretations) and end_of_list are optional. */
  Term t = p_con.read_term(std::cin, std::cerr);

  TermContainer   t_con;
  if (t_con.is_term(t, "terms", 1))
    t = p_con.read_term(std::cin, std::cerr);

  int interps_read = 0;
  InterpContainer i_con;
  while (t != nullptr && !Ioutil::end_of_list_term(t)) {
    Interp a = i_con.compile_interp(t, false);
    interps_read++;

    upper_covers(a, interps_read);

    i_con.zap_interp(a);
    t_con.zap_term(t);
    t = p_con.read_term(std::cin, std::cerr);
  }

  std::cout << "% upper-covers: translated=" << interps_read << "\n";

  exit(0);
}

