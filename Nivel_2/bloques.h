//  INCLUDES
#include <stdlib.h>         //  exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <stdio.h>          //  printf(), fprintf(), stderr, stdout, stdin

#include <fcntl.h>          //  O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h>       //  S_IRUSR, S_IWUSR

#include <unistd.h>         //  SEEK_SET, read(), write(), open(), close(), lseek()
#include <sys/types.h>      //  lseek()

#include <errno.h>          //  errno
#include <string.h>         //  strerror(), memset()

//  CONSTANTES
#define BLOCKSIZE 1024


//  FUNCIONES
int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);