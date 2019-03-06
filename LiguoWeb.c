#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <jansson.h>
#include <unistd.h>
/*#include <sys/time.h>*/

typedef unsigned char uint8;
typedef char int8;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short int uint16;
typedef short int int16;

#define STATIC static
#define START 4
extern int sockfd;
extern unsigned int LigPortNum;

uint8 LiguoWeb_GET_Method(const char *sstr,json_t *json,char *estr);
uint8 LiguoWeb_POST_Method(const unsigned char *sstr,json_t *json,char *estr);
STATIC uint8 CommandHandle(const char *sstr,json_t *json,char *estr);


STATIC void Uint8toString(int8 *str,uint8 *data,uint32 length);
STATIC void StringtoUint8(uint8 *dis,int8 *str);
STATIC uint8 JsonGetString(json_t *json,char *data);
STATIC uint8 JsonGetInteger(json_t *json,uint32 *data);
STATIC uint8 JsonGetUint8(json_t *json,uint8 *data);
STATIC uint8 JsonFromFile(uint8 *filepath,uint8 *data);
STATIC uint8 CmdStrHandler(uint8 *str,uint8 *buf);

STATIC uint8 GetDeviceModuleName(json_t *json,char *estr);
STATIC uint8 GetDeviceLinkStatus(json_t *json,char *estr);
STATIC uint8 GetCardOnlineStatus(json_t *json,char *estr);
typedef uint8 (*CMD_FUNC)(json_t *json,char * estr);
typedef struct{
	char CommandName[30];
	CMD_FUNC CmdHandler;
}LigCommandHandler;
LigCommandHandler CommandHandler[]={
	{"LinkStatus",&GetDeviceLinkStatus},
	{"CardOnline",&GetCardOnlineStatus},
	{"matrix_status",&GetDeviceModuleName}
};
STATIC uint32 PiPHandler(char *tx,char *rx,uint32 len);



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

uint8 CmdStrHandler(uint8 *str,uint8 *buf)
{
	uint8 flag=0;
	uint8 i;
	char *data=NULL;
	data=strstr(buf,str);
	if(data)
	{
		for(i=(strlen(str));i<(strlen(data));i++)
		{
			if(data[i]!=' ')
			{
				flag=i;
				strcpy(buf,data);
				printf("The buf is %s\n",buf);
				data=NULL;
				break;
			}
		}
	}
	return flag;
}


uint8 LiguoWeb_GET_Method(const char *sstr,json_t *json,char *estr)
{
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
	return CommandHandle(sstr,json,estr);
}

uint8 CommandHandle(const char *sstr,json_t *json,char *estr)
{
	json_error_t error;
    json_t *jsonget;
    jsonget=json_loads(sstr,0,&error);
    uint8 flag=0;
	int8 data[20000];
	uint8 i;
	uint8 length;
	length=((sizeof(CommandHandler)/sizeof(LigCommandHandler)));
    if(jsonget)
    {
        json_t *cmd;
        cmd=json_object_get(jsonget,"cmd");
		char str[30];
        if(JsonGetString(cmd,str))
        {	
			json_error_t error;
			json_object_set_new(json,"cmd",json_string(str));
			for(i=0;i<length;i++)
			{
				if(!strcmp(str,CommandHandler[i].CommandName))
				{
					flag=(*CommandHandler[i].CmdHandler)(json,estr);
					break;
				}
			}
			if(i>=length)
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

uint32 PiPHandler(char *tx,char *rx,uint32 len)
{
	uint32 length;
	lig_pip_read_bytes(sockfd,rx,len);
	length=lig_pip_write_bytes(sockfd,tx,strlen(tx)+1);
	if(length>0)
	{
		length=0;
		bzero(rx,len);
		do{
        	length=lig_pip_read_bytes(sockfd,rx,len);
		}while(length==0);
	}
	return length;
}

uint8 GetDeviceModuleName(json_t *json,char *estr)
{
	uint8 flag=0;
	uint8 i;
    char buf[80];
    char str[]="#model?\r\n";	
	PiPHandler(str,buf,sizeof(buf));
	buf[strlen(buf)-2]=NULL;
	flag=CmdStrHandler("MODEL",buf);
	if(flag)
	{
		json_object_set_new(json,"name",json_string(&buf[flag]));
	}
	else
	{
		strcpy(estr,"Not Get Model Name");
	}
	
	return flag;
}

uint8 GetDeviceLinkStatus(json_t *json,char *estr)
{
	uint8 flag=0;
	uint32 data[3];
    char buf[4096];
    char str[30]="#signal? *\r\n";
	uint8 i;
	uint8 PortNum=LigPortNum/8;
	json_t *portarr;
	portarr=json_array();
	json_t *portinfo[PortNum],copy;
	if(!portarr)
	{
		strcpy(estr,"Init array error\n");
	}
	for(i=0;i<PortNum;i++)
	{
		portinfo[i]=json_object();
		if(!portinfo[i])
		{
			break;
		}
	}
	if(i=PortNum-1)
	{
		for(i=1;i<17;i++)
		{
			sprintf(str,"#MODULE-TYPE? %d\r\n",i);
			PiPHandler(str,buf,sizeof(buf));
			flag=CmdStrHandler("MODULE-TYPE",buf);
			sscanf(&buf[flag],"%d,%d,%d\r\n",&data[0],&data[1],&data[2]);
			/*printf("The data 1 is %d\n",data[0]);
			printf("The data 2 is %d\n",data[1]);
			printf("The data 3 is %d\n",data[2]);*/
			json_object_set_new(json,"Data",json_string(buf));

		}
	}

	/*PiPHandler(str,buf,sizeof(buf));
	json_object_set_new(json,"In",json_string(buf));
	strcpy(str,"#display? *\r\n");
	printf("The str is %s\n",str);
	PiPHandler(str,buf,sizeof(buf));*/
	//json_object_set_new(json,"Out",json_string(buf));
	return flag;
}

uint8 GetCardOnlineStatus(json_t *json,char *estr)
{
	uint8 flag=0;
	uint32 data[3];
	uint8 status;
	uint8 index=1;
    char buf[4096];
    char str[30]="#MODULE-TYPE? *\r\n";
	char buffer[80];
	uint8 i;
	uint8 PortNum=LigPortNum/8;
	json_t *portarr;
	portarr=json_array();
	json_t *portinfo,*copy;
	if(!portarr)
	{
		strcpy(estr,"Init array error\n");
	}
	portinfo=json_object();
	if(portinfo!=NULL)
	{
		json_object_set_new(portinfo,"PortIndex",json_integer(0));
		json_object_set_new(portinfo,"OnlineStatus",json_false());
		PiPHandler(str,buf,sizeof(buf));
		i=0;
		do{
			flag=CmdStrHandler("MODULE-TYPE",buf);
			if(flag)
			{
				status=sscanf(&buf[flag],"%d,%d,%d\r\n",&data[0],&data[1],&data[2]);
				buf=&buf[flag];
			}
			else
			{
				status=0;
			}
			printf("The flag is %d\n",flag);
			printf("The status is %d\n",status);
			if(status!=3)
			{
				data[0]=4;
				data[1]=4;
				data[2]=4;
			}
			
			printf("The data 1 is %d\n",data[0]);
			printf("The data 2 is %d\n",data[1]);
			printf("The data 3 is %d\n",data[2]);
			for(flag=1;flag<=PortNum;flag++)
			{
				json_object_set(portinfo,"PortIndex",json_integer(PortNum*i+flag));
				if(data[2]==0)
				{
					json_object_set(portinfo,"OnlineStatus",json_true());
				}
				else
				{
					json_object_set(portinfo,"OnlineStatus",json_false());
				}
				copy=json_deep_copy(portinfo);
				json_array_append(portarr,copy);
			}
			++i;
		}while(i<16);
		json_object_set_new(json,"Data",portarr);	
	}
	return flag;
}