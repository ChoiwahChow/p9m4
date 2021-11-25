
#include <iomanip>
#include "../ladr/attrib.h"
#include "../ladr/clock.h"
#include "../ladr/ladrvglobais.h"
#include "../ladr/nonport.h"
#include "mace4.h"

Search* MACE4::curr_search = nullptr;

void
MACE4::init_attrs(void)
{
  // This will allow these attributes to occur on clauses.
  // Mace4 will ignore these attributes.
  AttributeContainer a_con;
  a_con.register_attribute("label",         Attribute_type::STRING_ATTRIBUTE);
  a_con.register_attribute("bsub_hint_wt",  Attribute_type::INT_ATTRIBUTE);
  a_con.register_attribute("answer",        Attribute_type::TERM_ATTRIBUTE);
  a_con.register_attribute("action",        Attribute_type::TERM_ATTRIBUTE);
  a_con.register_attribute("action2",       Attribute_type::TERM_ATTRIBUTE);
}

const std::string
MACE4::exit_string(int code) {
  switch (code) {
    case MAX_MODELS_EXIT:   return max_models_str;
    case ALL_MODELS_EXIT:   return all_models_str;
    case EXHAUSTED_EXIT:    return exhausted_str;
    case MAX_MEGS_YES_EXIT: return max_megs_yes_str;
    case MAX_MEGS_NO_EXIT:  return max_megs_no_str;
    case MAX_SEC_YES_EXIT:  return max_sec_yes_str;
    case MAX_SEC_NO_EXIT:   return max_sec_no_str;
    case MACE_SIGINT_EXIT:  return mace_sigint_str;
    case MACE_SIGSEGV_EXIT: return mace_sigsegv_str;
    default: return unknown_str;
  }
}

void
MACE4::mace4_exit(int exit_code)
{
  if (curr_search->Mace4vglobais->Opt && LADR_GLOBAL_OPTIONS.flag(curr_search->Mace4vglobais->Opt->verbose))
    curr_search->p_mem();

  if (curr_search->Mace4vglobais->Opt && LADR_GLOBAL_OPTIONS.parm(curr_search->Mace4vglobais->Opt->report_stderr) > 0)
    std::cerr << "Domain_size=" << curr_search->Domain_size << ". Models=" << curr_search->Total_models << ". User_CPU="
              << std::setprecision(2) << myClock::user_seconds() << ".\n";

  std::cout << "\nUser_CPU=" << std::setprecision(2) << myClock::user_seconds() << ", System_CPU="
            << std::setprecision(2) << myClock::system_seconds() << ", Wall_clock=" << myClock::wallclock() << ".\n";

  if (curr_search->Total_models == 0)
    std::cout << "\nExiting with failure.\n";
  else
    std::cout << "\nExiting with " << curr_search->Total_models << " model" << (curr_search->Total_models == 1 ? "" : "s") << ".\n";

  std::cerr << "\n------ process " << nonport::my_process_id() << " exit (" << exit_string(exit_code) << ") ------\n";
  std::cout << "\nProcess " << nonport::my_process_id() << " exit (" << exit_string(exit_code) << ") " << myClock::get_date();

  std::cout << "The process finished " << myClock::get_date();
  exit(exit_code);
}

void
MACE4::mace4_sig_handler(int condition)
{
  std::cout << "\nmace4_sig_handler: condition " <<  condition;
  switch (condition) {
  case SIGSEGV:
    curr_search->p_stats();
    mace4_exit(MACE_SIGSEGV_EXIT);
    break;
  case SIGINT:
    curr_search->p_stats();
    mace4_exit(MACE_SIGINT_EXIT);
    break;
  case SIGUSR1:
    curr_search->p_stats();
    std::flush(std::cout);
    break;
  default: fatal::fatal_error("mace4_sig_handler, unknown signal");
  }
}

