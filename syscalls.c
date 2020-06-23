


#include "fs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "dbgio.h"

#define	MAX_OPEN	64
static file_t fh[MAX_OPEN];
#include "dirent.h"
#define	CHECKFILE(file)	if (file<3) return -1
#define	FD2KOS(fd)	fh[fd]

//ian micheal system call replacements for saving routine

int
_read (int file,
       char *ptr,
       int len)
{
  CHECKFILE(file);
  return fs_read(FD2KOS(file),ptr,len);
}

int
_lseek (int file,
	int ptr,
	int dir)
{
  CHECKFILE(file);
//  dbgio_printf("seek %x,%d,%d\n",FD2KOS(file),ptr,dir);
  return fs_seek(FD2KOS(file),ptr,dir);
}

int
_write ( int file,
	 const char *ptr,
	 int len)
{
  switch(file) {
  case 0: /* stdin */
    return -1;
  case 1: /* stdout */
  case 2: /* stderr */
    dbgio_printf(ptr,len);
    return len;
  default:
    return fs_write(FD2KOS(file),ptr,len);
  }
}

int
_close (int file)
{
  CHECKFILE(file);
  fs_close(FD2KOS(file));
  fh[file]=0;
  return 0;
}

int
_open (const char *path,
	int flags)
{
  file_t f;
  int fd;

//  dbgio_printf(path);

  for(fd=3;fd<MAX_OPEN && fh[fd];fd++) ;
  if (fd==MAX_OPEN) {
    return -1;
  }
  f = fs_open(path,flags);
  if (f==0) {
//	dbgio_printf("\n");
  	return -1;
  }
//  dbgio_printf(":%x\n",f);
  fh[fd] = f;
  return fd;
}



DIR* opendir(const char *path)
{
	return (DIR*)fs_open(path,O_DIR);
}

struct dirent *readdir (DIR *dir)
{
	return fs_readdir((file_t)dir);
}

int closedir(DIR *dir)
{
	fs_close((file_t)dir);
	return 0;
}


