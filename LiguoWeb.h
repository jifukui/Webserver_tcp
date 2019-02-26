#ifndef _LIGUO_WEB_H_
#define _LIGUO_WEB_H_
unsigned char LiguoWeb_GET_Method(const char *sstr,json_t * json,char *estr);
unsigned char LiguoWeb_POST_Method(const unsigned char *sstr,json_t *json,char * estr);
#endif _LIGUO_WEB_H_