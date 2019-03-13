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
STATIC uint8 Port2Phy(uint8 port);
STATIC uint8 CmdStrHandler(uint8 *str,uint8 *buf);

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
	struct timeval start,end;
	//pid_t pid;
	unsigned long time;
	bzero(rx,len);
	//slig_pip_read_bytes(sockfd,rx,len);
	printf("The send buf is %s\n",tx);
	length=lig_pip_write_bytes(sockfd,tx,strlen(tx)+1);
	if(length>0)
	{
		length=0;
		gettimeofday(&start,NULL);
		do{
        	length=lig_pip_read_bytes(sockfd,rx,len);
		}while(length==0);
		gettimeofday(&end,NULL);
		time=1000000*(end.tv_sec-start.tv_sec)+end.tv_usec-start.tv_usec;
		printf("The time is %d\n",time);
		//pid =getpid();
		//printf("The child pid is %d \n",pid);
	}
	printf("The recieve buf is %s\n",rx);
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
		json_object_set_new(json,"name",json_string(&buf[flag]));
		json_object_set_new(json,"PortNumber",json_integer(LigPortNum));
		strcpy(str,"#VERSION?\r\n");
		PiPHandler(str,buf,sizeof(buf));
		buf[strlen(buf)-2]=NULL;
		flag=CmdStrHandler("VERSION",buf);
		if(flag)
		{
			json_object_set_new(json,"version",json_string(&buf[flag]));
			strcpy(str,"#SN?\r\n");
			PiPHandler(str,buf,sizeof(buf));
			buf[strlen(buf)-2]=NULL;
			flag=CmdStrHandler("SN",buf);
			if(flag)
			{
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
		
	}
	else
	{
		strcpy(estr,"Not Get Model Name");
	}
	return flag;
	
/*
	uint8 flag=0;
	uint8 i;
    uint8 buf[300];
    uint8 str[]="#MODEL?\r\n#VERSION?\r\n#SN?\r\n";	
	uint8 data[300];
	uint8 *status=NULL;
	PiPHandler(str,buf,sizeof(buf));
	memmove(buf,&buf[START],strlen(&buf[flag]));
	buf[strlen(buf)-2]=NULL;
	flag=CmdStrHandler("MODEL",buf);
	if(flag)
	{
		json_object_set_new(json,"PortNumber",json_integer(LigPortNum));
		status=strstr(buf,"~01@");
		if(status)
		{
			buf[status-buf-2]=NULL;
			json_object_set_new(json,"name",json_string(&buf[flag]));
			memmove(buf,status+4,strlen(status+4));
			flag=CmdStrHandler("VERSION",buf);
			if(flag)
			{
				status=strstr(buf,"~01@");
				if(status)
				{
					buf[status-buf-2]=NULL;
					json_object_set_new(json,"version",json_string(&buf[flag]));
					memmove(buf,status+4,strlen(status+4));
					flag=CmdStrHandler("SN",buf);
					if(flag)
					{
						json_object_set_new(json,"sn",json_string(&buf[flag]));
					}
					else
					{
						strcpy(estr,"Get SN Error");
					}
					
				}
				else
				{
					strcpy(estr,"Get SN Error");
				}
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
	}
	else
	{
		strcpy(estr,"Not Get Model Name");
	}
	return flag;*/
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
					//printf("The port is %d\n",data[0]);
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
			status=sscanf(&buf[START],"NAME ERR %d\r\n",&data);
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
	char buf[256]="#FACTORT\r\n";
	char data[256];
	PiPHandler(buf,data,sizeof(data));
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
					port=port/(LigPortNum/8);
				}
				else
				{
					port=Port2Phy(port);
				}
				
				sprintf(str,"#GEDID %d,%d\r\n",attr,port);
				printf("The str is %s\n",str);
				PiPHandler(str,buf,sizeof(buf));
				printf("The buf is :%s \n",buf);
				status=sscanf(&buf[START],"GEDID %d,%d,%d\r\n",&attr,&port,&len);
				printf("The status is %d\n",status);
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
						printf("The flag is %d \n",flag);
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
	uint32 bitmap=0;
	uint32 status;
	json_t *obj;
	json_t * arr;
	if(cmd)
	{
		obj=json_object_get(cmd,"org");
		if(JsonGetInteger(obj,&in))
		{
			in=Port2Phy(in);
			if(in)
			{
				obj=json_object_get(cmd,"type");
				if(JsonGetInteger(obj,&type))
				{
					if(type<=2)
					{
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
										bitmap|=(1<<out);
									}
								}
							}
							//printf("the in is %d\n",in);
							//printf("The type is %d\n",type);
							//printf("The bit map is %d\n",bitmap);
							if(bitmap)
							{
								sprintf(str,"#CPEDID %d,%d,0,0x%x\r\n",type,in,bitmap);
								PiPHandler(str,buf,sizeof(buf));
								//printf("The buf is :%s \n",buf);
								status=sscanf(&buf[START],"CPEDID ERR,%d\r\n",&type);
								//printf("The status is %d\n",status);
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
	uint32 bitmap=0;
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
						bitmap|=(1<<in);
					}
				}
			}
			if(bitmap)
			{
				obj=json_object_get(cmd,"EDID");
				if(JsonGetString(obj,data))
				{
					len=strlen(data)/2;
					printf("The len is %d\n",len);
					StringtoUint8(&edid[4],data);
					sprintf(str,"#LDEDID 0,0x%x,%d,1\r\n",bitmap,len);
					PiPHandler(str,buf,sizeof(buf));
					printf("The Buffer is %s\n",buf);
					if(strstr(buf,"READY"))
					{
						printf("good for first\n");
						bzero(str,sizeof(str));
						edid[0]=0;
						edid[1]=1;
						edid[2]=(len+2)/256;
						edid[3]=(len+2)%256;
						edid[len+4]=0xAA;
						edid[len+5]=0x55;
						for(i=0;i<len+6;i++)
						{
							printf("The data %d is %02x\n",i,edid[i]);
						}
						len=lig_pip_write_bytes(sockfd,edid,len+6);
						if(len)
						{
							printf("The len is %d\n",len);
							len=0;
							do{
								len=lig_pip_read_bytes(sockfd,buf,sizeof(buf));
								if(len>0)
								{
									printf("The buf is %s\n",buf);
									if(strstr(buf,"ERR"))
									{
										strcpy(estr,"second command error");
										break;
									}
									else if(sscanf(&buf[START],"LDEDID %d,0x%d,%d,%d OK\r\n",&in,&bitmap,&len,&status))
									{
										flag=1;
										break;
									}
								}
							}while(1);
							printf("end of ldedid\n");
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
			printf("The buf is %s\n",buf);
			if(strstr(buf,"ERR"))
			{
				strcpy(estr,"set net work error");
			}
		}
		obj=json_object_get(cmd,"tcp");
		if(JsonGetInteger(obj,&tcp))
		{
			printf("The tcp is %d\n",tcp);
			sprintf(str,"#ETH-PORT TCP,%d\r\n",tcp);
			PiPHandler(str,buf,sizeof(buf));
			printf("The buf is %s\n",buf);
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
			printf("The buf is %s\n",buf);
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
	json_t *arr;
	json_t *data;
	if(cmd)
	{
		if(json_typeof(cmd)==JSON_ARRAY)
		{
			for(i=0;i<json_array;i++)
			{
				data=json_array_get(arr,i);
				if(json_typeof(data)==JSON_OBJECT)
				{
					obj=json_object_get(cmd,"sid");
					if(JsonGetInteger(obj,&sid))
					{
						obj=json_object_get(cmd,"index");
						if(JsonGetInteger(obj,&index))
						{
							index=Port2Phy(index);
							if(index)
							{
								obj=json_object_get(cmd,"dir");
								if(JsonGetInteger(obj,&dir))
								{
									obj=json_object_get(cmd,"value");
									if(JsonGetInteger(obj,&value))
									{
										sprintf(str,"#MODULE-FUNC %d,%d,%d,%d\r\n",sid,dir,index,value);
										printf("The str is %s\n",str);
										PiPHandler(str,buf,sizeof(buf));
										printf("The buf is %s\n",buf);
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
	return flag;
}