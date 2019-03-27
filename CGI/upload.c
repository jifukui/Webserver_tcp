#include <stdio.h>
#include <string.h>
#include <jansson.h>
char uploadfilename[64];
int contentlen;
int codelength;
void getstartcode();
int getfilename();
void getfiletype();
int getfilecontent();
void getendcode();
int getdata(char *data);
int main()
{
	int flag=0;
	char *val=NULL;
	json_t *obj;
	obj=json_object();
	val=getenv("CONTENT_LENGTH");
	printf("Content-type: application/json; charset=utf-8\r\n");
	printf("Cache-Control: no-cache\r\n");
	printf("Accept-Rangs:bytes\r\n");
	printf("Connection:close\r\n");
	printf("\r\n");
	//printf("Content-type:text/plain\n\n");
	//printf("The val is %s\n\n",val);
	if(val!=NULL&&obj)
	{
		contentlen=atoi(val);
		json_object_set_new(obj,"cmd",json_string("UploadFile"));
		json_object_set_new(obj,"status",json_string("ERROR"));
		json_object_set_new(obj,"error",json_null());
		//printf("The content length is %d\n\n",contentlen);
		getstartcode();
		if(!getfilename())
		{
			//printf("Get filename success name is %s\n\n",uploadfilename);	
			getfiletype();
			int code=getfilecontent();
	                if(!code)
        	        {
				getendcode();
				flag=1;
                	}
                	else
                	{
				json_object_set(obj,"error",json_string("write file error"));
				json_object_set_new(obj,"code",json_integer(code));
                	}
                	
		}
		else
		{
			//printf("Get filename error\n\n");
			json_object_set(obj,"error",json_string("get file name error"));
		}
	}
	else
	{
		printf("{\"cmd\":\"UploadFile\",\"status\":\"ERROR\",\"error\":\"init error\"}");		
	}

	if(flag)
	{
		json_object_set(obj,"status",json_string("SUCCESS"));
	}
	char *str=NULL;
	str=json_dumps(obj,JSON_PRESERVE_ORDER);
	printf(str);
	free(str);
	if(str)
	{
		str=NULL;
	}
	printf("\r\n");
	return 0;
}
void getstartcode()
{
	//int len=0;
	char val[100];	
	codelength=getdata(val);
	//printf("The start code is %s\n\n",val);
	//printf("The length is %d\n\n",codelength);
	codelength+=2;
}
int getfilename()
{
	int len=0;
	char *name=NULL;
	char filename[1024];
	len=getdata(filename);
	filename[len-3]=0;
	name=strstr(filename,"filename");
	//printf("The filename is %s\n\n",filename);
	if(name)
	{
		//printf("The name is %s\n\n",name+10);
		strcpy(uploadfilename,name+10);
		return 0;
	}
	else
	{
		return 1;
	}
	//printf("The length is %d\n\n",len);
}
void getfiletype()
{
	//int len;
	char filetype[1024];
	getdata(filetype);
	//printf("The filetype is %s\n\n",filetype);
	//printf("The length is %d\n\n",len);
	getchar();
	getchar();
	contentlen-=2;
}
int getfilecontent()
{
	int i;
  	ssize_t num;
	int length=0;
	char data[contentlen];
	char path[256];
	length=contentlen-codelength;
	for(i=0;i<length;i++)
	{
		/*if(length-i>1024)
		{
			num=read(stdin,&data[i],1024);
			printf("num %d\n\n",num);
		}
		else
		{
			num=read(stdin,&data[i],length-i);
			printf("num %d length %d\n\n",num,length-i);
		}
		if(num<0)
		{
			printf("Have error %d\n\n",i);
			break;
		}*/
		data[i]=getchar();	
	}
	length-=2;
	data[length]=0;
	FILE *fp=NULL;
	sprintf(path,"/tmp/www/%s",uploadfilename);
	fp=fopen(path,"wb+");
	if(fp)
	{
	//	int filelength=fwrite(data,1,length,fp);
		if(length==fwrite(data,1,length,fp))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 2;
	}
	//printf("The data is %s\n\n",data);
	//printf("The data length is %d\n\n",length);
}
void getendcode()
{
	//int len=0;
	char data[100];
	getdata(data);
	//printf("The endcode is %s\n\n",data);
	//printf("The length is %d\n\n",len);
}
int getdata(char *data)
{
	int flag=0;
	int n=0;
	char ch;
	while(contentlen>0)
        {
                ch=getchar();
                data[n]=ch;
                n++;
		contentlen--;
                if(flag)
                {
                        flag=0;
                        if(ch==10)
                        {
                                data[n]=0;
                        	return n;
                        }
                }
                else
                {
                        if(ch==13)
                        {
                                flag=1;		
                        }
                }
        }
	return 0;

}
