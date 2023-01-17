
#include <iomanip>

#include "../ladr/banner.h"
#include "../ladr/ladrvglobais.h"
#include "../ladr/memory.h"
#include "../ladr/symbols.h"

#include "msearch.h"
#include "select.h"
#include "smallprime.h"


/* Ground terms.  MACE4 operates on ground clauses, which are
   represented by the structure mclause.  Ground terms (and
   atoms) are represented with ordinary LADR terms.  There are
   a few tricks:

   (1) We use upward pointers from terms to superterms
   and from atoms to clauses.

   (2) We need to mark atoms with a termflag, so that we know
   when to stop when following the upward pointers.  Also,
   a termflag is used to indicate that an atom is negated.

   (3) Domain elements are represented by variables (sorry,
   but it is very convenient to do so).  Also, there is only
   one actual copy of each domain element (structure sharing
   of domain elements).  Global array *Domain contains them.

   IDs.  If all of the arguments of a term (including atoms) are
   domain elements, that term is called an eterm.  For example,
   f(3,4), a, P(0), and Q.

   Each eterm has a unique ID which is used as an index into
   the cell table, for example, when a new eterm is obtained by
   evaluating a subterm to a domain element, we have to quickly
   check if this new eterm can be evaluated.  We do this by
   calculating its ID and looking up in Cells[ID].value.
   And when we have a new assignment, say f(3,4)=2, we find the
   list of occurrences of f(3,4) by looking in Cells[ID].occurrences.
*/

long long Search::next_message = 1;
int Search::Next_report = 0;
std::string Search::interp_file_name("models.out");

Search::Search(Mace4VGlobais* g) : Mace4vglobais(g), Domain_size(0), Domain(nullptr), max_models_str("max_models"), all_models_str("all_models"),
  exhausted_str("exhausted"), max_megs_yes_str("max_megs_yes"), max_megs_no_str("max_megs_no"), max_sec_yes_str("max_sec_yes"),
  max_sec_no_str("max_sec_no"), mace_sigint_str("mace_sigint"), mace_sigsegv_str("mace_sigsegv"), unknown_str("???"), Skolems_last(false),
  Number_of_cells(0), Cells(nullptr), Ordered_cells(nullptr), First_skolem_cell(0), Max_domain_element_in_input(0),
  Symbols(nullptr), Sn_to_mace_sn(nullptr), Sn_map_size(0), Models(nullptr), Grounder(nullptr),
  Total_models(0), Start_domain_seconds(0), Start_seconds(0), Start_megs(0), propagator(nullptr), print_cubes(-2), cubes_options(0)
{
}

void
Search::initialize_for_search(Plist clauses) {
  mace4_clock.init_clock("Mace4");

  /* In ground clauses, VARIABLEs represent domain elements,
     so from here on, print variables as integers. */

  // set_variable_style(INTEGER_STYLE);

  mace4_gv.init_globals();

  /* Set up Symbols list. */
  Symbols = Symbol_dataContainer::init_built_in_symbols(Symbols);  /* =/2 (and maybe others) */

  /* Maybe initialize for arithmetic. */
  if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->arithmetic))
    Mace4vglobais->Arith->init_arithmetic(Domain, Domain_size, mace4_gv);

  Skolems_last = LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->skolems_last);

  /* Collect data for each symbol. */

  Max_domain_element_in_input = -1;
  int i = Symbol_dataContainer::collect_mace4_syms(clauses, LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->arithmetic), *Mace4vglobais->Arith, &Symbols);
  Max_domain_element_in_input = std::max(Max_domain_element_in_input, i);

  if (Max_domain_element_in_input == -1)
    std::cout << "\n% There are no natural numbers in the input.\n";
  else
    std::cout << "\n% The largest natural number in the input is " << Max_domain_element_in_input << ".\n";

  /* Set up map from ordinary symnums to mace symnums. */

  int max = 0;
  i = 0;

  for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
    s->mace_sn = i++;
    max = (s->sn > max ? s->sn : max);
  }

  Sn_map_size = max+1;

  Sn_to_mace_sn = new Symbol_data[Sn_map_size];

  for (int i = 0; i < Sn_map_size; i++)
    Sn_to_mace_sn[i] = nullptr;

  for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
    Sn_to_mace_sn[s->sn] = s;
  }
  print_cubes = LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->print_cubes);
  cubes_options = LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->cubes_options);
}


void
Search::init_for_domain_size(void)
{
  /*
   *   Given the list of (general) clauses, set up the various data
   *   structures that will be needed for a given domain size.
   */

  InterpContainer interp_con;

  /* Give each symbol its "base" value, which is used to index cells.  */

  int nextbase = 0;
  for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
    s->base = nextbase;
    nextbase += interp_con.int_power(Domain_size, s->arity);
  }

  /* Set up the array of domain terms.  All ground terms refer to these. */

  TermContainer term_con;
  Domain = new Term[Domain_size];
  for (int i = 0; i < Domain_size; i++)
    Domain[i] = term_con.get_variable_term(i);

  /* Set up the table of cells. */

  Number_of_cells = nextbase;
  Cells           = new cell[Number_of_cells];
  Ordered_cells   = new Cell[Number_of_cells];
  // std::cout << "debug @@@@@@@@@@@@@@@@@@@@@@@@@ Number_of_cells " << Number_of_cells << std::endl;
  delete Grounder;
  Grounder = new Ground(Domain_size, Domain, Sn_to_mace_sn, Cells, &Mstats, Mace4vglobais->Arith, &mace4_gv);
  propagator = new propagate(Symbols, Domain_size, Domain, Cells, Sn_to_mace_sn, &Mstats,
                             &MScon, &EScon, Grounder, &mace4_gv, Mace4vglobais);

  for (int id = 0; id < Number_of_cells; id++) {
    struct cell *c = Cells + id;
    int n;
    c->id = id;
    c->occurrences = nullptr;
    c->value = nullptr;
    c->symbol = Symbol_dataContainer::find_symbol_node(id, Symbols);
    c->eterm = propagator->decode_eterm_id(id);
    c->max_index = Symbol_dataContainer::max_index(id, c->symbol, Domain_size);
    n = CellContainer::id_to_domain_size(id, Cells, Domain_size);
    c->possible = new Term[n];
    for (int j = 0; j < n; j++)
      c->possible[j] = Domain[j];  /* really just a flag */
  }

  CellContainer::order_cells(LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->verbose), Cells, Number_of_cells, Skolems_last, Ordered_cells);
}

void
Search::built_in_assignments(void)
{
  for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
    if (s->attribute == EQUALITY_SYMBOL) {
      int i, j;
      for (int i = 0; i < Domain_size; i++)
        for (j = 0; j < Domain_size; j++)
          Cells[X2(s->base,i,j)].value = (Domain[i==j ? 1 : 0]);
    }
  }
}

void
Search::special_assignments(void)
{
  SymbolContainer S;
  if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->integer_ring)) {
    /* Fix [+,-,*] as the ring of integers mod domain_size. */
    /* If any of those operations doesn't exist, then ignore it.*/
    for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
      if (S.is_symbol(s->sn, "+", 2)) {
    for (int i = 0; i < Domain_size; i++)
      for (int j = 0; j < Domain_size; j++)
        Cells[X2(s->base,i,j)].value = Domain[(i + j) % Domain_size];
      }
      else if (S.is_symbol(s->sn, "*", 2)) {
        for (int i = 0; i < Domain_size; i++)
          for (int j = 0; j < Domain_size; j++)
            Cells[X2(s->base,i,j)].value = Domain[(i * j) % Domain_size];
      }
      else if (S.is_symbol(s->sn, "-", 1)) {
        for (int i = 0; i < Domain_size; i++)
          Cells[X1(s->base,i)].value = Domain[(Domain_size - i) % Domain_size];
      }
      else if (S.is_symbol(s->sn, "--", 2)) {
        for (int i = 0; i < Domain_size; i++)
          for (int j = 0; j < Domain_size; j++)
            Cells[X2(s->base,i,j)].value = Domain[((i + Domain_size) - j) % Domain_size];
      }
    }
  }
  if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->order_domain)) {
    for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
      if (S.is_symbol(s->sn, "<", 2)) {
        for (int i = 0; i < Domain_size; i++)
          for (int j = 0; j < Domain_size; j++)
            Cells[X2(s->base,i,j)].value = (Domain[i<j ? 1 : 0]);
      }
      if (S.is_symbol(s->sn, "<=", 2)) {
        for (int i = 0; i < Domain_size; i++)
          for (int j = 0; j < Domain_size; j++)
            Cells[X2(s->base,i,j)].value = (Domain[i<=j ? 1 : 0]);
      }
    }
  }
}

Term
Search::interp_term(void)
{
  /*
   *   Construct a term representing the current interpretation, e.g.
   *
   *   interpretation( 3, [
   *         function(B, [2]),
   *         function(g(_), [1,0,1])]).
   */
  InterpContainer   interp_con;
  TermContainer     term_con;
  SymbolContainer   symbol_con;
  ListtermContainer list_con;
  Term symlist = list_con.get_nil_term();

  for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
    if (s->attribute != EQUALITY_SYMBOL) {
      Term symterm = term_con.get_rigid_term_dangerously(s->sn, s->arity);
      for (int i = 0; i < s->arity; i++)
        ARG(symterm,i) = term_con.get_variable_term(i);
      int n = interp_con.int_power(Domain_size, s->arity);
      Term tableterm = list_con.get_nil_term();
      for (int i = n-1; i >= 0; i--) {
        int id = s->base + i;
        if (Cells[id].value == nullptr)
          fatal::fatal_error("interp_term, incomplete interpretation");
        Term it = term_con.nat_to_term(VARNUM(Cells[id].value));
        tableterm = list_con.listterm_cons(it, tableterm);
      }
      Term entry = term_con.build_binary_term(symbol_con.str_to_sn(s->type == type_FUNCTION ? "function" : "relation", 2),
                                              symterm, tableterm);
      symlist = list_con.listterm_cons(entry, symlist);
    }
  }
  return term_con.build_binary_term(symbol_con.str_to_sn("interpretation", 2),
                                    term_con.nat_to_term(Domain_size), symlist);
}

int
Search::possible_model(void)
{
  if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->arithmetic)) {
    if (!propagator->check_with_arithmetic())
      return SEARCH_GO_NO_MODELS;
  }
  else if (!propagator->check_that_ground_clauses_are_true())
    fatal::fatal_error("possible_model, bad model found");

  Total_models++;
  Mstats.current_models++;

  if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->return_models)) {
    InterpContainer inter_con;
    TermContainer   term_con;
    Term   modelterm = interp_term();
    Interp model     = inter_con.compile_interp(modelterm, false);
    term_con.zap_term(modelterm);
    PlistContainer p_con;
    Models = p_con.plist_append(Models, model);
  }

  if (LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->print_models_interp) > 0)
    print_model_interp(*models_interp_file_stream);
  else if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->print_models))
    print_model_standard(std::cout, true);
  else if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->print_models_tabular))
    p_model(false);
  else if (next_message == Total_models) {
    std::cout << "\nModel " << Total_models << " has been found." << std::endl;
    if (Total_models >= 100000000)
    	next_message *= 2;
    else
    	next_message *= 10;
  }
  if (LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->max_models) != -1 && Total_models >= LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->max_models))
    return SEARCH_MAX_MODELS;
  else
    return SEARCH_GO_MODELS;
}

int
Search::mace_megs(void)
{
  return (Memory::megs_malloced() - Start_megs) + (EScon.estack_bytes() / (1024*1024));
}

int
Search::check_time_memory(int seconds)
{
  // double seconds = myClock::user_seconds();
  int max_seconds = LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->max_seconds);
  int max_seconds_per = LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->max_seconds_per);
  int max_megs = LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->max_megs);
  int report = LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->report_stderr);

  if (max_seconds != -1 && seconds - Start_seconds > max_seconds)
    return SEARCH_MAX_TOTAL_SECONDS;
  else if (max_seconds_per != -1 &&
           seconds - Start_domain_seconds > LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->max_seconds_per))
    return SEARCH_MAX_DOMAIN_SECONDS;
  else if (max_megs != -1 && mace_megs() > LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->max_megs))
    return SEARCH_MAX_MEGS;
  else {
    if (report > 0) {
      if (Next_report == 0)
        Next_report = LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->report_stderr);
      if (seconds >= Next_report) {
        std::cerr << "Domain_size=" << Domain_size << ". Models=" << Total_models
                  << ". User_CPU=" << std::setprecision(2) << seconds << "." << std::endl;
        while (seconds >= Next_report)
          Next_report += report;
      }
    }
    return SEARCH_GO_NO_MODELS;
  }
}

bool
Search::mace4_skolem_check(int id)
{
  /* Should we keep going w.r.t. the Skolem restriction? */
  if (!LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->skolems_last))
    return true;
  else if (Cells[id].symbol->attribute == SKOLEM_SYMBOL) {
    std::cout << "pruning\n";
    return false;
  }
  else
    return true;
}

/*
 *   Max_constrained is the maximum constrained domain element
 *   (or -1 is none is constrained).  Greater domain elements
 *   can all be considered symmetric.  An element can become
 *   constrained in two ways:  (1) it is an index of some selected
 *   cell, or (2) it is the value assigned to some selected cell.
 *   (Propagation does not constrain elements.  This might need
 *   careful justification.)
 *
 *   To apply the least number heuristic, we consider values
 *   0 ... MIN(max_constrained+1, Domain_size-1).
 *
 *   To make this effective, we should keep max_constrained as low as
 *   possible by selecting cells with maximum index <= max_constrained.
 *
 *   return:
 *     SEARCH_GO_MODELS
 *     SEARCH_GO_NO_MODELS
 *     SEARCH_MAX_MODELS
 *     SEARCH_MAX_MEGS
 *     SEARCH_MAX_TOTAL_SECONDS
 *     SEARCH_MAX_DOMAIN_SECONDS
 *
 */

int
Search::search(int max_constrained, int depth, Cube& splitter)
{
  int seconds = current_time();
  splitter.set_time(seconds);
  int rc = check_time_memory(seconds);
  if (rc != SEARCH_GO_NO_MODELS)
    return rc;
  else {
	Selection selector(Domain_size, Domain, Cells, &EScon, &Mstats, Mace4vglobais->Opt);
    // TODO: [cc] check correctness of conversion to C++
    int id = selector.select_cell(max_constrained, First_skolem_cell, Number_of_cells, Ordered_cells, propagator);

    if (id == -1) {
      /*
      // If it is generating cubes, print out the cube and forget the model. The cube will generate the model later
      if (print_cubes >= 0) {
    	splitter.print_new_cube(print_cubes, num_cells_filled);
      	return SEARCH_GO_NO_MODELS;
      }
      */
      rc = possible_model();
      return rc;
    }
    else {
      int x = Cells[id].max_index;
      max_constrained = std::max(max_constrained, x);
      Mstats.selections++;

      if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->trace)) {
        std::cout << "select: ";
        p_model(false);
        /* p_possible_values(); */
      }

      int last = Domain_size-1;
      if (Cells[id].symbol->type == type_RELATION)
        last = 1;
      else if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->lnh))
        last = std::min(max_constrained+1, Domain_size-1);
      else
        last = Domain_size-1;
      bool go = true;

      // begin for cubes
      ParseContainer   pc;
      int from_index = 0;
      int value = splitter.value(depth, id);
      if (value >= 0) {
    	  if (last < value) {
        	  std::cout << "debug Search::search exceeded bounds ***************************** do cell id = " << id << std::endl;
    		  return SEARCH_GO_NO_MODELS;
    	  }
    	  std::cout << "debug Search::search ******************* do cell id = " << id
    			    << ", cell value = " << value << ",  op = " << Symbol_dataContainer::get_op_symbol(Cells[id].get_sn())
    	  	  	    << ", updated max_constrained " << max_constrained << ", depth = " << depth << std::endl;
    	  from_index = value;
    	  last = value;
      }
      if (print_cubes >= 0 && splitter.real_depth(depth, id) >= print_cubes) {
      	splitter.print_new_cube(print_cubes, splitter.num_cells_filled(Cells));
    	return SEARCH_GO_NO_MODELS;
      }
      // end for cubes

      all_nodes.push_back(std::vector<int> {id, from_index, last});
      size_t curr_pos = all_nodes.size() - 1;
      for (int i = from_index, go = true; i <= all_nodes[curr_pos][2] && go; i++) {
    	all_nodes[curr_pos][1] = i;
        if (splitter.move_on(id, all_nodes)) {
    		std::cout << "debug, Search::search stolen from " << i+1 << " to " << last << std::endl;
    	}
        Estack stk;
        Mstats.assignments++;

        if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->trace)) {
          std::cout << "assign: ";
          pc.fwrite_term(std::cout, Cells[id].eterm);
          std::cout << "=" << i << " (" << all_nodes[curr_pos][2] << ") depth=" << depth << "\n";
        }

        stk = propagator->assign_and_propagate(id, Domain[i]);

        if (stk != nullptr) {
          /* no contradiction found during propagation, so we recurse */
          rc = search(std::max(max_constrained, i), depth+1, splitter);

          /* undo assign_and_propagate changes */
          EScon.restore_from_stack(stk);
          if (rc == SEARCH_GO_MODELS)
            go = mace4_skolem_check(id);
          else
            go = (rc == SEARCH_GO_NO_MODELS);
        }
      }
      all_nodes.pop_back();
      return rc;
    }
  }
}

int
Search::mace4n(Plist clauses, int order)
{
  Mstate initial_state = MScon.get_mstate();

  Variable_Style save_style = Variable_Style();
  SymbolContainer   symbol_con;
  symbol_con.set_variable_style(Variable_Style::INTEGER_STYLE);

  if (Max_domain_element_in_input >= order) {
    if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->arithmetic)) {
      if (!Mace4vglobais->Arith->ok_for_arithmetic(clauses, order))
        return SEARCH_DOMAIN_OUT_OF_RANGE;
    }
    else
      return SEARCH_DOMAIN_OUT_OF_RANGE;
  }

  Domain_size = order;

  init_for_domain_size();

  built_in_assignments();  /* Fill out equality table (and maybe others). */

  special_assignments();  /* assignments determined by options */

  /* Instantiate clauses over the domain.  This also
     (1) makes any domain element constants into real domain elements,
     (2) applies OR, NOT, and EQ simplification, and
     (3) does unit propagation (which pushes events onto initial_state->stack).
     Do the units first, then the 2-clauses, then the rest. */

  for (Plist p = clauses; initial_state->ok && p != nullptr; p = p->next)
    if (LADRV_GLOBAIS_INST.Lit.number_of_literals((Literals)p->v) < 2) {
      propagator->generate_ground_clauses(static_cast<Topform>(p->v), initial_state);
    }

  for (Plist p = clauses; initial_state->ok && p != nullptr; p = p->next)
    if (LADRV_GLOBAIS_INST.Lit.number_of_literals((Literals)p->v) == 2) {
      propagator->generate_ground_clauses(static_cast<Topform>(p->v), initial_state);
    }

  for (Plist p = clauses; initial_state->ok && p != nullptr; p = p->next)
    if (LADRV_GLOBAIS_INST.Lit.number_of_literals((Literals)p->v) > 2) {
      propagator->generate_ground_clauses(static_cast<Topform>(p->v), initial_state);
    }

  /* The preceding calls push propagation events onto initial_state->stack.
     We won't have to undo those initial events during the search,
     but we can undo them after the search.
  */

  if (LADR_GLOBAL_OPTIONS.flag(Mace4vglobais->Opt->verbose)) {
    std::cout << "\nInitial partial model:\n";
    p_model(false);
    std::flush(std::cout);
  }

  Cube splitter(order, Cells, Ordered_cells, Number_of_cells, cubes_options);

  /* Here we go! */
  int rc = SEARCH_GO_NO_MODELS;
  if (initial_state->ok) {
    rc = search(Max_domain_element_in_input, 0, splitter);
    bool done = !splitter.reinitialize_cube();
    while (!done) {
    	all_nodes.clear();
        rc = search(Max_domain_element_in_input, 0, splitter);
    	done = !splitter.reinitialize_cube();
    }
  }
  // CC: changed b/c it has no effect.  rc is initialized to SEARCH_GO_NO_MODELS
  // else
  //  rc = SEARCH_GO_NO_MODELS;  /* contradiction in initial state */

  /* Free all of the memory associated with the current domain size. */

  EScon.restore_from_stack(initial_state->stack);
  MScon.free_mstate(initial_state);

  delete Ordered_cells;
  Ordered_cells = nullptr;

  for (int i = 0; i < Number_of_cells; i++) {
    Grounder->zap_mterm(Cells[i].eterm);
    delete Cells[i].possible;
  }
  delete Cells;
  Cells = nullptr;

  TermContainer   term_con;
  for (int i = 0; i < Domain_size; i++)
    term_con.zap_term(Domain[i]);
  delete Domain;
  Domain = nullptr;

  delete propagator;
  propagator = nullptr;

  symbol_con.set_variable_style(save_style);
  return rc;
}

bool
Search::iterate_ok(int n, const std::string& class_name)
{
  if (class_name == "all")
    return true;
  else if (class_name == "evens")
    return n % 2 == 0;
  else if (class_name == "odds")
    return n % 2 == 1;

  SmallPrime& sp = SmallPrime::getInstance();
  if (class_name == "primes")
    return sp.isPrime(n);
  else if (class_name == "nonprimes")
    return !sp.isPrime(n);
  else {
    fatal::fatal_error("iterate_ok, unknown class");
    return false;
  }
}

int
Search::next_domain_size(int n)
{
  int top = (LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->end_size) == -1 ? INT_MAX : LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->end_size));

  if (n == 0)
    n = LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->start_size);  /* first call */
  else
    n += LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->increment);

  while (!iterate_ok(n, LADR_GLOBAL_OPTIONS.stringparm1(Mace4vglobais->Opt->iterate)))
    n += LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->increment);

  return (n > top ? -1 : n);
}

Mace_results
Search::mace4(Plist clauses)
{
  Memory::enable_max_megs_check(false);   /* mace4 does its own max_megs check */
  Start_seconds = myClock::user_seconds();
  Start_megs = Memory::megs_malloced();
  Memory::set_max_megs(8000);

  initialize_for_search(clauses);
  if (LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->print_models_interp)>0) {
	  models_interp_file_stream = new ofstream();
	  models_interp_file_stream->open(Search::interp_file_name, std::ios_base::app);
  }

  int n = next_domain_size(0);  /* returns -1 if we're done */
  int rc = SEARCH_GO_NO_MODELS;

  while (n >= 2 && (rc == SEARCH_GO_NO_MODELS || rc == SEARCH_GO_MODELS)) {
    char str[20];
    sprintf(str, "DOMAIN SIZE %d", n);
    banner::print_separator(std::cout, str, true);
    std::flush(std::cout);
    std::cerr << "\n=== Mace4 starting on domain size " << n << " ===\n";

    Start_domain_seconds = myClock::user_seconds();
    mace4_clock.clock_start();
    rc = mace4n(clauses, n);
    if (rc == SEARCH_MAX_DOMAIN_SECONDS) {
      std::cout << "\n====== Domain size " << n << " terminated by max_seconds_per. ======\n";
      rc = SEARCH_GO_NO_MODELS;
    }
    else if (rc == SEARCH_DOMAIN_OUT_OF_RANGE) {
      std::cout << "\n====== Domain size " << n << " skipped because domain element too big. ======\n";
      rc = SEARCH_GO_NO_MODELS;
    }
    mace4_clock.clock_stop();
    p_stats();
    Mstats.reset_current_stats();
    mace4_clock.clock_reset();
    n = next_domain_size(n);  /* returns -1 if we're done */
  }

  if (LADR_GLOBAL_OPTIONS.parm(Mace4vglobais->Opt->print_models_interp)>0) {
	  models_interp_file_stream->close();
	  models_interp_file_stream = nullptr;
  }
  /* free memory used for all domain sizes */
  EScon.free_estack_memory();
  delete Sn_to_mace_sn;
  Sn_to_mace_sn = nullptr;

  Mace_results results = new mace_results();
  results->success = Total_models != 0;
  results->models = Models;  /* NULL if no models or not collecting models */
  results->user_seconds = myClock::user_seconds() - Start_seconds;

  if (rc == SEARCH_MAX_MODELS)
    results->return_code = MAX_MODELS_EXIT;
  else if (rc == SEARCH_GO_MODELS || rc == SEARCH_GO_NO_MODELS)
    results->return_code = Total_models==0 ? EXHAUSTED_EXIT : ALL_MODELS_EXIT;
  else if (rc == SEARCH_MAX_TOTAL_SECONDS)
    results->return_code = Total_models==0 ? MAX_SEC_NO_EXIT : MAX_SEC_YES_EXIT;
  else if (rc == SEARCH_MAX_MEGS)
    results->return_code = Total_models==0 ? MAX_MEGS_NO_EXIT : MAX_MEGS_YES_EXIT;
  else
    fatal::fatal_error("mace4: unknown return code");

  Memory::enable_max_megs_check(true);
  return results;
}

int
Search::id2val(int id)
{
  return (Cells[id].value == nullptr ? -1 : VARNUM(Cells[id].value));
}

int
Search::f0_val(int base)
{
  int id = X0(base);
  return id2val(id);
}

int
Search::f1_val(int base, int i)
{
  int id = X1(base,i);
  return id2val(id);
}

int
Search::f2_val(int base, int i, int j)
{
  int id = X2(base,i,j);
  return id2val(id);
}

void
Search::p_model(bool print_head)
{
  if (print_head) {
    banner::print_separator(std::cout, "MODEL", true);
    std::cout << "\n% Model " << Total_models << " at " << std::setprecision(2) << myClock::user_seconds() << " seconds.\n";
  }

  int n = Domain_size;
  SymbolContainer   symbol_con;
  for (Symbol_data p = Symbols; p != nullptr; p = p->next) {
    const char* name = symbol_con.sn_to_str(p->sn).c_str();
    if (p->attribute != EQUALITY_SYMBOL) {
      /* This prints both relations and functions. */
      if (p->arity == 0) {
        int v = f0_val(p->base);
        if (v < 0)
          std::cout << "\n " << name << " : -\n";
        else
          std::cout << "\n " << name << " : " << v << "\n";
      }
      else if (p->arity == 1) {
        const int   s1 = n <= 10 ? 2 : 3;
        const char *s2 = n <= 10 ? "--"  : "---";
        const char *s3 = n <= 10 ? " -"  : "  -";

        std::cout << "\n " << name << " :\n";
        std::cout << "       ";
        for (int i = 0; i < n; i++)
          std::cout << std::setw(s1) << i;
        std::cout << "\n    ---";
        for (int i = 0; i < n; i++)
          std::cout << s2;
        std::cout << "\n       ";
        for (int i = 0; i < n; i++) {
          int v = f1_val(p->base, i);
          if (v < 0)
            std::cout << s3;
          else
            std::cout << std::setw(s1) << v;
        }
        std::cout << "\n";
      }
      else if (p->arity == 2) {
        const int   s1 = n <= 10 ? 2 : 3;
        const char *s2 = n <= 10 ? "--"  : "---";
        const char *s3 = n <= 10 ? " -"  : "  -";

        std::cout << "\n " << name << " :\n";
        std::cout << "      |";
        for (int i = 0; i < n; i++)
          std::cout << std::setw(s1) << i;
        std::cout << "\n    --+";
        for (int i = 0; i < n; i++)
          std::cout << s2;
        std::cout << "\n";

        for (int i = 0; i < n; i++) {
          std::cout << std::setw(5) << i << " |";
          for (int j = 0; j < n; j++) {
            int v = f2_val(p->base, i, j);
            if (v < 0)
              std::cout << s3;
            else
              std::cout << std::setw(s1) << v;
          }
          std::cout << "\n";
        }
      }
      else {
        InterpContainer   interp_con;
        int n = interp_con.int_power(Domain_size, p->arity);

        Variable_Style save_style = Variable_Style();
        SymbolContainer   symbol_con;
        symbol_con.set_variable_style(Variable_Style::INTEGER_STYLE);
        ParseContainer pc;
        for (int i = 0; i < n; i++) {
          int id = p->base + i;
          pc.fwrite_term(std::cout, Cells[id].eterm);
          if (Cells[id].value == nullptr)
            std::cout << " = -.\n";
          else
            std::cout << " = " << VARNUM(Cells[id].value) << ".\n";
        }
        symbol_con.set_variable_style(save_style);
      }
    }
  }

  if (print_head)
    banner::print_separator(std::cout, "end of model", true);

}


void
Search::print_model_standard(std::ostream& fp, bool print_head)
{
  if (print_head)
    banner::print_separator(fp, "MODEL", true);

  fp << "\ninterpretation( " << Domain_size << ", [number=" << Total_models << ", seconds="
     << static_cast<int>(myClock::user_seconds()) << "], [\n";

  int syms_printed = 0;
  InterpContainer   interp_con;
  SymbolContainer   symbol_con;

  for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
    if (s->attribute != EQUALITY_SYMBOL) {
      if (syms_printed > 0)
        fp << ",\n";
      fp << "\n        " << (s->type == type_FUNCTION ? "function" : "relation") << "("
         << symbol_con.sn_to_str(s->sn) << (s->arity == 0 ? "" : "(_");
      for (int i = 1; i < s->arity; i++)
        fp << ",_";
      fp << (s->arity == 0 ? "" : ")") << ", [" << (s->arity >= 2 ? "\n\t\t\t  " : "");

      int n = interp_con.int_power(Domain_size, s->arity);
      for (int i = 0; i < n; i++) {
        int id = s->base + i;
        if (Cells[id].value == nullptr)
          fp << "-";
        else
          fp << std::setw(2) << VARNUM(Cells[id].value);
        if (i < n-1)
          fp << "," << ((i+1) % Domain_size == 0 ? "\n\t\t\t  " : "");
        else
          fp << " ])";
      }
      syms_printed++;
    }
  }

  fp << "\n]).\n";

  if (print_head)
    banner::print_separator(fp, "end of model", true);

}


void
Search::print_model_interp(std::ostream& fp)
{
  /* Prints the model the same format as interpformat, to be used as inputs to isofilter directly*/
  /* also ignore constants */

  fp << "interpretation( " << Domain_size << ", [number=" << Total_models << ", seconds="
     << static_cast<int>(myClock::user_seconds()) << "], [";

  bool syms_printed = false;
  InterpContainer   interp_con;
  SymbolContainer   symbol_con;

  for (Symbol_data s = Symbols; s != nullptr; s = s->next) {
    if (s->attribute != EQUALITY_SYMBOL && s->arity > 0) {
      if (syms_printed)
        fp << ",";
      fp << "\n  " << (s->type == type_FUNCTION ? "function" : "relation") << "("
         << symbol_con.sn_to_str(s->sn) << (s->arity == 0 ? "" : "(_");
      for (int i = 1; i < s->arity; i++)
        fp << ",_";
      fp << (s->arity == 0 ? "" : ")") << ", [" << (s->arity >= 2 ? "\n    " : "");

      int n = interp_con.int_power(Domain_size, s->arity);
      for (int i = 0; i < n; i++) {
        int id = s->base + i;
        if (Cells[id].value == nullptr)
          fp << "-";
        else
          fp << VARNUM(Cells[id].value);
        if (i < n-1)
          fp << "," << ((i+1) % Domain_size == 0 ? "\n    " : "");
        else
          fp << " ])";
      }
      syms_printed = true;
    }
  }

  fp << "]).\n";
}

void
Search::p_matom(Term atom)
{
  ParseContainer pc;
  if (atom == nullptr)
    std::cout << "(NULL)";
  else if (!mace4_gv.NEGATED(atom))
    pc.fwrite_term(std::cout, atom);
  else if (mace4_gv.EQ_TERM(atom)) {
    pc.fwrite_term(std::cout, ARG(atom,0));
    std::cout << " != ";
    pc.fwrite_term(std::cout, ARG(atom,1));
  }
  else {
    std::cout << "~(";
    pc.fwrite_term(std::cout, atom);
    std::cout << ")";
  }
  std::cout << ".\n";
}

int
Search::eterms_count(Term t)
{
  return (t == nullptr ? 0 : 1 + eterms_count(static_cast<Term>(t->u.vp)));
}

void
Search::p_eterms(void)
{
  std::cout << "\n------- Cells --------\n";
  ParseContainer pc;
  for (int i = 0; i < Number_of_cells; i++) {
    int n = eterms_count(Cells[i].occurrences);
    if (n > 0) {
      pc.fwrite_term(std::cout, Cells[i].occurrences);
      std::cout << "	" << n << " occ, id=" << i << ", val=";
      if (Cells[i].value == nullptr)
        std::cout << "NULL";
      else
        pc.fwrite_term(std::cout, Cells[i].value);
      std::cout << ", pvals=";
      for (int j = 0; j < CellContainer::id_to_domain_size(i, Cells, Domain_size); j++) {
        if (Cells[i].possible[j] == nullptr)
          std::cout << " -";
        else
          std::cout << std::setprecision(2) << j;
      }
      std::cout << "\n";
    }
  }
}

void
Search::p_stats(void)
{
  Mstats.p_stats(Domain_size, mace4_clock);
}

void
Search::p_mem(void)
{
  std::cout << "\n------------- memory usage (for entire run) -------------------\n";

  std::cout << "\nTotal malloced: " << Memory::megs_malloced() << " megabytes\n";

  StrbufContainer SB;
  SB.fprint_strbuf_mem(std::cout, true);

  ParseContainer P;
  P.fprint_parse_mem(std::cout, false);
  LADR_GLOBAL_G_LIST.fprint_glist_mem(std::cout, false);

  TermContainer T;
  T.fprint_term_mem(std::cout, false);

  TopformContainer TF;
  TF.fprint_topform_mem(std::cout, false);

  ClistContainer CL;
  CL.fprint_clist_mem(std::cout, false);

  Grounder->fprint_mclause_mem(std::cout, false);
  MScon.fprint_mstate_mem(std::cout, false);
  EScon.fprint_estack_mem(std::cout, false);
  Memory::memory_report(std::cout);
}

