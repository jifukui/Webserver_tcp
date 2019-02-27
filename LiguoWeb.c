#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <jansson.h>
/*#include <sys/time.h>*/

typedef unsigned char uint8;
typedef char int8;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short int uint16;
typedef short int int16;

#define STATIC static

uint8 LiguoWeb_GET_Method(const char *sstr,json_t *json,char *estr);
uint8 LiguoWeb_POST_Method(const unsigned char *sstr,json_t *json,char *estr);
STATIC uint8 CommandHandle(const char *sstr,json_t *json,char *estr);


STATIC void Uint8toString(int8 *str,uint8 *data,uint32 length);
STATIC void StringtoUint8(uint8 *dis,int8 *str);
STATIC uint8 JsonGetString(json_t *json,char *data);
STATIC uint8 JsonGetInteger(json_t *json,uint32 *data);
STATIC uint8 JsonGetUint8(json_t *json,uint8 *data);
STATIC uint8 JsonFromFile(uint8 *filepath,uint8 *data);

STATIC uint8 GetDeviceModuleName(json_t *json,char *estr);

void Uint8toString(int8 *str,uint8 *data,uint32 length)
{

	uint32 i,j=0;
	for(i=0;i<length;i++,j+=2)
	{
		sprintf(str+j,"%02X",*(data+i));
	}
}
void StringtoUint8(uint8 *dis,int8 *str)
{
	unsigned char data[3];
    data[2]='\0';
	uint32 test;
    while(*(str))
    {
        data[0]=*(str++);
        data[1]=*(str++);
		sscanf(data,"%02X",&test);
		*dis=test%256;
		dis++;
    }
}
uint8 JsonGetString(json_t *json,char *data)
{
	uint8 flag=0;
	if(json)
	{
		if(json_typeof(json)==JSON_STRING)
		{
			strcpy(data,json_string_value(json));
			flag=1;
		}	
	}
	return flag;
}
uint8 JsonGetInteger(json_t *json,uint32 *data)
{
	uint8 flag=0;
	if(json)
	{
		if(json_typeof(json)==JSON_INTEGER)
		{
			*data=json_integer_value(json);
			flag=1;
		}
	}

	return flag;
}
uint8 JsonGetUint8(json_t *json,uint8 *data)
{
	uint8 flag=0;
	uint32 val;
	if(json)
	{
		if(json_typeof(json)==JSON_INTEGER)
		{
			val=json_integer_value(json);
			*data=val%256;
			flag=1;
		}

	}
	return flag;
}


uint8 JsonFromFile(uint8 *filepath,uint8 *data)
{
	uint8 flag=0;
	FILE *file=NULL;
	file=fopen(filepath,"r");
	if(file)
	{
		uint32 i;
		for(i=0;i<4096;i++)
		{
			*(data+i)=fgetc(file);
			
			if(0==*(data+i))
			{
				break;
			}
		}
		flag=1;
	}
	return flag;
}



uint8 LiguoWeb_GET_Method(const char *sstr,json_t *json,char *estr)
{
	printf("GET Method\n");
	char* str=strchr(sstr,'=');
	uint8 flag=0;
	if(str)
	{
		flag=CommandHandle((str+1),json,estr);
	}
	else
	{
		strcpy(estr,"Error of format");
	}
	if(str)
	{
		str=NULL;
	}
	return flag;
}

uint8 LiguoWeb_POST_Method(const unsigned char *sstr,json_t *json,char *estr)
{
	printf("POST Method\n");
	return CommandHandle(sstr,json,estr);
}

uint8 CommandHandle(const char *sstr,json_t *json,char *estr)
{
	json_error_t error;
    json_t *jsonget;
    jsonget=json_loads(sstr,0,&error);
    uint8 flag=0;
	int8 data[20000];
    if(jsonget)
    {
        json_t *cmd;
        cmd=json_object_get(jsonget,"cmd");
		char str[30];
        if(JsonGetString(cmd,str))
        {
			
			json_error_t error;
			json_object_set_new(json,"cmd",json_string(str));
            if(!strcmp(str,"matrix_status"))
            {
                flag=GetDeviceModuleName(json,estr);
            }
			else
			{
				strcpy(estr,"not this command");
			}
        }
        else
        {
            strcpy(estr,"No the key of cmd");
        }
        json_decref(cmd);
    }
    else
    {
        strcpy(estr,"The Format is error");
    }
    return flag;
}

uint8 GetDeviceModuleName(json_t *json,char *estr)
{
#define MAXLINE 80
#define SERV_PORT 5000
	printf("get name\n");
	struct sockaddr_in servaddr;
    char buf[MAXLINE];
    int sockfd,n;
    char str[]="#model?\r\n";
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
	bzero(buf,sizeof(buf));
    servaddr.sin_family=AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr);
    servaddr.sin_port=htons(SERV_PORT);
    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    write(sockfd,str,strlen(str));
	printf("write\n");
    n=read(sockfd,buf,MAXLINE);
    printf("Response form server:%s\n",buf);
    json_object_set_new(json,"name",json_string(buf));
	close(sockfd);
	return flag;
}