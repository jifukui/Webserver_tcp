#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <jansson.h>
#include <unistd.h>
#include <sys/time.h>

typedef unsigned char uint8;
typedef char int8;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short int uint16;
typedef short int int16;

#define STATIC static
#define START 4
#define EXTPORT 2
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
STATIC uint8 PortImage(uint8 port,uint8 flag);
STATIC uint8 CmdStrHandler(uint8 *str,uint8 *buf);

STATIC uint8 GetDeviceModuleName(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetPortInfo(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetCardOnlineStatus(json_t *json,json_t* cmd,char *estr);
STATIC uint8 VideoSwitch(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetDeviceName(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetInputHDCPMOD(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetDeviceReset(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetDeviceFactory(json_t *json,json_t* cmd,char *estr);
STSTIC uint8 GetPortEDID(json_t *json,json_t* cmd,char *estr);


typedef uint8 (*CMD_FUNC)(json_t *json,json_t* cmd,char * estr);
typedef struct{
	char CommandName[30];
	CMD_FUNC CmdHandler;
}LigCommandHandler;
LigCommandHandler CommandHandler[]={
	{"PortInfo",&GetPortInfo},
	{"PortOnline",&GetCardOnlineStatus},
	{"matrix_status",&GetDeviceModuleName},
	{"VideoSetting",&VideoSwitch},
	{"SetDeviceName",&SetDeviceName},
	{"SetInputHDCPMOD",&SetInputHDCPMOD},
	{"SetDeviceReset",&SetDeviceReset},
	{"SetDeviceFactory",&SetDeviceFactory},
	{"GetPortEDID",&GetPortEDID},
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

uint8 PortImage(uint8 port,uint8 flag)
{
	uint8 index;
	if(flag)
	{
		if(port<=(LigPortNum/2))
		{
			index=port+(LigPortNum/2);
		}
		else if(port<=LigPortNum)
		{
			index=port+LigPortNum;
		}
		else if(port==(LigPortNum+1))
		{
			index=LigPortNum*2+2;
		}
	}
	else
	{
		if(port<=(LigPortNum/2))
		{
			index=port;
		}
		else if(port<=LigPortNum)
		{
			index=port+(LigPortNum/2);
		}
		else if(port==(LigPortNum+1))
		{
			index=LigPortNum*2+1;
		}
	}
	return index;
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
				//printf("The buf is %s\n",buf);
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
	uint8 flag;
	//pid_t pid;
	//pid=getpid();
	//printf("The father pid is %d \n",pid);
	flag=CommandHandle(sstr,json,estr);
	//pid=getpid();
	//printf("end pid is %d \n",pid);
	return flag;
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
		json_t *command;
        cmd=json_object_get(jsonget,"cmd");
		command=json_object_get(jsonget,"Data");
		char str[30];
        if(JsonGetString(cmd,str))
        {	
			json_error_t error;
			json_object_set_new(json,"cmd",json_string(str));
			for(i=0;i<length;i++)
			{
				if(!strcmp(str,CommandHandler[i].CommandName))
				{
					flag=(*CommandHandler[i].CmdHandler)(json,command,estr);
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
	//struct timeval start,end;
	//pid_t pid;
	//unsigned long time;
	bzero(rx,len);
	//slig_pip_read_bytes(sockfd,rx,len);
	//printf("The send buf is %s\n",tx);
	length=lig_pip_write_bytes(sockfd,tx,strlen(tx)+1);
	if(length>0)
	{
		length=0;
		//gettimeofday(&start,NULL);
		do{
        	length=lig_pip_read_bytes(sockfd,rx,len);
		}while(length==0);
		//gettimeofday(&end,NULL);
		//time=1000000*(end.tv_sec-start.tv_sec)+end.tv_usec-start.tv_usec;
		//printf("The time is %d\n",time);
		//pid =getpid();
		//printf("The child pid is %d \n",pid);
	}
	//printf("The recieve buf is %s\n",rx);
	return length;
}

uint8 GetDeviceModuleName(json_t *json,json_t* cmd,char *estr)
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
		json_object_set_new(json,"PortNumber",json_integer(LigPortNum));
	}
	else
	{
		strcpy(estr,"Not Get Model Name");
	}
	return flag;
}

uint8 GetPortInfo(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint32 data[2];
    char buf[4096];
    char str[30]="#SIGNAL? *\r\n";
	uint8 i;
	uint8 index;
	uint8 index1;
	uint8 status;
	json_t *portarr;
	json_t *portarr1;
	portarr=json_array();
	portarr1=json_array();
	json_t *portinfo,*portinfo1,*copy;
	portarr=json_array();
	portinfo=json_object();
	portinfo1=json_object();
	if(portarr!=NULL&&portinfo!=NULL&&portinfo1!=NULL&&portarr1!=NULL)
	{
		json_object_set_new(portinfo,"Linkstatus",json_false());
		json_object_set_new(portinfo,"PortIndedx",json_integer(0));
		PiPHandler(str,buf,sizeof(buf));
		for(i=0;i<(LigPortNum*2)+EXTPORT;i++)
		{
			json_object_set(portinfo,"PortIndedx",json_integer(i+1));
			copy=json_deep_copy(portinfo);
			json_array_append(portarr,copy);
		}
		for(i=0;i<=LigPortNum;i++)
		{
			flag=CmdStrHandler("SIGNAL",buf);
			if(flag)
			{
				status=sscanf(&buf[flag],"%d,%d\r\n",&data[0],&data[1]);
				memmove(buf,&buf[flag],sizeof(buf[flag]));
			}
			else
			{
				status=0;
			}
			if(status!=(sizeof(data)/sizeof(uint32)))
			{
				data[0]=LigPortNum+EXTPORT;
				data[1]=0;
			}
			if(data[0]<LigPortNum+EXTPORT)
			{
				if(data[1]==1)
				{
					printf("The port is %d\n",data[0]);
					index=PortImage(data[0],0);
					json_object_set(portinfo,"PortIndedx",json_integer(index));
					json_object_set(portinfo,"Linkstatus",json_true());
					copy=json_deep_copy(portinfo);
					json_array_set(portarr,index-1,copy);
				}
			}
		}
		strcpy(str,"#DISPLAY? *\r\n");
		PiPHandler(str,buf,sizeof(buf));
		for(i=0;i<=LigPortNum;i++)
		{
			flag=CmdStrHandler("DISPLAY",buf);
			if(flag)
			{
				status=sscanf(&buf[flag],"%d,%d\r\n",&data[0],&data[1]);
				memmove(buf,&buf[flag],sizeof(buf[flag]));
			}
			else
			{
				status=0;
			}
			if(status!=(sizeof(data)/sizeof(uint32)))
			{
				data[0]=LigPortNum+EXTPORT;
				data[1]=0;
			}
			if(data[0]<LigPortNum+EXTPORT)
			{
				if(data[1]>0)
				{
					//printf("The port is %d\n",data[0]);
					index=PortImage(data[0],1);
					json_object_set(portinfo,"PortIndedx",json_integer(index));
					json_object_set(portinfo,"Linkstatus",json_true());
					copy=json_deep_copy(portinfo);
					json_array_set(portarr,index-1,copy);
				}
			}
		}
		json_object_set(json,"LinkStatus",portarr);
		strcpy(str,"#VID? *\r\n");
		json_object_set_new(portinfo1,"InPort",json_integer(0));
		json_object_set_new(portinfo1,"OutPort",json_integer(0));
		PiPHandler(str,buf,sizeof(buf));
		for(i=0;i<=LigPortNum;i++)
		{
			if(i==0)
			{
				flag=CmdStrHandler("VID",buf);
			}
			else
			{
				flag=CmdStrHandler(",",buf);
			}
			
			if(flag)
			{
				status=sscanf(&buf[flag],"%d>%d",&data[0],&data[1]);
				memmove(buf,&buf[flag],sizeof(buf[flag]));
			}
			else
			{
				status=0;
			}
			if(status!=(sizeof(data)/sizeof(uint32)))
			{
				data[0]=LigPortNum+EXTPORT;
				data[1]=0;
			}
			if(data[0]<(LigPortNum+EXTPORT)&&data[1]<(LigPortNum+EXTPORT))
			{
				index=PortImage(data[0],0);
				index1=PortImage(data[1],1);
				json_object_set(portinfo1,"InPort",json_integer(index));
				json_object_set(portinfo1,"OutPort",json_integer(index1));
				copy=json_deep_copy(portinfo1);
				json_array_append(portarr1,copy);
			}
		}
		json_object_set(json,"VideoRouting",portarr1);
		flag=1;	
	}
	else
	{
		strcpy(estr,"Init json error\n");
	}
	return flag;
}

uint8 GetCardOnlineStatus(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint32 data[3];
	uint8 status;
	uint8 index=1;
    char buf[4096];
    char str[30]="#MODULE-TYPE? *\r\n";
	char buffer[80];
	uint8 i;
	json_t *portarr;
	portarr=json_array();
	json_t *portinfo,*copy;
	uint8 PortNum=LigPortNum/8;
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
				memmove(buf,&buf[flag],sizeof(buf[flag]));
			}
			else
			{
				status=0;
			}
			if(status!=(sizeof(data)/sizeof(uint32)))
			{
				data[0]=4;
				data[1]=4;
				data[2]=4;
			}
			for(flag=1;flag<=PortNum;flag++)
			{
				json_object_set(portinfo,"PortIndex",json_integer(PortNum*i+flag-1));
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
	else
	{
		strcpy(estr,"Init json error\n");
	}
	return flag;
}
uint8 VideoSwitch(json_t *json,json_t* cmd,char *estr)
{
	uint flag=0;
	if(cmd)
	{
		json_t *Inport;
		json_t *Outport;
		json_t *port;
		uint32 in,out;
		uint8 str[4096];
		uint8 buf[4096];
		uint8 cmdbuf[80];
		uint8 length;
		uint8 i;
		uint8 status;
		Inport=json_object_get(cmd,"Inport");
		if(Inport)
		{
			if(JsonGetInteger(Inport,&in))
			{
				if(in<LigPortNum+EXTPORT)
				{
					Outport=json_object_get(cmd,"Outport");
					if(json_typeof(Outport)==JSON_ARRAY)
					{
						length=json_array_size(Outport);
						//printf("The length is %d\n",length);
						if(length==0)
						{
							flag=1;
						}
						else
						{
							//printf("start buf\n");
							strcpy(str,"#VID ");
							for(i=0;i<length;i++)
							{
								port=json_array_get(Outport,i);
								if(JsonGetInteger(port,&out))
								{
									if(out<LigPortNum+EXTPORT&&out!=0)
									{
										status=1;
										sprintf(cmdbuf,"%d>%d,",in,out);
										strcat(str,cmdbuf);
									}
								}
							}
							if(status)
							{
								//printf("good this\n");
								str[strlen(str)-1]=NULL;
								strcat(str,"\r\n");
								//printf("The str is %s\n",str);
								PiPHandler(str,buf,sizeof(buf));
								flag=CmdStrHandler("VID",buf);
								flag=CmdStrHandler("ERR",&buf[flag]);
								//printf("The err is %d\n",flag);
								flag=!flag;
							}
							else
							{
								strcpy(estr,"Outport error ");
							}
						}
					}
					else
					{
						strcpy(estr,"Outport is not array ");
					}
				}
				else
				{
					strcpy(estr,"Inport out of range ");
				}
			}
			else
			{
				strcpy(estr,"Inport is not integer");
			}
		}
		else
		{
			strcpy(estr,"Not The Inport");
		}
	}
	else
	{
		strcpy(estr,"Not The data");
	}
	return flag;
}

uint8 SetDeviceName(json_t *json,json_t* cmd,char *estr)
{
	uint flag=0;
	json_t *name;
	char namebuf[64];
	char sendbuf[256];
	char buf[256];
	uint32 data;
	uint8 status;
	name=json_object_get(cmd,"Name");
	if(JsonGetString(name,namebuf))
	{	
		sprintf(sendbuf,"#NAME %s\r\n",namebuf);
		PiPHandler(sendbuf,buf,sizeof(buf));
		if(CmdStrHandler("NAME",buf))
		{
			status=sscanf(buf,"NAME ERR %d\r\n",&data);
			flag=!status;
		}
		else
		{
			strcpy(estr,"Not get name");
		}
	}
	return flag;
}

uint8 SetInputHDCPMOD(json_t *json,json_t* cmd,char *estr)
{
	uint flag=0;
	json_t *name;
	uint32 in,mod;
	char sendbuf[256];
	char buf[256];
	uint32 data;
	uint8 status;
	name=json_object_get(cmd,"Inport");
	if(JsonGetInteger(name,&in)&&in<=(LigPortNum+1))
	{	
		name=json_object_get(cmd,"mode");
		if(JsonGetInteger(name,&mod)&&mod<=1)
		{
			sprintf(sendbuf,"#HDCP-MOD %d,%d\r\n",in,mod);
			PiPHandler(sendbuf,buf,sizeof(buf));
			if(CmdStrHandler("HDCP-MOD",buf))
			{
				status=sscanf(buf,"HDCP-MOD ERR %d\r\n",&data);
				flag=!status;
			}
			else
			{
				strcpy(estr,"Not get name of HDCP-MOD");
			}
		}
		else
		{
			strcpy(estr,"Get mode Error");
		}
	}
	else
	{
		strcpy(estr,"Get Inport Error");
	}
	return flag;
}

uint8 SetDeviceReset(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=1;
	char buf[256]="#RESET\r\n";
	PiPHandler(sendbuf,buf,sizeof(buf));
	return flag;
}

uint8 SetDeviceFactory(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=1;
	char buf[256]="#FACTORT\r\n";
	PiPHandler(sendbuf,buf,sizeof(buf));
	return flag;
}

uint8 GetPortEDID(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	json_t obj;
	char buf[1024];
	char str[80];
	uint32 port,attr;
	uint32 length;
	if(cmd)
	{
		obj=json_object_get(cmd,"port");
		if(JsonGetInteger(obj,&port))
		{
			obj=json_object_get(cmd,"attr");
			if(JsonGetInteger(obj,&attr))
			{
				if(attr==2)
				{
					port=port/(LigPortNum/8);
				}
				sprintf(str,"#GEDID %d,%d\r\n");
				PiPHandler(sendbuf,buf,sizeof(buf));
				printf("The buf is :%s \n",buf);
				length=lig_pip_write_bytes(sockfd,buf,sizeof(buf));
				if(length>0)
				{
					length=0;
					do{
        				length=lig_pip_read_bytes(sockfd,buf,sizeof(buf));
					}while(length==0);
				}
				printf("the length is %d\n",length);
				printf("The data is %s\n",buf);

			}
			else
			{
				strcpy(estr,"get attr error");
			}
			
		}
		else
		{
			strcpy(estr,"get port error");
		}
		
	}
	else
	{
		strcpy(estr,"error get Data");
	}
	
	return flag;
}