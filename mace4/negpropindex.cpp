

#include "../ladr/fatal.h"
#include "../ladr/parse.h"
#include "ground.h"
#include "negpropindex.h"

negpropindex::negpropindex(const Symbol_data s, int d, const Symbol_data* Sn) : Symbols(s), Domain_size(d), Sn_to_mace_sn(Sn), Index(nullptr)
{
}

void*
negpropindex::init_recurse(int n)
{
  if (n == 0)
    return nullptr;
  else {
    void **p = new void*[Domain_size+1];
    for (int i = 0; i < Domain_size+1; i++)
      p[i] = init_recurse(n-1);  /* assign a void** to a void* */
    return p;
  }
}

void
negpropindex::init_negprop_index()
{
  int num_syms = 0;
  for(Symbol_data p = Symbols; p != nullptr; p = p->next)
    num_syms++;

  Index = (void****) new void*[2];

  for (int sign = 0; sign < 2; sign++) {
    Index[sign] = (void***) new void*[num_syms];
    Symbol_data p = Symbols;
    for (int sym = 0; sym < num_syms; sym++, p = p->next) {
      /* Do nothing if this is the equality symbol. */
      /* Do nothing if this is a relation symbol and we are negative. */
      if (p->attribute != EQUALITY_SYMBOL &&
          !(p->type == type_RELATION && sign == 0)) {

        int range_size = (p->type == type_FUNCTION ? Domain_size : 2);
        Index[sign][sym] = new void*[range_size];

        for (int val = 0; val < range_size; val++) {
          Index[sign][sym][val] = init_recurse(p->arity);
        }
      }
    }
  }
}

void
negpropindex::free_recurse(int n, void **p)
{
  if (n == 0)
    return;
  else {
    for (int i = 0; i < Domain_size+1; i++)
      free_recurse(n-1, (void**)p[i]);
    delete p;
  }
}

negpropindex::~negpropindex()
{
  if (Index == nullptr)
    return;

  int num_syms = 0;
  for(Symbol_data p = Symbols; p != nullptr; p = p->next)
    num_syms++;

  for (int sign = 0; sign < 2; sign++) {
    Symbol_data p = Symbols;
    for (int sym = 0; sym < num_syms; sym++, p = p->next) {
      /* Do nothing if this is the equality symbol. */
      /* Do nothing if this is a relation symbol and we are negative. */
      if (p->attribute != EQUALITY_SYMBOL &&
          !(p->type == type_RELATION && sign == 0)) {

        int range_size = (p->type == type_FUNCTION ? Domain_size : 2);

        for (int val = 0; val < range_size; val++) {
          free_recurse(p->arity, (void**)Index[sign][sym][val]);
        }
        delete Index[sign][sym];
      }
    }
    delete Index[sign];
  }
  delete Index;
  Index = nullptr;
}

void
negpropindex::p_recurse(void **p, int x, int n, int depth)
{
  for (int j = 0; j < depth; j++)
    std::cout << "    ";
  std::cout << "[" << x << "] " << p;
  if (n == 0) {
    if (p != nullptr) {
      ParseContainer pc;
      for (Term t = (Term) p; t != nullptr; t = static_cast<Term>(t->u.vp)) {
        std::cout << " : ";
        pc.fwrite_term(std::cout, t);
      }
    }
    std::cout << "\n";
  }
  else {
    std::cout << "\n";
    for (int i = 0; i < Domain_size+1; i++) {
      p_recurse((void**)p[i], i, n-1, depth+1);
    }
  }
}

void
negpropindex::p_negprop_index()
{
  int num_syms = 0;
  for(Symbol_data p = Symbols; p != nullptr; p = p->next)
    num_syms++;

  for (int sign = 0; sign < 2; sign++) {
    Symbol_data p = Symbols;
    for (int sym = 0; sym < num_syms; sym++, p = p->next) {
      /* Do nothing if this is the equality symbol. */
      /* Do nothing if this is a relation symbol and we are negative. */
      if (p->attribute != EQUALITY_SYMBOL &&
          !(p->type == type_RELATION && sign == 0)) {

        int range_size = (p->type == type_FUNCTION ? Domain_size : 2);
        SymbolContainer sym_con;
        for (int val = 0; val < range_size; val++) {
          std::cout << (sign == 0 ? "~" : "+") << " " << sym_con.sn_to_str(p->sn) << " val=" << val << "\n";
          p_recurse((void**)Index[sign][sym][val], -1, p->arity, 1);
        }
      }
    }
  }
}

void
negpropindex::insert_recurse(void **p, Term atom, Term t, int n, Mstate state, EstackContainer& es_con)
{
  Term arg = ARG(t,n);
  /* If the argument is not a domain element, then use the
     subtree "Domain_size", which is 1 more than the maximum
     domain element.  Note that many terms correspond to each leaf.
  */
  int i = (VARIABLE(arg) ? VARNUM(arg) : Domain_size);
  if (ARITY(t) == n+1) {
    /* We are at a leaf.  Insert the atom into the list.
       If this is an eterm, then the list will never have more
       than one member.
    */
    if (atom->u.vp != NULL)
      fatal::fatal_error("insert_recurse: atom link in use");
    state->stack = es_con.update_and_push((void **) &(atom->u.vp), p[i], state->stack);
    state->stack = es_con.update_and_push((void **) &(p[i]), atom, state->stack);
  }
  else
    insert_recurse((void**)p[i], atom, t, n+1, state, es_con);
}

void
negpropindex::insert_negprop_eq(Term atom, Term alpha, int val, Mstate state, EstackContainer& es_con,
                                const Mace4GlobalValues& mace4_gv)
{
  int sign = (mace4_gv.NEGATED(atom) ? 0 : 1);
  int sym = Sn_to_mace_sn[SYMNUM(alpha)]->mace_sn;

  if (ARITY(alpha) == 0)
    /* This could be handled if we need it.  The assignments are
       atom->u.p = Index[sign][sym][val];
       Index[sign][sym][val] = atom;
     */
    fatal::fatal_error("insert_negprop_eq, arity 0");
  else
    insert_recurse((void**)Index[sign][sym][val], atom, alpha, 0, state, es_con);
}

void
negpropindex::insert_negprop_noneq(Term atom, Mstate state, EstackContainer& es_con,
                                   const Mace4GlobalValues& mace4_gv)
{
  int val = (mace4_gv.NEGATED(atom) ? 0 : 1);
  int sym = Sn_to_mace_sn[SYMNUM(atom)]->mace_sn;

  if (ARITY(atom) == 0)
    /* This could be handled if we need it.  The assignments are
       atom->u.p = Index[1][sym][val];
       Index[1][sym][val] = atom;
     */
    fatal::fatal_error("insert_negprop_noneq, arity 0");
  else
    insert_recurse((void**)Index[1][sym][val], atom, atom, 0, state, es_con);
}

Term
negpropindex::negprop_find_near_recurse(void **p, Term query, int pos, int n)
{
  int i = (pos == 0 ? Domain_size : VARNUM(ARG(query, n)));
  if (ARITY(query) == n+1)
    return static_cast<Term>(p[i]);
  else
    return negprop_find_near_recurse((void**)p[i], query, pos-1, n+1);
}

Term
negpropindex::negprop_find_near(int sign, int sym, int val, Term query, int pos)
{
  return negprop_find_near_recurse((void**)Index[sign][sym][val], query, pos, 0);
}
