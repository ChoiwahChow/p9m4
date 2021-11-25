
#ifndef MACE4_MACE4_H
#define MACE4_MACE4_H

#include <ostream>
#include "../ladr/glist.h"
#include "options.h"
#include "msearch.h"


class MACE4 {
public:
  static constexpr auto max_models_str = "max_models";
  static constexpr auto all_models_str = "all_models";
  static constexpr auto exhausted_str = "exhausted";
  static constexpr auto max_megs_yes_str = "max_megs_yes";
  static constexpr auto max_megs_no_str = "max_megs_no";
  static constexpr auto max_sec_yes_str = "max_sec_yes";
  static constexpr auto max_sec_no_str = "max_sec_no";
  static constexpr auto mace_sigint_str = "mace_sigint";
  static constexpr auto mace_sigsegv_str = "mace_sigsegv";
  static constexpr auto unknown_str = "???";

private:
  static Search* curr_search;

public:
  static void  init_attrs(void);
  static void  mace4_exit(int exit_code);
  static void  mace4_sig_handler(int condition);

  static const std::string exit_string(int code);

  static void set_curr_search(Search* s) {curr_search = s;}
};



#endif
