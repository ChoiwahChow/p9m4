#ifndef TP_PARSE_H
#define TP_PARSE_H

#include "glist.h"
#include "strbuf.h"
#include "term.h"
#include "symbols.h"
#include "memory.h"


#define COMMENT_CHAR '%'


class GlobalParse {


private:
    char Cons_char;
    char Quote_char;
    int Quantifier_precedence;  /* see declare_quantifier_precedence */
    bool Parenthesize;
    bool Check_for_illegal_symbols;
    bool Simple_parse;
    bool   Translate_neg_equalities;
    PlistContainer Multiple_char_special_syms; //temos aqui uma PlistContainer global....tem de se zapar no final
    // Memory:: management
    unsigned Token_gets, Token_frees;
    unsigned Pterm_gets, Pterm_frees;
   
    
    
public:
    GlobalParse();
    ~GlobalParse();
    void Free_Mem(void);
    
    friend class ParseContainer;
  
    
};


/* Token types */

enum class TokType {
    TOK_UNKNOWN,  /* probably a garbage binary char */
    TOK_ORDINARY, /* see ordinary_char() */
    TOK_SPECIAL,  /* see special_char() */
    TOK_STRING,   /* see quote_char() */
    TOK_COMMENT,  /* see comment_char() */
    TOK_PUNC      /* see punctutation_char() */
};



/* Return codes from read_buf() */

enum class Read_rc{
    READ_BUF_OK,
    READ_BUF_EOF,
    READ_BUF_ERROR,
    READ_BUF_QUOTE_ERROR
};


struct token {
  TokType           type;
  char              c;        /* for punctuation and unknown tokens */
  String_buf        sb;       /* for other tokens */
  int               buf_pos;  /* position of this token in buffer */
  token             *next;
};

typedef struct token * Token;

/* A list of terms with some other data. */
struct pterm {
  Term  t;
  bool  parenthesized;  /* prevents "a b" being parsed as a(b) */
  pterm *prev, *next;
};
typedef struct pterm * Pterm;



/* Token position */
struct tok_pos {
  Token tok;
  string error_message;
  int start_error, end_error;
};

typedef struct tok_pos *Tok_pos;



class ParseContainer {

private:
            int getc(std::istream& in);
            
    
public:
        ParseContainer();
        ~ParseContainer();
        
        Token get_token(void);
        void free_token(Token);
        Pterm get_pterm(void);
        void free_pterm(Pterm);
        
        void fprint_parse_mem(ostream &, const bool) const;
        void p_parse_mem(void) const;
        void translate_neg_equalities(bool);
        Term translate_neg_eq(Term);
        Term untranslate_neg_eq(Term);
        void free_pterm_list(Pterm);
        void free_token_list(Token);
        bool end_char(char);
        bool comment_char(char);
        bool quote_char(char);
        bool punctuation_char(char);
        bool ordinary_char(char);
        bool special_char(char);
        bool white_char(char);
        bool all_whitespace(String_buf);
        int  finish_comment(istream &);
        int finish_comment(String_buf, int &);
        
        int read_buf(istream &, String_buf);
        int read_buf(String_buf, String_buf,int &);
        String_buf max_special_symbol(String_buf, int *);
        Token tokenize(String_buf);
        int   comma_terms(Term);
        void  transfer_comma_term(Term, Term, int *);
        bool quantifier(Term);
        bool ordinary_constant_string(string);
        bool ordinary_constant_string(char *);
        bool ordinary_constant(Term);
        bool quant_prefix(Pterm, Pterm);
        Term terms_to_term(Pterm, Pterm, int); 
        void next_token(Tok_pos);
        Term make_a_list(Term, Term);
        Term toks_to_list(Tok_pos);
        Term toks_to_term(Tok_pos);
        Pterm toks_to_terms(Tok_pos);
        void fprint_parse_error(ostream &, string , String_buf , int , int);
        Term sread_term(String_buf, ostream &);

        Term read_term(istream &, ostream &);
        Term read_term_from_string_buf(String_buf passed_sb, std::ostream& fout, int &);
        Term parse_term_from_string(const string &);
       
        
        void sb_remove_some_space(String_buf, int, int);
        
        void fwrite_term(ostream &, Term);
        void fwrite_term_nl(ostream &, Term);
        string process_quoted_symbol(string);
        void remember_multiple_char_special_syms(string);
        void p_tokens(Token);
        void declare_parse_type(string, int, ParseType); 
        void look_for_illegal_symbols(string);
        void forget_multiple_char_special_syms(string);
        void declare_standard_parse_types(void);
        void declare_quantifier_precedence(int);
        void set_cons_char(char);
        char get_cons_char(void);
        void set_quote_char(char);
        char get_quote_char(void);
        void parenthesize(bool);
        void simple_parse(bool);
        void sb_write_term(String_buf, Term);
        void arrange_term(String_buf , Term , int );
        bool redeclare_symbol_and_copy_parsetype(string, string,bool echo, ostream &);
        void p_token(Token); //only for debugging
        void p_pterm(Pterm);//only for debugging

        Plist split_string(const char* onlys);
       
};




#endif
