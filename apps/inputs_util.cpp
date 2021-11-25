

#include <string.h>
#include <fstream>
#include "../ladr/banner.h"
#include "../ladr/clause_misc.h"
#include "../ladr/fatal.h"
#include "../ladr/glist.h"
#include "../ladr/just.h"
#include "../ladr/mystring.h"
#include "../ladr/parse.h"
#include "../ladr/symbols.h"
#include "../ladr/term.h"

#include "../VERSION_DATE.h"

#include "inputs_util.h"

namespace isu
{

  bool read_line(std::istream& fin, char* line) {
    bool read_ok = !!fin.get(line, isu::line_buf_size);
    if (read_ok) {
      while (fin.get() == 10) ;
      fin.unget();
    }
    return read_ok;
  }

  int which_member_arg(int argc, const char *argv[], const char* str)
  {
    for (int i = 1; i < argc; ++i)
      if (strcmp(str, argv[i]) == 0 )
        return i;
    return -1;
  }

  void handle_help(int argc, const char *argv[], const char* Help_string, const char* PROGRAM_NAME) {
    if (which_member_arg(argc, argv, "help") > 0 ||
        which_member_arg(argc, argv, "-help") > 0) {
      std::cout << "\n" << PROGRAM_NAME <<", version " << PROGRAM_VERSION << ", " << PROGRAM_DATE << "\n";
      std::cout << Help_string;
      exit(1);
    }
  }

  char* beginning_of_line(std::istream& fin, char* line)
  {
    // Read the beginning of the line for the first token, or just % for comment line.
    bool done = false;
    char c;
    fin.get(c);
    while (c == '\n' || c == ' ')
      fin.get(c);
    if (c == '%') {
      line[0] = c;
      line[1] = '\0';
      return line;
    }
    int ptr = 0;
    while (c != ' ') {
      line[ptr++] = c;
      fin.get(c);
    }
    line[ptr] = '\0';
    return line;
  }

  bool read_to_line(std::istream& fin, const char *str)
  {
    while (fin.get() == 10) ;
    fin.unget();

    char line[line_buf_size];  /* the first line_buf_size - 1 chars of the line */
    bool read_ok = read_line(fin, line);

    while (read_ok && !myString::substring(str, line))
      read_ok = read_line(fin, line);

    return read_ok;
  }

  String_buf read_heading(std::istream& fin)
  {
    StrbufContainer strbuf_con;
    char line[line_buf_size];  /* the first BUF_MAX-1 chars of the line */
    strbuf_con.new_string_buf();

    int iptr = 0;
    bool read_ok = read_line(fin, line);
    while (read_ok && !myString::substring("=== end of head", line)) {
      if (iptr != 0) {
        strbuf_con.sb_append(line);
        strbuf_con.sb_append("\n");
      }
      iptr++;
      read_ok = read_line(fin, line);
    }

    if (!read_ok)
      fatal::fatal_error("read_heading, \"=== end of head\" not found");

    return strbuf_con.get_string_buf();
  }

  std::istream* get_fin(int argc, const char *argv[], std::ifstream& fin, const char** filename)
  {
    *filename = nullptr;
    int rc = which_member_arg(argc, argv, "-f");
    if (rc == -1)
      return &std::cin;
    else if (rc+1 >= argc)
      fatal::fatal_error("file name missing");
    else {
      *filename = argv[rc+1];
      fin.open(*filename, ios::in);
      if (!fin) {
        std::string s("File ");
        s = s + *filename + " not found.";
        fatal::fatal_error(s);
      }
    }
    return &fin;
  }

  void read_program_input(std::istream& fin, TopInput& topInput, Output_format output_format)
  {
    TermContainer    term_con;
    ParseContainer   parse_con;
    SymbolContainer  symbol_con;
    char  line[isu::line_buf_size];    // the first line_buf_size-1 chars of the line
    bool  in_list = false;             // parsing list of clauses, formulas, or terms?
    Plist lang_commands = nullptr;     // in case of multiple identical commands
    bool  read_ok = isu::read_line(fin, line);

    while (read_ok && !myString::substring("= end of input=", line)) {
      if (in_list)
        in_list = !myString::initial_substring("end_of_list.", line);
      else if (myString::initial_substring("clauses(", line)  ||
               myString::initial_substring("formulas(", line) ||
  	         myString::initial_substring("terms(", line))
        in_list = true;
      else if (myString::initial_substring("op(", line) ||
               myString::initial_substring("redeclare(", line)) {
        Term cmd = parse_con.parse_term_from_string(line);
        if (myString::initial_substring("op(", line))
          topInput.process_op(cmd, false, std::cout);
        else
          topInput.process_redeclare(cmd, false, std::cout);
        if (output_format != Output_format::XML && output_format != Output_format::IVY) {
          if (!term_con.tlist_member(cmd, lang_commands)) {  // don't print duplicates
            parse_con.fwrite_term_nl(std::cout, cmd);
            PlistContainer  p0;
            p0.set_head(lang_commands);
            lang_commands = p0.plist_prepend(cmd);
          }
        }
      }
      else if (myString::substring("set(prolog_style_variables)", line)) {
        if (output_format != Output_format::XML && output_format != Output_format::IVY)
          std::cout << "\nset(prolog_style_variables).";
        symbol_con.set_variable_style(Variable_Style::PROLOG_STYLE);
      }

      read_ok = isu::read_line(fin, line);
    }

    if (!read_ok)
      fatal::fatal_error("read_program_input, \"= end of input =\" not found");

    term_con.zap_plist_of_terms(lang_commands);
  }

  void print_proof(std::ostream& fp, Plist proof, String_buf comment,
                   Clause_print_format format, I3list jmap, int number)
  {
    JustContainer j_con;
    int length = j_con.proof_length(proof);
    ClausesContainer c_con;
    int max_count = c_con.max_clause_symbol_count(proof);

    StrbufContainer sb_con;
    if (format == Clause_print_format::CL_FORM_XML) {
      fp << "\n<proof number=\"" << number << "\" length=\"" << length << "\" max_count=\"" << max_count << "\">\n";
      fp << "\n<comments><![CDATA[\n";
      sb_con.set_string_buf(comment);
      sb_con.fprint_sb(fp);
      fp << "]]></comments>\n";

    }
    else if (format == Clause_print_format::CL_FORM_IVY) {
      fp << "\n;; BEGINNING OF PROOF OBJECT\n";
      fp << "(\n";
    }
    else {
      banner::print_separator(std::cout, "PROOF", true);
      fp << "\n% -------- Comments from original proof --------\n";
      sb_con.set_string_buf(comment);
      sb_con.fprint_sb(fp);
      fp << "\n";
    }

    for (Plist p = proof; p; p = p->next)
      Ioutil::fwrite_clause_jmap(fp, static_cast<Topform>(p->v), (int)format, jmap);

    if (format == Clause_print_format::CL_FORM_XML)
      fp << "\n</proof>\n";
    else if (format == Clause_print_format::CL_FORM_IVY) {
      fp << ")\n";
      fp << ";; END OF PROOF OBJECT\n";
    }
    else
      banner::print_separator(std::cout, "end of proof", true);
  }

}
