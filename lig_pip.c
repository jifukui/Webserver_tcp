/*
 *      filename :lig_pip.c
 *      func:
 *      auther :zb
 *      date :20161030
 *
 */

#include "lig_pip.h"
#include <stdio.h>
#include <unistd.h>

#define THIS_MODULE_NAME "pip"

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
                
                return -1;
        }
        if(mkfifo(LIG_PIP_CLN_FILE_PATH,S_IRUSR|S_IWUSR|S_IWGRP)==-1&&errno!=EEXIST)
        {
               
                return -1;
        }

        if(server_or_client==LIG_PIP_SERVER)
        {
                m_lig_pip_rdwr_fd=open(LIG_PIP_CLN_FILE_PATH,O_RDONLY|O_NONBLOCK);
                s_mode=O_RDONLY|O_NONBLOCK;
                c_mode=O_WRONLY|O_NONBLOCK;
        }
        else
        {
                m_lig_pip_rdwr_fd= open(LIG_PIP_SRV_FILE_PATH,O_RDONLY|O_NONBLOCK);
                s_mode=O_WRONLY|O_NONBLOCK;
                c_mode=O_RDONLY|O_NONBLOCK;
        }

        s_fd= open(LIG_PIP_SRV_FILE_PATH,s_mode);
        c_fd= open(LIG_PIP_CLN_FILE_PATH,c_mode);

        if(s_fd==-1||c_fd==-1)
        {
              
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
        restart:
        res=read(m_lig_pip_fd[fd].rfd,buff,bufflen);
        if(res<0&& ( errno == EINTR || errno == EAGAIN ))
        {
                printf("have error for read\n");
                printf("The pip error is %s\n",strerror(errno));
                sleep(1);
                goto restart;
        }
        if(res<0)
        {
                printf("read Have error\n");
                printf("The pip error is %s\n",strerror(errno));
                res=0;
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
        restart:
        res=write(m_lig_pip_fd[fd].wfd,buff,datalen);
        if(res<0&& ( errno == EINTR || errno == EAGAIN ))
        {
                printf("error no write\n");
                printf("The pip error is %s\n",strerror(errno));
                sleep(1);
                goto restart;
        }
        if(res<0)
        {
                printf("write Have error\n");
                printf("The pip error is %s\n",strerror(errno));
                res=0;
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

