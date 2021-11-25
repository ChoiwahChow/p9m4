#include "prover9vglobais.h"
#include "./ladr/clauseid.h"
#include "./ladr/demod.h"
#include "./ladr/backdemod.h"
#include "./ladr/flatdemod.h"






void Demodulate::init_demodulator_index(Mindextype mtype, Uniftype utype, int fpa_depth) {
  MindexContainer M;
  PROVER9_GLOBAL_DEMODULATE.Demod_idx = M.mindex_init(mtype, utype, fpa_depth);
} 

void Demodulate::init_back_demod_index(Mindextype mtype, Uniftype utype, int fpa_depth) {
    MindexContainer M;
    PROVER9_GLOBAL_DEMODULATE.Back_demod_idx = M.mindex_init(mtype, utype, fpa_depth);
}

void Demodulate::index_demodulator(Topform c, int type, Indexop operation, ClockStructure clock) {
  myClock CLOCK;
  CLOCK.clock_start(clock);
  Demod::idx_demodulator(c, type, operation, PROVER9_GLOBAL_DEMODULATE.Demod_idx);
  CLOCK.clock_stop(clock);
}

void Demodulate::index_back_demod(Topform c, Indexop operation, ClockStructure clock, bool enabled) {
  myClock CLOCK;
  if (enabled) {
    CLOCK.clock_start(clock);
    Backdemod::index_clause_back_demod(c, PROVER9_GLOBAL_DEMODULATE.Back_demod_idx, operation);
    CLOCK.clock_stop(clock);
  }
} 

void Demodulate::destroy_demodulation_index(void){
  MindexContainer M;
  M.mindex_destroy(PROVER9_GLOBAL_DEMODULATE.Demod_idx);
  PROVER9_GLOBAL_DEMODULATE.Demod_idx = NULL;
} 

void Demodulate::destroy_back_demod_index(void) {
  MindexContainer M;
  M.mindex_destroy(PROVER9_GLOBAL_DEMODULATE.Back_demod_idx);
  PROVER9_GLOBAL_DEMODULATE.Back_demod_idx = NULL;
} 


void Demodulate::demodulate_clause(Topform c, int step_limit, int increase_limit,bool print, bool lex_order_vars) {
  
  static int limit_hits = 0;
  ClauseidContainer CI;
  
  int starting_step_limit;
  step_limit = step_limit == -1 ? INT_MAX : step_limit;
  increase_limit = increase_limit == -1 ? INT_MAX : increase_limit;
  starting_step_limit = step_limit;

  Flatdemod::fdemod_clause(c, PROVER9_GLOBAL_DEMODULATE.Demod_idx, &step_limit, &increase_limit, lex_order_vars);

  if (step_limit == 0 || increase_limit == -1) {
    limit_hits++;
    string mess = (step_limit == 0 ? "step" : "increase");

    if (print) {
      if (limit_hits == 1) {
		cerr<<"Demod_"<<mess<<"_limit (see cout)"<<endl;	
	  cout<<"Demod_"<<mess<<"_limit (see cout)"<<endl;	  
	  cout<<endl<<"Demod_"<<mess<<"_limit (steps="<<starting_step_limit - step_limit;
	  cout<<", size="<<LADRV_GLOBAIS_INST.Lit.clause_symbol_count(c->literals)<<")."<<endl;  
	  cout<<"The most recent kept clause is "<<CI.clause_ids_assigned()<<endl;
	  cout<<"From here on, a short message will be printed"<<endl;
	  cout<<"for each 100 times the limit is hit."<<endl;
      }
      else if (limit_hits % 100 == 0) {
		cout<<"Demod_limit hit "<<limit_hits<<" times."<<endl;
		cout<<endl;
      }
    }
  }
} 


Plist Demodulate::back_demodulatable(Topform demod, int type, bool lex_order_vars) {
  return Backdemod::back_demod_indexed(demod, type, PROVER9_GLOBAL_DEMODULATE.Back_demod_idx, lex_order_vars);
}

void Demodulate::back_demod_idx_report(void) {
  FpaIndexContainer FPA;
  printf("Back demod index: ");
  cout<<"Back demod index: ";
  FPA.p_fpa_density(PROVER9_GLOBAL_DEMODULATE.Back_demod_idx->fpa);
}  /* back_demod_idx_report */
