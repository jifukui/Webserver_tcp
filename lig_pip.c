/*
 *      filename :lig_pip.c
 *      func:
 *      auther :zb
 *      date :20161030
 *
 */

#include "lig_pip.h"
#include <stdio.h>

//#include "debug_config.h"

#define THIS_MODULE_NAME "pip"
//#define LIG_PIP_DEBUG_ENABLE
#ifdef LIG_PIP_DEBUG_ENABLE
	#define pr_pip_debug(lv,fmt,args...)                pr_log(lv,THIS_MODULE_NAME,fmt,##args)
	#define Exerr(x)        do{pr_pip_debug(LIG_LOG_ERR,"error=%d\n",errno);exit(x);}while(0);
	#define perr_pip(fmt)   pr_pip_debug(LIG_LOG_ERR,"lig_pip:"fmt)
#else
	#define pr_pip_debug(lv,fmt,args...)                /* do nothing */
        #define Exerr(x)
	#define perr_pip(fmt)   
#endif

/*for 
thttpd , ser:lig_core_demo
         clt:thttpd
*/

#define LIG_PIP_SRV_FILE_PATH "/tmp/webpip_ser"
#define LIG_PIP_CLN_FILE_PATH "/tmp/webpip_clt"
#define LIG_PIP_CLIENT_NUM      1

extern int errno;

typedef struct PIP_FD
{
        int rfd;
        int wfd;
        int struct_id;
}LIG_STRUCT_PIP_FD;

LIG_STRUCT_PIP_FD m_lig_pip_fd[LIG_PIP_CLIENT_NUM];
int m_lig_pip_rdwr_fd;


int lig_pip_open(int server_or_client)
{
        int s_fd;
        int c_fd;
        mode_t s_mode,c_mode;
        if(server_or_client!=LIG_PIP_CLIENT &&server_or_client!=LIG_PIP_SERVER)
        {
                return -1;
        } 

        if(mkfifo(LIG_PIP_SRV_FILE_PATH,S_IRUSR|S_IWUSR|S_IWGRP)==-1&&errno!=EEXIST)
        {
                perror("mkfifo_server:");
                Exerr(1);
                return -1;
        }
        if(mkfifo(LIG_PIP_CLN_FILE_PATH,S_IRUSR|S_IWUSR|S_IWGRP)==-1&&errno!=EEXIST)
        {
                perror("mkfifo_client:");
                Exerr(1);
                return -1;
        }

        if(server_or_client==LIG_PIP_SERVER)
        {
                m_lig_pip_rdwr_fd=open(LIG_PIP_CLN_FILE_PATH,O_RDONLY|O_NONBLOCK);
                s_mode=O_RDONLY;
                c_mode=O_WRONLY|O_NONBLOCK;
        }
        else
        {
                m_lig_pip_rdwr_fd= open(LIG_PIP_SRV_FILE_PATH,O_RDONLY|O_NONBLOCK);
                s_mode=O_WRONLY;
                c_mode=O_RDONLY|O_NONBLOCK;
        }

        s_fd= open(LIG_PIP_SRV_FILE_PATH,s_mode);
        c_fd= open(LIG_PIP_CLN_FILE_PATH,c_mode);

        if(s_fd==-1||c_fd==-1)
        {
                perror("open fifo:");
                Exerr(1);
                return -1;
        }

        if(server_or_client==LIG_PIP_CLIENT)
        {
                m_lig_pip_fd[0].rfd=c_fd;
                m_lig_pip_fd[0].wfd=s_fd;
        }
        else
        {
                m_lig_pip_fd[0].rfd=s_fd;
                m_lig_pip_fd[0].wfd=c_fd;
        }
        if(signal(SIGPIPE,SIG_IGN)==SIG_ERR)
        {
                perr_pip("signal SIGPIPE:");
                Exerr(1);
                return -1;
        }
        return 0;
}

int  lig_pip_read_bytes(int fd,char *buff,int bufflen)
{
        int res=0;
        if(fd>=LIG_PIP_CLIENT_NUM)
        {
                return -1;
        }
        if(buff==NULL)
        {
                return -1;
        }
        res=read(m_lig_pip_fd[fd].rfd,buff,bufflen);
        if(res<0)
        {
                if(errno==EEXIST||errno==EAGAIN)
                {
                        res=0;
                }
                else
                {
                        pr_pip_debug(LIG_LOG_DEBUG,"error num :[%d]\n",errno);
                }
        }

        return res;
}

int lig_pip_write_bytes(int fd,char*buff,int datalen)
{
        int res=0;
        if(fd>=LIG_PIP_CLIENT_NUM)
        {
                return -1;
        }
        if(buff==NULL)
        {
                return -1;
        }
        res=write(m_lig_pip_fd[fd].wfd,buff,datalen);
        if(res<0)
        {
                if(errno==EEXIST||errno==EAGAIN)
                {
                        res=0;
                }
                else
                {
                      pr_pip_debug(LIG_LOG_ERR,"error num %d\n,",errno);
                }
        }
        return res;
}

void lig_pip_close(int fd)
{
        close(m_lig_pip_fd[fd].rfd);
        close(m_lig_pip_fd[fd].wfd);
        close(m_lig_pip_rdwr_fd);

}

int lig_pip_write(int id,int len,char* buff,void*special)
{
        return lig_pip_write_bytes(id,buff,len);
}
int lig_pip_read(int id,int len,char* buff,void*special)
{
        return lig_pip_read_bytes(id,buff,len);
}

//end

