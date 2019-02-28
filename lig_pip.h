/**
 *
 *      filename:       lig_pip.h
 *      func:          COMMUNICATE WITH SOFT KB 
 *      auther:         zb
 *
 *
 *
 */

#ifndef __LIG_PIP_H__
#define __LIG_PIP_H__
#ifdef __cplusplus    
extern "C" {          
#endif
#include <unistd.h>
#include <stdint.h>
#include <error.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


#define LIG_PIP_CLIENT  0
#define LIG_PIP_SERVER  1

//return vale 
// < 0:error
// >= 0 :ok ,it's a file id,but not a standard  fd,only can used in this file
//
extern int lig_pip_open(int client_or_server);
extern void lig_pip_close(int fd);

//return vale
//<0 :error
//>=0 :the lenth of success read
extern int  lig_pip_read_bytes(int fd,char *buff,int bufflen);

//return vale
//<0 :error
//>=0 :the lenth of success write
extern int lig_pip_write_bytes(int fd,char *buff,int datalen);


extern int lig_pip_write(int id,int len,char* buff,void*special);
extern int lig_pip_read(int id,int len,char* buff,void*special);

#ifdef __cplusplus
}
#endif

#endif //__LIG_PIP_H__









