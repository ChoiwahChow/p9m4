/* Format an interpretation in various ways
 */

#include <fstream>
#include <string>

#include "../ladr/banner.h"
#include "../ladr/fatal.h"
#include "../ladr/interp.h"
#include "../ladr/ioutil.h"
#include "../ladr/mystring.h"
#include "../ladr/parse.h"
#include "../ladr/strbuf.h"
#include "../ladr/top_input.h"

#include "inputs_util.h"

static constexpr char PROGRAM_NAME[] = "modelformat";

static constexpr char Help_string[] =
"\nThis program reads interpretations in standard format\n"
"(from std::cin or with -f <file>).  The input can be just interps\n"
"(with or without list(interpretations)) or a Mace4 output file.\n"
"It and takes a command-line argument saying how to print the interps:\n\n"
"    standard    : one line per operation\n"
"    standard2   : standard, with binary operations in a square (default)\n"
"    portable    : list of lists, suitable for parsing by Python, GAP, etc.\n"
"    tabular     : as nice tables\n"
"    raw         : similar to standard, but without punctuation\n"
"    cooked      : as terms, e.g., f(0,1)=2\n"
"    tex         : formatted for LaTeX\n"
"    xml         : XML\n\n"
"Also, argument \"output '<operations>'\" is accepted.\n\n";

enum {STANDARD, STANDARD2, PORTABLE, TABULAR, RAW, COOKED, TEX, XML};

// Note that this program cannot handle input lines longer than line_buf_size-1 characters


static
String_buf read_next_section(std::istream& fin)
{
  StrbufContainer strbuf_con;
  strbuf_con.new_string_buf();

  char line[isu::line_buf_size];  /* the first line_buf_size - 1 chars of the line */
  bool read_ok = isu::read_line(fin, line);

  bool ok = false;
  while (read_ok && !myString::substring("==== end of", line)) {
    if (ok) {
      strbuf_con.sb_append(line);
      strbuf_con.sb_append("\n");
    }
    else if (myString::initial_substring("====", line))
      ok = true;
    read_ok = isu::read_line(fin, line);
  }
  if (!read_ok)
    fatal::fatal_error("read_next_section, \"==== end of\" not found");

  return strbuf_con.get_string_buf();
}

static
String_buf read_mace4_input(std::istream& fin)
{
  StrbufContainer strbuf_con;
  strbuf_con.new_string_buf();

  char line[isu::line_buf_size];  /* the first line_buf_size-1 chars of the line */
  bool read_ok = !!fin.get(line, isu::line_buf_size);
  if (read_ok) {
    while (fin.get() == 10) ;
    fin.unget();
  }
  bool ok = false;
  while (read_ok && !myString::substring("==== end of input", line)) {
    if (!ok && (myString::initial_substring("clauses(", line) ||
    		    myString::initial_substring("formulas(", line))) {
      if (strbuf_con.sb_size() != 0)
        strbuf_con.sb_append("\n");  /* no newline before first list */
      ok = true;
    }

    if (ok) {
      strbuf_con.sb_append(line);
      strbuf_con.sb_append("\n");
      if (myString::initial_substring("end_of_list.", line))
        ok = false;
    }

    read_ok = !!fin.get(line, isu::line_buf_size);
    if (read_ok) {
      while (fin.get() == 10) ;
      fin.unget();
    }
  }

  if (!read_ok)
    fatal::fatal_error("read_mace4_input, \"==== end of input\" not found");

  return strbuf_con.get_string_buf();
}

static
Term next_interp(istream& fin, bool mace4_file)
{
  ParseContainer pc;
  if (mace4_file) {
    if (isu::read_to_line(fin, "==== MODEL="))
      return pc.read_term(fin, std::cerr);
    else
      return nullptr;
  }
  else {
    Term t = pc.read_term(fin, std::cerr);
    TermContainer   tc;
    if (t == nullptr)
      return nullptr;
    else if (tc.is_term(t, "terms", 1) || tc.is_term(t, "list", 1)  || Ioutil::end_of_list_term(t)) {
      tc.zap_term(t);
      return next_interp(fin, false);
    }
    else
      return t;
  }
}  /* next_interp */

int main(int argc, const char *argv[])
{
  isu::handle_help(argc, argv, Help_string, PROGRAM_NAME);

  int type = STANDARD2; // default
  if (isu::which_member_arg(argc, argv, "standard") > 0)
    type = STANDARD;
  else if (isu::which_member_arg(argc, argv, "standard2") > 0)
    type = STANDARD2;
  else if (isu::which_member_arg(argc, argv, "portable") > 0)
    type = PORTABLE;
  else if (isu::which_member_arg(argc, argv, "tabular") > 0)
    type = TABULAR;
  else if (isu::which_member_arg(argc, argv, "raw") > 0)
    type = RAW;
  else if (isu::which_member_arg(argc, argv, "cooked") > 0)
    type = COOKED;
  else if (isu::which_member_arg(argc, argv, "tex") > 0)
    type = TEX;
  else if (isu::which_member_arg(argc, argv, "xml") > 0)
    type = XML;

  bool wrap = isu::which_member_arg(argc, argv, "wrap") > 0;  /* enclose output in list(interpretations). */

  Plist output_strings = nullptr;
  ParseContainer pc;
  int rc = isu::which_member_arg (argc, argv, "output");
  if (rc == -1)
    rc = isu::which_member_arg (argc, argv, "-output");
  if (rc > 0) {
    if (rc+1 >= argc)
      fatal::fatal_error("interpformat: missing \"output\" argument");
    else
      output_strings = pc.split_string(argv[rc+1]);
  }

  std::ifstream fp;
  const char* filename = nullptr;
  std::istream* fin = isu::get_fin(argc, argv, fp, &filename);

  /* Input can be any of 3 types:
       1. stream of interps
       2. list of interps, surrounded by list(interpretations) .. end_of_list
       3. Mace4 output file, with "= MODEL =" ... "= end of model ="
     See next_interp().
  */

  rc = fin->get();
  bool mace4_file = (rc == '=');      /* list of interps or mace4 output */
  if (!mace4_file)
    fin->unget();

  TopInput topInput;
  topInput.init_standard_ladr();
  pc.simple_parse(true);


  String_buf heading = nullptr;
  String_buf mace4_input = nullptr;
  if (mace4_file) {
    heading = read_next_section(*fin);
    mace4_input = read_mace4_input(*fin);
  }

  /* Okay, read and format the interps. */

  if (type == XML) {
    std::cout << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
    std::cout << "\n<!DOCTYPE interps SYSTEM \"interp3.dtd\">\n";
    std::cout << "\n<?xml-stylesheet type=\"text/xsl\" href=\"interp3.xsl\"?>\n";
    std::cout << "\n<interps>\n";
    if (fin != &std::cin)
      std::cout << "\n<source>" << filename << "</source>\n";
    if (heading) {
      std::cout << "\n<heading><![CDATA[\n";
      StrbufContainer heading_sb_con;
      heading_sb_con.set_string_buf(heading);
      heading_sb_con.fprint_sb(std::cout);
      std::cout << "]]></heading>\n";
    }
    if (mace4_input) {
      std::cout << "\n<input><![CDATA[\n";
      StrbufContainer mace4_input_sb_con;
      mace4_input_sb_con.set_string_buf(mace4_input);
      mace4_input_sb_con.fprint_sb(std::cout);
      std::cout << "]]></input>\n";
    }
  }

  if (wrap)
    std::cout << "list(interpretations).\n";

  Term t = next_interp(*fin, mace4_file);
  TermContainer   tc;
  InterpContainer ipc;
  int count = 0;

  while (t != nullptr) {
    count++;

    if (output_strings != nullptr)
      ipc.interp_remove_others(t, output_strings);

    Interp a = ipc.compile_interp(t, true);

    if (type == STANDARD)
      ipc.fprint_interp_standard(std::cout, a);
    else if (type == STANDARD2)
    	ipc.fprint_interp_standard2(std::cout, a);
    else if (type == PORTABLE) {
      std::cout << (count == 1 ? "[" : ",") << "\n";
      ipc.fprint_interp_portable(std::cout, a);
    }
    else if (type == TABULAR)
      ipc.fprint_interp_tabular(std::cout, a);
    else if (type == COOKED)
      ipc.fprint_interp_cooked(std::cout, a);
    else if (type == RAW)
      ipc.fprint_interp_raw(std::cout, a);
    else if (type == TEX)
      ipc.fprint_interp_tex(std::cout, a);
    else if (type == XML)
      ipc.fprint_interp_xml(std::cout, a);
    else
      ipc.fprint_interp_standard2(std::cout, a);

    ipc.zap_interp(a);
    tc.zap_term(t);

    t = next_interp(*fin, mace4_file);
  }

  if (fin != &std::cin)
	  fp.close();

  if (type == XML)
    std::cout << "\n</interps>\n";
  else if (type == PORTABLE)
    std::cout << "\n]\n";

  if (wrap)
    std::cout << "end_of_list.\n";

  exit(count > 0 ? 0 : 2);
}



