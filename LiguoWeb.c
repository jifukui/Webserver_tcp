#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <jansson.h>
#include <unistd.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
typedef unsigned char uint8;
typedef char int8;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short int uint16;
typedef short int int16;
typedef unsigned long long uint64;
typedef long long int64;

#define STATIC static
#define START 4
#define EXTPORT 2
#define MAXBYTE 2048
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
STATIC uint8 Port2Phy(uint8 port);
STATIC uint8 CmdStrHandler(uint8 *str,uint8 *buf);
STATIC void J2Uppercase(uint8 *str,uint8 *buf);

STATIC uint8 GetDeviceModuleName(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetPortInfo(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetCardOnlineStatus(json_t *json,json_t* cmd,char *estr);
STATIC uint8 VideoSwitch(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetDeviceName(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetInputHDCPMOD(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetDeviceReset(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetDeviceFactory(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetPortEDID(json_t *json,json_t* cmd,char *estr);
STATIC uint8 CopyPortEDID(json_t *json,json_t* cmd,char *estr);
STATIC uint8 LoadEDID(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetNetwork(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetPortFunc(json_t *json,json_t* cmd,char *estr);
STATIC uint8 SetDHCPStatus(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetHDCPStatus(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetUpgradeFileName(json_t *json,json_t* cmd,char *estr);
STATIC uint8 Upgrade(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetSlotUart(json_t *json,json_t* cmd,char *estr);
STATIC uint8 GetStaticNetWork(json_t *json,json_t* cmd,char *estr);

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
	{"CopyPortEDID",&CopyPortEDID},
	{"LoadEDID",&LoadEDID},
	{"SetNetwork",&SetNetwork},
	{"SetPortFunc",&SetPortFunc},
	{"SetDHCPStatus",&SetDHCPStatus},
	{"GetHDCPStatus",&GetHDCPStatus},
	{"GetUpgradeFileName",&GetUpgradeFileName},
	{"Upgrade",&Upgrade},
	{"GetSlotUart",&GetSlotUart},
	{"GetStaticNetWork",&GetStaticNetWork}
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

uint8 Port2Phy(uint8 port)
{
	uint8 flag=0;
	uint8 Port=LigPortNum/2;
	if(port<=Port)
	{
		flag=port;
	}
	else if(port<=2*Port)
	{
		flag=port-Port;
	}
	else if(port<=3*Port)
	{
		flag=port-Port;
	}	
	else if(port<=4*Port)
	{
		flag=port-2*Port;
	}
	else if(port<=4*Port+2)
	{
		flag=LigPortNum+1;
	}
	return flag;
}

/**找到str是否在buf中如果在buf中设置buf的值为str开始的字符串，并返回找到的第一个非空格的位置*/
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
				data=NULL;
				break;
			}
		}
	}
	return flag;
}

void J2Uppercase(uint8 *str,uint8 *buf)
{
	uint16 i=0;
	while(*str)
	{
		if(*str==' ')
		{
			str++;
			continue;
		}
		if(*str>='a'&&*str<='z')
		{
			*buf=(*str)&0xdf;
		}
		else
		{
			*buf=*str;
		}
		//printf("The old is %c new is %c\n",*str,*buf);
		str++;
		buf++;
	}
	*buf=NULL;
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
	uint8 flag=0;
	flag=CommandHandle(sstr,json,estr);
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
        json_t *cmd=NULL;
		json_t *command=NULL;
		json_t *cpy=NULL;
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
					printf("handler over\n");
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
    }
    else
    {
        strcpy(estr,"The Format is error");
    }
    return flag;
}

uint32 PiPHandler(char *tx,char *rx,uint32 len)
{
	uint32 length=0;
	uint32 status=0;
	int32 flag=0;
	//struct timeval start,end;
	//unsigned long time;
	/*do{
		length=lig_pip_read_bytes(sockfd,rx,len);
	}while(length);*/
	//printf("The length is %d\n",length);
	printf("The send buf is %s",tx);
	length=lig_pip_read_bytes(sockfd,rx,len);
	//printf("The length is %d\n",length);
	bzero(rx,len);
	length=lig_pip_write_bytes(sockfd,tx,strlen(tx)+1);
	if(length>0)
	{
		length=0;
		//gettimeofday(&start,NULL);
		if(LigPortNum==64)
		{
			usleep(90000);
		}
		do{
			//usleep(20);
        	length=lig_pip_read_bytes(sockfd,rx,len);
		}while(length==0);
		
		/*if(LigPortNum==64)
		{
			//usleep(20000);
			do{
				status=lig_pip_read_bytes(sockfd,&rx[length],len-length-1);
				length+=status;
				printf("The length  2 is %d\n",length);
			}while(status);
		}*/
		//printf("The length  3 is %d\n",length);
		//gettimeofday(&end,NULL);
		//time=1000000*(end.tv_sec-start.tv_sec)+end.tv_usec-start.tv_usec;
		//printf("The time is %d\n",time);
		//pid =getpid();
		//printf("The child pid is %d \n",pid);
	}
	printf("The recieve buf is %s",rx);
	return length;
}

uint8 GetDeviceModuleName(json_t *json,json_t* cmd,char *estr)
{
	
	uint8 flag=0;
	uint8 i;
    char buf[80];
    char str[]="#MODEL?\r\n";	
	PiPHandler(str,buf,sizeof(buf));
	buf[strlen(buf)-2]=NULL;
	flag=CmdStrHandler("MODEL",buf);
	if(flag)
	{
		json_t *name=NULL;
		json_t *portnum=NULL;
		json_t *version=NULL;
		json_t *sn=NULL;
		json_object_set_new(json,"name",json_string(&buf[flag]));
		
		json_object_set_new(json,"PortNumber",json_integer(LigPortNum));
		
		strcpy(str,"#VERSION?\r\n");
		PiPHandler(str,buf,sizeof(buf));
		buf[strlen(buf)-2]=NULL;
		flag=CmdStrHandler("VERSION",buf);
		if(flag)
		{
			/**下面这两段有错误需要进行*/
			//version=json_string(&buf[flag]);
			//json_object_set_new(json,"version",version);
			//json_decref(version);
			json_object_set_new(json,"version",json_string(&buf[flag]));

			strcpy(str,"#SN?\r\n");
			PiPHandler(str,buf,sizeof(buf));
			buf[strlen(buf)-2]=NULL;
			flag=CmdStrHandler("SN",buf);
			if(flag)
			{
				sn=json_string(&buf[flag]);
				json_object_set_new(json,"sn",sn);
				json_object_set_new(json,"sn",json_string(&buf[flag]));
			}
			else
			{
				strcpy(estr,"Get Version Error");
			}
		}
		else
		{
			strcpy(estr,"Get Version Error");
		}
		//json_decref(portnum);
		//json_decref(sn);
		//json_decref(name);
		//json_decref(version);
		
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
    char buf[MAXBYTE];
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
		/*copy=json_false();
		json_object_set_new(portinfo,"Linkstatus",copy);
		json_decref(copy);*/
		json_object_set_new(portinfo,"Linkstatus",json_false());
		/*copy=json_integer(0);
		json_object_set_new(portinfo,"PortIndex",copy);
		json_decref(copy);*/
		json_object_set_new(portinfo,"PortIndex",json_integer(0));
		PiPHandler(str,buf,sizeof(buf));
		for(i=0;i<(LigPortNum*2)+EXTPORT;i++)
		{
			/*copy=json_integer(i+1);
			json_object_set(portinfo,"PortIndex",copy);
			json_decref(copy);*/
			json_object_set(portinfo,"PortIndex",json_integer(i+1));
			copy=json_deep_copy(portinfo);
			json_array_append(portarr,copy);
			//json_decref(copy);
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
				strcpy(estr,"Get Data Error");
				return flag;
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
					index=PortImage(data[0],0);
					/*copy=json_integer(index);
					json_object_set(portinfo,"PortIndex",copy);
					json_decref(copy);*/
					json_object_set(portinfo,"PortIndex",json_integer(index));
					/*copy=json_true();
					json_object_set(portinfo,"Linkstatus",copy);
					json_decref(copy);*/
					json_object_set(portinfo,"Linkstatus",json_true());
					copy=json_deep_copy(portinfo);
					json_array_set(portarr,index-1,copy);
					//json_decref(copy);
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
				strcpy(estr,"Get Data Error");
				return flag;
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
					index=PortImage(data[0],1);
					/*copy=json_integer(index);
					json_object_set(portinfo,"PortIndex",copy);
					json_decref(copy);*/
					json_object_set(portinfo,"PortIndex",json_integer(index));
					/*copy=json_true();
					json_object_set(portinfo,"Linkstatus",json_true());
					json_decref(copy);*/
					json_object_set(portinfo,"Linkstatus",json_true());
					copy=json_deep_copy(portinfo);
					json_array_set(portarr,index-1,copy);
					//json_decref(copy);
				}
			}
		}
		json_object_set(json,"LinkStatus",portarr);
		strcpy(str,"#VID? *\r\n");
		/*copy=json_integer(0);
		json_object_set_new(portinfo1,"InPort",copy);
		json_object_set_new(portinfo1,"OutPort",copy);
		json_decref(copy);*/
		json_object_set_new(portinfo1,"InPort",json_integer(0));
		json_object_set_new(portinfo1,"OutPort",json_integer(0));
		/*copy=json_true();
		json_object_set(portinfo,"Linkstatus",copy);
		json_decref(copy);*/
		json_object_set(portinfo,"Linkstatus",json_true());
		
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
				strcpy(estr,"Get Data Error");
				return flag;
			}
			if(status!=(sizeof(data)/sizeof(uint32)))
			{
				data[0]=LigPortNum+EXTPORT;
				data[1]=0;
			}
			if(data[0]<(LigPortNum+EXTPORT)&&data[1]<(LigPortNum+EXTPORT))
			{
				index=PortImage(data[0],0);
				/*copy=json_integer(index);
				json_object_set(portinfo1,"InPort",json_integer(index));
				json_decref(copy);*/
				json_object_set(portinfo1,"InPort",json_integer(index));
				index1=PortImage(data[1],1);
				/*copy=json_integer(index1);
				json_object_set(portinfo1,"OutPort",copy);
				json_decref(copy);*/
				json_object_set(portinfo1,"OutPort",json_integer(index1));
				copy=json_deep_copy(portinfo1);
				json_array_append(portarr1,copy);
			}
		}
		json_object_set(json,"VideoRouting",portarr1);
		/*json_decref(copy);
		json_decref(portarr);
		json_decref(portarr1);
		json_decref(portinfo);
		json_decref(portinfo1);*/
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
    char buf[MAXBYTE];
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
				strcpy(estr,"Get Data Error");
				return flag;
			}
			//printf("the buf is %s\n",buf);
			//printf("The status is %d\n",status);
			if(status!=(sizeof(data)/sizeof(uint32)))
			{
				data[0]=4;
				data[1]=4;
				data[2]=4;
			}
			//printf("The 3 is %d\n",data[2]);
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
		uint8 str[MAXBYTE];
		uint8 buf[MAXBYTE];
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
						if(length==0)
						{
							flag=1;
						}
						else
						{
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
								str[strlen(str)-1]=NULL;
								strcat(str,"\r\n");
								PiPHandler(str,buf,sizeof(buf));
								flag=CmdStrHandler("VID",buf);
								flag=CmdStrHandler("ERR",&buf[flag]);
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
			//printf("The buf is %s",buf);
			status=sscanf(buf,"NAME ERR %d",&data);
			printf("NAME status is  %d\n",status);
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
				status=sscanf(&buf[START],"HDCP-MOD ERR %d\r\n",&data);
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
	char data[256];
	PiPHandler(buf,data,sizeof(data));
	return flag;
}

uint8 SetDeviceFactory(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=1;
	char buf[256]="#FACTORY\r\n";
	char data[256];
	PiPHandler(buf,data,sizeof(data));
	//printf("The data is %s\n",data);
	return flag;
}

uint8 GetPortEDID(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	json_t *obj;
	uint8 buf[1024];
	uint8 str[1024];
	uint32 port,attr;
	uint32 length;
	uint8 status;
	uint32 len;
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
					port=(port-1)/(LigPortNum/8)+1;
				}
				else
				{
					port=Port2Phy(port);
				}
				sprintf(str,"#GEDID %d,%d\r\n",attr,port);
				PiPHandler(str,buf,sizeof(buf));
				status=sscanf(&buf[START],"GEDID %d,%d,%d\r\n",&attr,&port,&len);
				if(status==3)
				{
					if(attr==2)
					{
						for(length=0;length<50;length++)
						{
							if(buf[length]=='\n')
							{
								flag=length+1;
								break;
							}
						}
						memmove(buf,&buf[flag],len);
					}
					else
					{
						length=0;
						do{
        					length=lig_pip_read_bytes(sockfd,buf,sizeof(buf));
						}while(length==0);
					}
					
					
					bzero(str,sizeof(str));
					Uint8toString(str,buf,len);
					json_object_set_new(json,"EDID",json_string(str));
					flag=1;
				}
				else
				{
					strcpy(estr,"Get EDID ERROR");
				}
				
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

uint8 CopyPortEDID(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint8 str[80];
	uint8 buf[80];
	uint32 in;
	uint32 type;
	uint32 out=0;
	uint32 bitmap[2]={0,0};
	uint32 status;
	json_t *obj;
	json_t * arr;
	if(cmd)
	{
		obj=json_object_get(cmd,"org");
		if(JsonGetInteger(obj,&in))
		{			
			if(in)
			{
				obj=json_object_get(cmd,"type");
				if(JsonGetInteger(obj,&type))
				{
					if(type<=2)
					{
						if(type==2)
						{
							in=(in-1)/(LigPortNum/8)+1;
						}
						else
						{
							in=Port2Phy(in);
						}
						arr=json_object_get(cmd,"dim");
						if(json_typeof(arr)==JSON_ARRAY)
						{
							uint8 i=0;
							for(i=0;i<json_array_size(arr);i++)
							{
								obj=json_array_get(arr,i);
								if(JsonGetInteger(obj,&out))
								{
									out=Port2Phy(out);
									if(out)
									{
										out-=1;
										if(out<32)
										{
											bitmap[0]|=1<<out;
										}
										else
										{
											bitmap[1]|=1<<(out-32);
										}
									}
								}
							}
							if(bitmap[0]||bitmap[1])
							{
								sprintf(str,"#CPEDID %d,%d,0,0x%08x%08x\r\n",type,in,bitmap[1],bitmap[0]);
								PiPHandler(str,buf,sizeof(buf));
								status=sscanf(&buf[START],"CPEDID ERR,%d\r\n",&type);
								flag=!status;
							}
							else
							{
								flag=1;
							}
						}
						else
						{
							strcpy(estr,"Get dim Error");
						}
					}
					else
					{
						strcpy(estr,"type out of range ");
					}
				}
				else
				{
					strcpy(estr,"Get type Error");
				}
			}
			else
			{
				strcpy(estr,"org out of range");
			}
		}
		else
		{
			strcpy(estr,"Get org Error");
		}
	}
	else
	{
		strcpy(estr,"error get Data");
	}
	return flag;
}

uint8 LoadEDID(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint8 str[1024];
	uint8 data[1024];
	uint8 edid[1024];
	uint8 buf[80];
	uint32 in;
	uint32 bitmap[2]={0,0};
	uint32 status;
	uint32 len;
	json_t *obj;
	json_t * arr;
	if(cmd)
	{
		arr=json_object_get(cmd,"dim");
		if(json_typeof(arr)==JSON_ARRAY)
		{
			uint16 i=0;
			for(i=0;i<json_array_size(arr);i++)
			{
				obj=json_array_get(arr,i);
				if(JsonGetInteger(obj,&in))
				{
					in=Port2Phy(in);
					if(in)
					{
						in-=1;
						if(in<32)
						{
							bitmap[0]|=1<<in;
						}
						else
						{
							bitmap[1]|=1<<(in-32);
						}
						//bitmap|=(1<<in);
					}
				}
			}
			if(bitmap[0]||bitmap[1])
			{
				obj=json_object_get(cmd,"EDID");
				if(JsonGetString(obj,data))
				{
					len=strlen(data)/2;
					StringtoUint8(&edid[4],data);
					sprintf(str,"#LDEDID 0,0x%08x%08x,%d,1\r\n",bitmap[1],bitmap[0],len);
					PiPHandler(str,buf,sizeof(buf));
					if(strstr(buf,"READY"))
					{
						bzero(str,sizeof(str));
						edid[0]=0;
						edid[1]=1;
						edid[2]=(len+2)/256;
						edid[3]=(len+2)%256;
						edid[len+4]=0xAA;
						edid[len+5]=0x55;
						len=lig_pip_write_bytes(sockfd,edid,len+6);
						if(len)
						{
							len=0;
							do{
								len=lig_pip_read_bytes(sockfd,buf,sizeof(buf));
								if(len>0)
								{
									if(strstr(buf,"ERR"))
									{
										strcpy(estr,"second command error");
										break;
									}
									else if(sscanf(&buf[START],"LDEDID %d,0x%llx,%d,%d OK\r\n",&in,&bitmap,&len,&status))
									{
										flag=1;
										break;
									}
								}
							}while(1);
						}
						else
						{
							strcpy(estr,"send second command error");
						}
					}
					else
					{
						strcpy(estr,"First command error");
					}
				}
				else
				{
					strcpy(estr,"Get EDID Error");
				}
				
			}
			else
			{
				flag=1;
			}
		}
		else
		{
			strcpy(estr,"Get Dim Error");
		}
	}
	else
	{
		strcpy(estr,"error get Data");
	}
	return flag;
}

uint8 SetNetwork(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint8 str[1024];
	uint8 buf[1024];
	uint8 ip[100];
	uint8 mask[100];
	uint8 gateway[100];
	uint32 tcp;
	uint32 udp;
	//uint32 status=0;
	json_t *obj;
	if(cmd)
	{
		obj=json_object_get(cmd,"ip");
		if(JsonGetString(obj,ip))
		{
			obj=json_object_get(cmd,"mask");
			if(JsonGetString(obj,mask))
			{
				obj=json_object_get(cmd,"gateway");
				if(JsonGetString(obj,gateway))
				{
					flag=1;
				}
			}
		}
		if(flag)
		{
			sprintf(str,"#NET-CONFIG 0,%s,%s,%s\r\n",ip,mask,gateway);
			PiPHandler(str,buf,sizeof(buf));
			if(strstr(buf,"ERR"))
			{
				flag=0;
				strcpy(estr,"set net work error");
			}
		}
		obj=json_object_get(cmd,"tcp");
		if(JsonGetInteger(obj,&tcp))
		{
			sprintf(str,"#ETH-PORT TCP,%d\r\n",tcp);
			PiPHandler(str,buf,sizeof(buf));
			if(strstr(buf,"ERR"))
			{
				strcpy(estr,"set tcp error");
			}
			else
			{
				flag=1;
			}
		}
		obj=json_object_get(cmd,"udp");
		if(JsonGetInteger(obj,&tcp))
		{
			sprintf(str,"#ETH-PORT UDP,%d\r\n",tcp);
			PiPHandler(str,buf,sizeof(buf));
			if(strstr(buf,"ERR"))
			{
				strcpy(estr,"set UDP error");
			}
			else
			{
				flag=1;
			}
		}

	}
	else
	{
		strcpy(estr,"error get Data");
	}
	return flag;
}

uint8 SetPortFunc(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint8 str[1024];
	uint8 buf[1024];
	uint32 index;
	uint32 sid;
	uint32 dir;
	uint32 value;
	uint8 i=0;
	json_t *obj;
	json_t *data;
	uint32 errorid[128];
	uint8 error=0;
	if(cmd)
	{
		if(json_typeof(cmd)==JSON_ARRAY)
		{
			for(i=0;i<json_array_size(cmd);i++)
			{
				data=json_array_get(cmd,i);
				if(json_typeof(data)==JSON_OBJECT)
				{
					obj=json_object_get(data,"sid");
					if(JsonGetInteger(obj,&sid))
					{
						obj=json_object_get(data,"index");
						if(JsonGetInteger(obj,&index))
						{
							index=Port2Phy(index);
							if(index)
							{
								obj=json_object_get(data,"dir");
								if(JsonGetInteger(obj,&dir))
								{
									obj=json_object_get(data,"value");
									if(JsonGetInteger(obj,&value))
									{
										sprintf(str,"#MODULE-FUNC %d,%d,%d,%d\r\n",sid,dir,index,value);
										PiPHandler(str,buf,sizeof(buf));
										if(strstr(buf,"ERR"))
										{
											//strcpy(estr,"set Module function error");
											errorid[error++]=sid;
										}
										else
										{
											flag=1;
										}
									}
									else
									{
										strcpy(estr,"Get value error");
									}
								}
								else
								{
									strcpy(estr,"Get dir error");
								}	
							}
							else
							{
								strcpy(estr,"Get index error");
							}	
						}
					}
					else
					{
						strcpy(estr,"Get sid error");
					}
				}
				else
				{
					strcpy(estr,"Data index is not Object");
				}
			}
			
		}
		else
		{
			strcpy(estr,"Data is not array");
		}
	}
	else
	{
		strcpy(estr,"error get Data");
	}
	for(i=0;i<error;i++)
	{
		sprintf(estr+strlen(estr),"%d,",errorid[i]);
	}
	if(error!=0)
	{
		estr[strlen(estr)-1]=NULL;
		printf("The error data is %s\r\n",estr);
	}
	return flag;
}

uint8 SetDHCPStatus(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint32 dhcp;
	json_t * obj;
	uint8 str[100];
	uint8 buf[100];
	if(cmd)
	{
		obj=json_object_get(cmd,"dhcp");
		if(JsonGetInteger(obj,&dhcp))
		{
			if(dhcp)
			{
				dhcp=1;
			}
			sprintf(str,"#NET-DHCP %d\r\n",dhcp);
			PiPHandler(str,buf,sizeof(buf));
			if(strstr(buf,"ERR"))
			{
				strcpy(estr,"set Module function error");
			}
			else
			{
				flag=1;
			}
		}
		else
		{
			strcpy(estr,"Data is not integer");
		}
	}
	else
	{
		strcpy(estr,"error get Data");
	}
	return flag;
}

uint8 GetHDCPStatus(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint32 data[3]={7,7,7};
    char buf[MAXBYTE];
    char str[30];
	json_t *portarr;
	json_t *portinfo,*copy;
	portarr=json_array();
	portinfo=json_object();
	uint8 i,n;
	uint32 status;
	uint32 index;
	if(portarr&&portinfo)
	{
		json_object_set_new(portinfo,"HDCPStatus",json_false());
		json_object_set_new(portinfo,"PortIndex",json_integer(0));
		for(i=0;i<(LigPortNum*2)+EXTPORT;i++)
		{
			json_object_set(portinfo,"PortIndex",json_integer(i+1));
			copy=json_deep_copy(portinfo);
			json_array_append(portarr,copy);
		}
		for(n=0;n<2;n++)
		{
			sprintf(str,"#HDCP-STAT? %d,*\r\n",n);
			PiPHandler(str,buf,sizeof(buf));
			//printf("The buf is %s\n",buf);
			for(i=0;i<=LigPortNum;i++)
			{
				flag=CmdStrHandler("HDCP-STAT",buf);
				if(flag)
				{
					status=sscanf(&buf[flag],"%d,%d,%d\r\n",&data[0],&data[1],&data[2]);
					memmove(buf,&buf[flag],sizeof(buf[flag]));
				}
				else
				{
					status=0;
					strcpy(estr,"Get Data Error");
					return flag;
				}
				//printf("status is %d\n",status);
				if(status!=(sizeof(data)/sizeof(uint32)))
				{
					data[0]=n;
					data[1]=i+1;
					data[2]=0;
				}
				//printf("%d,%d,%d\n",data[0],data[1],data[2]);
				if(data[0]==n&&data[1]==(i+1))
				{
					if(data[2]==1)
					{
						index=PortImage(data[1],data[0]);
						json_object_set(portinfo,"PortIndex",json_integer(index));
						json_object_set(portinfo,"HDCPStatus",json_true());
						copy=json_deep_copy(portinfo);
						json_array_set(portarr,index-1,copy);
					}
				}
			}
		}
		json_object_set(json,"HDCPStatus",portarr);
	}
	return flag;
}

uint8 GetUpgradeFileName(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint8 filename[1024];
	uint8 untarfilename[1024];
	uint8 newfilename[1024];
	uint8 newpath[1024];
	uint8 IsKmpt=0;
	char *str;
	json_t *untar;
	json_t *file;
	if(cmd)
	{
		file=json_object_get(cmd,"Filename");
		if(JsonGetString(file,filename))
		{
			str=strrchr(filename,'.');
			if(!strcasecmp(str,".kmpt")||!strcasecmp(str,".json"))
			{
				printf("match kmpt or json\n");
				IsKmpt=1;
				memcpy(untarfilename,filename,sizeof(filename));
			}
			else
			{
				FILE * fstream;
				sprintf(untarfilename,"unzip -o \"/tmp/www/%s\" -d /tmp/www  > /dev/null && ls -t /tmp/www/ | grep -i \".xml\"",filename);
				printf("The untarfilename is %s\n",untarfilename);
				if(NULL==(fstream=popen(untarfilename,"r"))||NULL==fgets(untarfilename,sizeof(untarfilename), fstream))    
				{    
					strcpy(estr,"untar file failed");
					return flag;	
				}
				pclose(fstream);
				untarfilename[strlen(untarfilename)-1]=NULL;
			}
			printf("the str is %d\n",strlen(untarfilename));
			printf("The untarfilename is %s\n",untarfilename);
			J2Uppercase(untarfilename,newfilename);
			printf("The new filename is %s\n",newfilename);
			sprintf(filename,"/tmp/www/%s",untarfilename);
			sprintf(newpath,"/tmp/www/%s",newfilename);
			// printf("The old file path is %s\n",filename);
			// printf("The new file path is %s\n",filename);
			if(rename(filename,newpath))
			{
				strcpy(estr,"rename error");
			}
			else
			{
				printf("good for this \n");
				struct stat file;
				stat(newpath,&file);
				json_object_set(json,"Filename",json_string(newfilename));
				json_object_set(json,"FileSize",json_integer(file.st_size));
				flag=1;
			}
			// str=strrchr(newfilename,'.');
			// printf("the str is %s\n",str);
			if(IsKmpt==1)
			{
				printf("have match kmpt\n");
				// flag=1;	
			}	
			else
			{
				sprintf(untarfilename,"mv %s /nandflash/thttpd/www/",newpath);
				printf("The value is %s\n",untarfilename);
				system(untarfilename);
			}
			return flag;		
		}
		else
		{
			strcpy(estr,"Get File  error");
		}
	}
	else
	{
		strcpy(estr,"not the file name");
	}
	return flag;
}

uint8 Upgrade(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint8 oldpath[512];
	uint8 newpath[512];
	uint8 oldfilename[256];
	uint8 newfilename[256];
	uint8 str[256];
	uint8 buf[256];
	struct stat jistat;
	uint32 jifile=0;
	uint32 length;
	uint32 data[3];
	uint32 status=0;
	json_t *file;
	if(cmd)
	{
		//printf("upgrade\n");
		file=json_object_get(cmd,"oldfile");
		if(JsonGetString(file,oldfilename))
		{
			//printf("The old file name is %s\n",oldfilename);
			file=json_object_get(cmd,"newfile");
			if(JsonGetString(file,newfilename))
			{
				//printf("The new file name is %s\n",newfilename);
				sprintf(oldpath,"/tmp/www/%s",oldfilename);
				sprintf(newpath,"/tmp/www/%s",newfilename);
				//printf("the old path is %s\n",oldpath);
				//printf("the new path is %s\n",newpath);
				jifile=rename(oldpath,newpath);
				//printf("The jifile is %d\n",jifile);
				if(jifile)
				{
					strcpy(estr,"rename file name error");
				}
				else
				{
					if(stat(newpath,&jistat))
					{
						strcpy(estr,"get file attribute error");
					}
					else
					{
						sprintf(str,"#MODULE-LOAD %s,%d\r\n",newfilename,jistat.st_size);
						//printf("the str is %s\n",str);
						PiPHandler(str,buf,sizeof(buf));
						printf("The first return is %s",buf);
						if(strstr(buf,"START"))
						{
							if(strstr(newfilename,"VS-1616DN-EM_VS-3232DN-EM_VS-6464DN-EM"))
							{
								printf("ctrl\n");
								flag=1;
							}
							else
							{
								printf("board\n");
								bzero(buf,sizeof(buf));
								do{
        							length=lig_pip_read_bytes(sockfd,buf,sizeof(buf));
								}while(length==0);
								printf("The first file is %s",buf);
								flag=CmdStrHandler("LOAD",buf);
								if(flag)
								{
									status=sscanf(&buf[flag],"%d,%d,%d %[OK]\r\n",&data[0],&data[1],&data[2],oldfilename);
									if(status==4)
									{
										memmove(buf,&buf[flag],sizeof(buf[flag]));
										if(CmdStrHandler("LOAD",buf))
										{
											printf("have second\n");
										}
										else
										{
											printf("no second\n");
											bzero(buf,sizeof(buf));
											do{
        										length=lig_pip_read_bytes(sockfd,buf,sizeof(buf));
											}while(length==0);
										}
										
										sprintf(str,"LOAD %s,%d OK",newfilename,jistat.st_size);
										printf("buf is %s\n",buf);
										printf("str is %s\n",str);
										if(strstr(buf,str))
										{
											flag=1;
										}
										else
										{
											flag=0;
											strcpy(estr,"upgrade2 failed");
										}	
									}
									else
									{
										flag=0;
										strcpy(estr,"upgrade1 failed");
									}	
								}
								else
								{
									strcpy(estr,"upgrade failed");
								}
							}
						}
						else
						{
							strcpy(estr,"upgrade start error");
						}
					}
				}
			}
			else
			{
				strcpy(estr,"get new file name error");
			}
		}
		else
		{
			strcpy(estr,"get old file name error");
		}
	}
	else
	{
		strcpy(estr,"not the file name");
	}
	return flag;
}

uint8 GetSlotUart(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag=0;
	uint32 slotid;
	uint32 uart;
	json_t *obj;
	uint8 str[1024];
	uint8 buf[1024];
	json_t *uartnum;
	uint32 status;
	uartnum=json_object();
	if(cmd&&uartnum)
	{
		obj=json_object_get(cmd,"slotid");
		if(JsonGetInteger(obj,&slotid))
		{
			sprintf(str,"#MODULE-UART? %d\r\n",slotid);
			PiPHandler(str,buf,sizeof(buf));
			flag=CmdStrHandler("MODULE-UART",buf);
			if(flag)
			{
				status=sscanf(&buf[flag],"%d,%d\r\n",&slotid,&uart);
				if(status==2)
				{
					json_object_set(uartnum,"slotid",json_integer(slotid));
					json_object_set(uartnum,"uart",json_integer(uart));
					json_object_set(json,"Data",uartnum);
					flag=1;
				}
				else
				{
					flag=0;
					strcpy(estr,"get uart  error1");
				}
			}
			else
			{
				strcpy(estr,"get uart  error");
				flag=0;
			}
		}
	}
	return flag;
}
uint8 GetStaticNetWork(json_t *json,json_t* cmd,char *estr)
{
	uint8 flag;
	json_t network;
	network=json_object();
	uint8 str[1024];
	uint8 buf[1024];
	uint8 ip[16];
	uint8 submask[16];
	uint8 gateway[16];
	json_t *obj;
	uint8 status;
	if(network)
	{
		sprintf(str,"#EXT-NET-STATIC?\r\n");
		PiPHandler(str,buf,sizeof(buf));
		flag=CmdStrHandler("EXT-NET-STATIC",buf);
		if(flag)
		{
			status=sscanf(&buf[flag],"%[^,],%[^,],%15s\r\n",&ip,&submask,&gateway);
			if(status==3)
			{
				json_object_set(network,"ip",json_string(ip));
				json_object_set(network,"submask",json_string(submask));
				json_object_set(network,"gateway",json_string(gateway));
				flag=1;
			}
			else
			{
				flag=0;
				strcpy(estr,"get static network format error");
			}
		}
		else
		{
			strcpy(estr,"get static network error");
			flag=0;
		}
	}
	return flag;
}