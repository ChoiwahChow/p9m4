#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include "ibuffer.h"
#include "fatal.h"
#include <ext/stdio_filebuf.h>

using namespace std;

using __gnu_cxx::stdio_filebuf;

using std::istream;


Ibuffer::Ibuffer(void) {
write_position = 0;
read_position = 0;
}

Ibuffer::~Ibuffer(void) {

}

void  Ibuffer::ibuf_init(void)
{
  buf =(int *) malloc ( sizeof(int) *IBUF_INIT_SIZE );
  size = IBUF_INIT_SIZE;
  write_position = 0;
  read_position = 0;
}




void Ibuffer::ibuf_free(void)
{
  free(buf);
}

void Ibuffer::ibuf_write(int i)
{
  if (write_position >= size) {
    int size2 = size * 2;
    int *buf2, i;
    cout<<"ibuf_write, increasing buf size from "<<size<< " to " << size2<<endl;
    buf2 = (int *) malloc(size2 * sizeof(int));
    for (i = 0; i < size; i++) buf2[i] = buf[i];
    free(buf);
    buf = buf2;
    size = size2;
  }
  buf[write_position] = i;
  write_position++;
}


/*************
 *
 *   ibuf_write_block()
 *
 *************/

/* DOCUMENTATION
Write an array of integers to an Ibuffer.
*/

/* PUBLIC */
void Ibuffer::ibuf_write_block(int *a, int n)
{
  int i;
  for (i = 0; i < n; i++)
    ibuf_write(a[i]);
}  /* ibuf_write_block */

/*************
 *
 *   ibuf_rewind()
 *
 *************/

/* DOCUMENTATION
Reset an Ibuffer for reading.
*/

/* PUBLIC */
void Ibuffer::ibuf_rewind(void)
{
  read_position = 0;
}  /* ibuf_rewind */

/*************
 *
 *   ibuf_read()
 *
 *************/

/* DOCUMENTATION
Get the next integer from an Ibuffer.
This version returns IBUF_EOF (INT_MIN) if there is nothing to read.
*/

/* PUBLIC */
int Ibuffer::ibuf_read(void)
{
  if (read_position >= write_position) return IBUF_EOF;
  else {
    int i = buf[read_position];
    read_position++;
    return i;
  }
}  /* ibuf_read */

/*************
 *
 *   ibuf_xread()
 *
 *************/

/* DOCUMENTATION
Get the next integer from an Ibuffer.
This version assumes there is an integer to read;
if it is at the end IBUF_EOF, a fatal error occurs.
*/

/* PUBLIC */
int Ibuffer::ibuf_xread(void)
{
  int i;
  if (read_position >= write_position) fatal::fatal_error("ibuf_xread: end of buffer");
  i = buf[read_position];
  read_position++;
  return i;
}  /* ibuf_xread */

/*************
 *
 *   ibuf_length()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int Ibuffer::ibuf_length(void)
{
  return write_position;
}  /* ibuf_length */

/*************
 *
 *   ibuf_buffer()
 *
 *************/

/* DOCUMENTATION
*/

/* PUBLIC */
int *Ibuffer::ibuf_buffer(void)
{
  return buf;
}  /* ibuf_buffer */

/*************
 *
 *   fd_read_to_ibuf()
 *
 *************/

#define ISIZE 100

/* DOCUMENTATION
*/


   




/* PUBLIC */
void Ibuffer::fd_read_to_ibuf(int fd)
{
  int csize = ISIZE * sizeof(int);
  int tibuf[ISIZE];
  int rc;
 
  do {
    rc = read(fd, tibuf, csize);
    if (rc == -1)
        fatal::fatal_error("fd_read_to_ibuf, read error");
    
    else if (rc == 0) ;  /* we're done */
    else if (rc % sizeof(int) != 0)
      fatal::fatal_error("fd_read_to_ibuf, bad number of chars read");
    else {
      ibuf_write_block(tibuf, rc / sizeof(int));
    }
  } while (rc > 0);

}  /* fd_read_to_ibuf */







/*************
 *
 *   p_ibuf()
 *
 *************/

/* DOCUMENTATION
Print a an Ibuffer to a stdout.  This is mainly for debugging.
*/

/* PUBLIC */
void Ibuffer::p_ibuf(void)
{
  int i = ibuf_read();
  while (i != IBUF_EOF) {
    cout <<i<<" ";
    i = ibuf_read();
  }
  cout <<endl;
}  /* p_ibuf */
