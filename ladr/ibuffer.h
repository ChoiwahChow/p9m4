#ifndef _INBUFFER_H
#define _INBUFFER_H

#include <climits>
#include <iostream>


using std::istream;

#define IBUF_INIT_SIZE      40000
#define IBUF_EOF            INT_MIN

class Ibuffer {
    private:
            int write_position; /* current number of ints in buf (next pos to write) */
            int read_position;  /* next pos to read */
            int size;           /* size of buf */
            int *buf;           /* the buffer */
            
          
   
            
    public:
            Ibuffer(void);
            ~Ibuffer();

            void ibuf_init(void);
            void ibuf_free(void);
            void ibuf_write(int);
            void ibuf_write_block(int *, int );
            void ibuf_rewind(void);
            int ibuf_read(void);
        
            int ibuf_xread(void);
            int ibuf_length(void);
            int *ibuf_buffer(void);
            void fd_read_to_ibuf(int);
            void p_ibuf(void);
};


#endif
