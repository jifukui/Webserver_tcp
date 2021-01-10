#ifndef _LIGUO_WEB_H_
#define _LIGUO_WEB_H_
unsigned char LiguoWeb_GET_Method(const char *sstr,json_t * json,char *estr);
unsigned char LiguoWeb_POST_Method(const unsigned char *sstr,json_t *json,char * estr);
typedef struct 
{
    char username[15];
    char password[PASSWORDLEN];
}authenticate;              
typedef struct{
    int security;
    authenticate Auth[AUTH_NUM];
}Auth_liguo;
void writesecurityfile();
uint8 CheckPassword(uint8 *password);
#endif _LIGUO_WEB_H_