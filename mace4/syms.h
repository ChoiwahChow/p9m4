
#ifndef MACE4_SYMS_H
#define MACE4_SYMS_H

#include "arithmetic.h"


/* Symbol types */
enum {
  type_FUNCTION,
  type_RELATION
};

/* Symbol attributes */
enum {
  ORDINARY_SYMBOL,
  EQUALITY_SYMBOL,
  SKOLEM_SYMBOL
};

typedef struct symbol_data* Symbol_data;

//TODO: [Choiwah] make this object one-for-each thread. Add free memory of Symbol, all default functions
//TODO: [Choiwah] perhaps add get functions to the private members
//TODO: [choiwah] reduce number of friends
class symbol_data {   /* SORT */
private:
  int sn;             /* ordinary symbol ID */
  int mace_sn;        /* MACE symbol ID */
  int arity;
  int type;           /* type_FUNCTION or type_RELATION */
  int base;
  int attribute;      /* ORDINARY_SYMBOL or EQUALITY_SYMBOL */
  Symbol_data next;

public:
  symbol_data(const symbol_data&) = delete;
  symbol_data& operator=(const symbol_data&) = delete;

  symbol_data() : sn(0), mace_sn(0), arity(0), type(0), base(0), attribute(ORDINARY_SYMBOL), next(nullptr) {};

  inline int get_type() const {return type;}

public:
  friend class Symbol_dataContainer;
  friend class CellContainer;
  friend class Ground;
  friend class negpropindex;
  friend class propagate;
  friend class Search;
};

class Symbol_dataContainer {
private:
  static Symbol_data get_symbol_data(void);

public:
  Symbol_dataContainer() = delete;
  Symbol_dataContainer(const Symbol_dataContainer&) = delete;
  Symbol_dataContainer& operator=(const Symbol_dataContainer&) = delete;

public:
  static Symbol_data insert_mace4_sym(Symbol_data syms, int sn, int type);
  static int         collect_mace4_syms(Plist clauses, bool arith_op, const arithmetic& arith, Symbol_data* Symbols);
  static Symbol_data find_symbol_node(int id, Symbol_data Symbols);
  static int         max_index(int id, Symbol_data s, int Domain_size);

  static Symbol_data init_built_in_symbols(Symbol_data cur_ptr);  // Starts the chain of symbols
};

#endif

