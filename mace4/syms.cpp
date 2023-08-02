
#include "../ladr/fatal.h"
#include "../ladr/memory.h"
#include "../ladr/glist.h"
#include "../ladr/interp.h"
#include "../ladr/mystring.h"
#include "../ladr/symbols.h"
#include "../ladr/topform.h"
#include "syms.h"


std::string&
Symbol_dataContainer::get_op_symbol(int sn) {
  SymbolContainer  sym_con;
  Symbol symbol = sym_con.lookup_by_id(sn);
  return *(symbol->name);
}

Symbol_data
Symbol_dataContainer::get_symbol_data(void)
{
  Symbol_data p = static_cast<Symbol_data>(Memory::memNew(sizeof(symbol_data)));

  p->sn = 0;
  p->mace_sn = 0;
  p->arity = 0;
  p->type = 0;
  p->base = 0;
  p->attribute = ORDINARY_SYMBOL;
  p->next = nullptr;
  return(p);
}

Symbol_data
Symbol_dataContainer::init_built_in_symbols(Symbol_data cur_ptr)
{
  // This function returns the "head pointer" of all symbob_data
  SymbolContainer  sym_con;
  Symbol_data s = get_symbol_data();
  s->sn = sym_con.str_to_sn(sym_con.eq_sym(), 2);
  s->arity = 2;
  s->type = type_RELATION;
  s->attribute = EQUALITY_SYMBOL;
  s->next = cur_ptr;
  return s;
}

Symbol_data
Symbol_dataContainer::insert_mace4_sym(Symbol_data syms, int sn, int type)
{
  // insert new symbol in "syms", in ascending order of sn.  So first ind the position to insert
  SymbolContainer  sym_con;
  if (syms == nullptr || sym_con.sn_to_lex_val(sn) < sym_con.sn_to_lex_val(syms->sn)) {
    Symbol_data new_sym = get_symbol_data();
    new_sym->sn = sn;
    new_sym->arity = sym_con.sn_to_arity(sn);
    new_sym->type = type;
    if (sym_con.is_skolem(sn))
      new_sym->attribute = SKOLEM_SYMBOL;
    new_sym->next = syms;
    return new_sym;
  }
  else {
    syms->next = insert_mace4_sym(syms->next, sn, type);
    return syms;
  }
}

int
Symbol_dataContainer::collect_mace4_syms(Plist clauses, bool arith_op, const arithmetic& arith, Symbol_data* Symbols)
{
  // *Symbols may be updated
  TopformContainer tf_con;
  Ilist fsyms = tf_con.fsym_set_in_topforms(clauses);
  Ilist rsyms = tf_con.rsym_set_in_topforms(clauses);

  SymbolContainer  sym_con;
  sym_con.lex_order(fsyms, rsyms, nullptr, nullptr, sym_con.lex_compare_arity_0123);

  int max_domain = -1;
  for (Ilist p = fsyms; p; p = p->next) {
    int sn = p->i;
    int dom = myString::natural_string(sym_con.sn_to_str(sn));
    if (dom >= 0)
      max_domain = std::max(dom, max_domain);
    else if (arith_op && arith.arith_op_sn(sn))
      ;  /* don't insert */
    else
      *Symbols = insert_mace4_sym(*Symbols, sn, type_FUNCTION);
  }

  for (Ilist p = rsyms; p; p = p->next) {
    int sn = p->i;
    int dom = myString::natural_string(sym_con.sn_to_str(sn));
    if (dom >= 0) {
      std::cout << "\nThe bad symbol is: " << sym_con.sn_to_str(sn) << "\n";
      fatal::fatal_error("collect_mace4_syms, relation symbol is domain element");
    }
    else if (sym_con.is_eq_symbol(sn) || (arith_op && arith.arith_rel_sn(sn)))
      ;  /* don't insert */
    else
      *Symbols = insert_mace4_sym(*Symbols, sn, type_RELATION);
  }

  IlistContainer f_il_con(fsyms);
  f_il_con.zap_ilist();
  IlistContainer r_il_con(rsyms);
  r_il_con.zap_ilist();

  return max_domain;
}

Symbol_data
Symbol_dataContainer::find_symbol_node(int id, Symbol_data Symbols)
{
  /* Assume bases are increasing and that the id is in range.
     Return node with the largest base <= the id. */
  Symbol_data prev = nullptr;
  Symbol_data curr = Symbols;
  while (curr != nullptr && curr->base <= id) {
    prev = curr;
    curr = curr->next;
  }
  return prev;
}

int
Symbol_dataContainer::max_index(int id, Symbol_data s, int Domain_size)
{
  /*   If the cell given by ID represents f(i,j,k,...), return the
   *   maximum of (i,j,k,...).  If it is a constant, return -1.
   */
  InterpContainer  ip_con;
  int max = -1;
  int n = Domain_size;
  int x = id - s->base;
  for (int i = s->arity - 1; i >= 0; i--) {
    int p = ip_con.int_power(n, i);
    int e = x / p;
    max = std::max(e, max);
    x = x % p;
  }
  return max;
}


