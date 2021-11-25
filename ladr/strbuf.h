#ifndef TP_STRBUF_H
#define TP_STRBUF_H

#include "string.h"
#include <iostream>

using namespace std;

class GlobalStrbuf {
                        private:
                                    unsigned String_buf_gets,String_buf_frees;
                                    unsigned Chunk_gets, Chunk_frees;

                                    void fprint_strbuf_mem(ostream &, bool);
                                    void p_strbuf_mem(void);
                        public:
                                    GlobalStrbuf();
                                    ~GlobalStrbuf();
                            
                                    friend class StrbufContainer;
                                    friend class LadrVGlobais;


};


#define CHUNK_SIZE 100

struct chunk {
  char s[CHUNK_SIZE];
  struct chunk *next;
};

typedef struct chunk *Chunk;

struct string_buf {
  struct chunk *first;
  int size;
};

typedef struct string_buf * String_buf;


class StrbufContainer{

                private:
                       
                    
                    
                        Chunk get_chunk(void);
                        void  free_chunk(Chunk);
                        void  free_string_buf(String_buf); 
                        String_buf private_get_string_buf(void); 
                      
                        String_buf private_init_string_buf(char *);
                        String_buf private_init_string_buf(string);
                        void fprint_sb(ostream &,String_buf);
                        void p_sb(String_buf);
                        void sb_append(String_buf,char *);
                       
                       
                       
                      
                        void sb_cat_copy(String_buf, String_buf);
                        void sb_cat(String_buf, String_buf);
                        char *sb_to_malloc_string(String_buf);
                        char *sb_to_malloc_char_array(String_buf);
                        void sb_append(String_buf,string);
                        void sb_append_int(String_buf,int);
                        void sb_append_char(String_buf, char);
                        int sb_size(String_buf);
                        char sb_char(String_buf, int);
                        void sb_replace_char(String_buf, int , char);
                     
                        void zap_string_buf(String_buf);
                        
                        
                        
                        String_buf strbuf; //o strbuf interno do objecto
                       

                public:
                        StrbufContainer(); 
                        ~StrbufContainer(); 
                        
                        void set_string_buf(String_buf); //apenas afecta o container strbuf, não faz festão de memória.....
                        String_buf get_string_buf(void);
                        void fprint_strbuf_mem(ostream &, bool);
                        void p_strbuf_mem();
                        //métodos que trabalham com o strbuf interno do objecto e que vão estar disponíveis para usar
                        void sb_append(string);
                        void new_string_buf(string);
                        void zap_string_buf(void);
                        bool null();
                        int sb_size(void);
                        char *sb_to_malloc_string(void);
                        char *sb_to_malloc_char_array(void);
                        char sb_char(int);
                        void sb_append_char(char);
                        void new_string_buf(void);
                        void p_sb(void);
                        void sb_replace_char(int , char);
                        void sb_append_int(int);
                        void sb_cat(String_buf);
                        void fprint_sb(ostream &);
                        
                        


};


#endif
