

#include "../ladr/fatal.h"
#include "../ladr/flatdemod.h"
#include "../ladr/flatterm.h"
#include "../ladr/mindex.h"
#include "../ladr/mystring.h"
#include "../ladr/term.h"
#include "../ladr/top_input.h"
#include "../ladr/unify.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "genterms";

// static char Help_string[] = "";

static Mindex Demod_index;  /* demodulator index */

static unsigned Generated = 0;
static unsigned Kept      = 0;

#define N0 4   /* 4 constants */
#define N1 1   /* 1 unary op */
#define N2 3   /* 3 binary ops */

struct syms {
  int s0[N0], c0[N0];
  int s1[N1], c1[N1];
  int s2[N2], c2[N2];
};


static
Plist catalan(int n)
{
  TermContainer   t_con;
  if (n == 1) {
    Term t = t_con.get_rigid_term("_", 0);
    Plist head = nullptr;
    PlistContainer  head_con;
    head_con.set_head(head);
    return head_con.plist_append(t);
  }
  else {
    Plist results = nullptr;  /* collect results */
    PlistContainer  results_con;
    results_con.set_head(results);

    for (int i = 1, j = n-1; i < n; i++, j--) {
      Plist left  = catalan(i);
      Plist right = catalan(j);
      for (Plist l = left; l; l = l->next) {
        for (Plist r = right; r; r = r->next) {
          Term c = t_con.build_binary_term_safe("o", t_con.copy_term(static_cast<Term>(l->v)), t_con.copy_term(static_cast<Term>(r->v)));
          results = results_con.plist_prepend(c);
        }
      }
      t_con.zap_plist_of_terms(left);
      t_con.zap_plist_of_terms(right);
    }
    return results_con.reverse_plist();
  }
}


static
bool rewritable_top(Flatterm f)
{
  if (ARITY(f) == 0)
    return false;
  else {
    UnifyContainer u_con;
    Context subst = u_con.get_context();
    Discrim_pos dpos;
    void* t = Flatdemod::discrim_flat_retrieve_first(f, Demod_index->discrim_tree, subst, &dpos);
    if (t)
      Flatdemod::discrim_flat_cancel(dpos);
    u_con.free_context(subst);
    return t != nullptr;
  }
}


static
bool rewritable(Flatterm head)
{
  for (Flatterm f = head; f != head->end->next; f = f->next) {
    if (rewritable_top(f))
      return true;
  }
  return false;
}


static
void candidate(Flatterm f)
{
  Generated++;
  if (!rewritable(f)) {
    Kept++;
    FlattermContainer  ft_con;
    ft_con.print_flatterm(f);
    std::cout << " = C.\n";
    std::flush(std::cout);
  }
}


static
void genterms(Flatterm f, Flatterm head, struct syms s)
{
  if (ARITY(f) == 0) {
    for (int i = 0; i < N0; i++) {
      if (s.c0[i] > 0) {
        s.c0[i]--;
        f->private_symbol = -s.s0[i];
        if (f->next == nullptr)
          candidate(head);
        else
          genterms(f->next, head, s);
        s.c0[i]++;
      }
    }
  }
  else if (ARITY(f) == 1) {
    for (int i = 0; i < N1; i++) {
      if (s.c1[i] > 0) {
        s.c1[i]--;
        f->private_symbol = -s.s1[i];
        genterms(f->next, head, s);
        s.c1[i]++;
      }
    }
  }
  else if (ARITY(f) == 2) {
    for (int i = 0; i < N2; i++) {
      if (s.c2[i] > 0) {
        s.c2[i]--;
        f->private_symbol = -s.s2[i];
        genterms(f->next, head, s);
        s.c2[i]++;
      }
    }
  }
  else
    fatal::fatal_error("genterms, bad arity");
}


static
int num_constants(struct syms s)
{
  int n = 0;
  for (int i = 0; i < N0; i++)
    n += s.c0[i];
  return n;
}


static
int num_binaries(struct syms s)
{
  int n = 0;
  for (int i = 0; i < N2; i++)
    n += s.c2[i];
  return n;
}


static
void check_counts(struct syms s)
{
  if (num_constants(s) != num_binaries(s) + 1)
    fatal::fatal_error("check_counts, number of constants and binaries do not match");
}


static
int unary_occurrences(struct syms s)
{
  int n = 0;
  for (int i = 0; i < N1; i++)
    n += s.c1[i];
  return n;
}


static
void insert_unaries(Flatterm f, int n, struct syms s)
{
  if (n == 0) {
    Flatterm x = f;
    while (x->prev)
      x = x->prev;
    /* std::out << "genterms: "; FlattermContainer ft_con; ft_con.p_flatterm(x); */
    genterms(x, x, s);
  }
  else {
    FlattermContainer  ft_con;
    Flatterm u = ft_con.get_flatterm();
    u->arity = 1;
    SymbolContainer s_con;
    u->private_symbol = -s_con.str_to_sn("unary", 1);
    u->end = f->end; u->next = f; u->prev = f->prev;
    if (f->prev)
      f->prev->next = u;
    f->prev = u;
    insert_unaries(f, n-1, s);
    if (u->prev)
      u->prev->next = f;
    f->prev = u->prev;
    ft_con.free_flatterm(u);
    if (f->next)
      insert_unaries(f->next, n, s);
  }
}


static
void unary_gen(Term t, struct syms s)
{
  FlattermContainer  ft_con;
  Flatterm f = ft_con.term_to_flatterm(t);
  insert_unaries(f, unary_occurrences(s), s);
  ft_con.zap_flatterm(f);
}


static
int lookfor(const char *s, int argc, const char **argv)
{
  int n;
  int i = isu::which_member_arg(argc, argv, s);
  if (i == -1)
    return 0;
  else if (argc > i+1 && myString::str_to_int(argv[i+1], &n))
    return n;
  else {
    fatal::fatal_error("lookfor: bad arg list");
    return 0;
  }
}


int main(int argc, const char **argv)
{
  struct syms s;
  Plist demods;

  TopInput topInput;
  topInput.init_standard_ladr();

  AttributeContainer a_con;
  a_con.register_attribute("label",  Attribute_type::STRING_ATTRIBUTE);  /* ignore these */

  SymbolContainer s_con;
  int n = lookfor("-A",  argc, argv);  s.s0[0] = s_con.str_to_sn("A",  0);  s.c0[0] = n;
  n = lookfor("-E",  argc, argv);  s.s0[1] = s_con.str_to_sn("E",  0);  s.c0[1] = n;
  n = lookfor("-P1", argc, argv);  s.s0[2] = s_con.str_to_sn("P1", 0);  s.c0[2] = n;
  n = lookfor("-P2", argc, argv);  s.s0[3] = s_con.str_to_sn("P2", 0);  s.c0[3] = n;

  n = lookfor("-K",  argc, argv);  s.s1[0] = s_con.str_to_sn("K",  1);  s.c1[0] = n;

  n = lookfor("-a",  argc, argv);  s.s2[0] = s_con.str_to_sn("a",  2);  s.c2[0] = n;
  n = lookfor("-p",  argc, argv);  s.s2[1] = s_con.str_to_sn("p",  2);  s.c2[1] = n;
  n = lookfor("-f",  argc, argv);  s.s2[2] = s_con.str_to_sn("f",  2);  s.c2[2] = n;

  check_counts(s);

  MindexContainer m_con;
  Demod_index = m_con.mindex_init(Mindextype::DISCRIM_BIND, Uniftype::ORDINARY_UNIF, 0);

  int i = isu::which_member_arg(argc, argv, "-demod");

  if (i == -1)
    demods = nullptr;
  else {
    ifstream head_fp;
    head_fp.open(argv[1], ios::in);
    if (!head_fp)
      fatal::fatal_error("demod file cannot be opened for reading");
    demods = Ioutil::read_clause_list(head_fp, std::cerr, true);
    head_fp.close();

    Parautil p_util;
    for (Plist p = demods; p != nullptr; p = p->next) {
      /* assume positive equality unit */
      Topform d = static_cast<Topform>(p->v);
      Literals lit = d->literals;
      Term alpha = lit->atom->args[0];
      p_util.mark_oriented_eq(lit->atom);     /* do not check for termination */
      m_con.mindex_update(Demod_index, alpha, Indexop::INSERT);
    }
  }

  Plist forms = catalan(num_constants(s));

  n = 0;
  Plist p;
  for (p = forms; p; p = p->next) {
    n++;
    Term t = static_cast<Term>(p->v);
    unary_gen(t, s);
  }
  std::cout << "% Generated=" << Generated << ", Kept=" << Kept << ".\n";
  exit(0);
}
