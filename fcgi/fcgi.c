#include "fcgi_stdio.h"
#include <stdlib.h>
#include <regex.h>
#include <syslog.h>
#include <string.h>
#include <locale.h>
#include <curl/curl.h>


int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "C.UTF-8");
  regex_t rgex;
  char* pattern = "^([0-9]{0,3}\\.?[0-9]+\t){2}([[:print:]]*\t){1,4}[[:print:]]*$";
  int ret=regcomp(&rgex,pattern,REG_EXTENDED);
  if(ret){
    syslog(LOG_ERR,"Broken regex!\n");
    exit(1);
  }
  
  CURL *curl = curl_easy_init();
  if(!curl)
  {
    syslog(LOG_ERR,"Can not initialize curl");
    return 1;
  }
      
  FILE* file; 
  file=fopen("/var/www/data/data","a");
  
  if(!file)
  {
    syslog(LOG_ERR,"Can not open file /var/www/data/data");
    return 1;
  }
  while (FCGI_Accept() >= 0) {
    char* dataset=curl_easy_unescape(curl,getenv("QUERY_STRING"),0,NULL);
    if(!dataset)
      syslog(LOG_NOTICE,"urlunescape failed.");
    
    int status = regexec(&rgex, dataset, (size_t) 0, NULL, 0);
    if(status)
    {
      printf("Content-type: text/plain\r\n\r\n Wrong Format");
    }
    else
    {
      int ret=fputs(dataset,file);
      fflush(file);
      if(ret!=EOF)
        printf("Content-type: text/plain\r\n\r\n successfully added %d Bytes:\n%s",ret,dataset);
    }
    curl_free(dataset);
  }
  return(0);
}

