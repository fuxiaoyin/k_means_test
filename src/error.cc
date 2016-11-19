/*
* error.cc
* Author:
* Created on: 2016-10-22
* Copyright (c) Ainirobot.com, Inc. All Rights Reserved
*/

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "commons.h"
#include "error.h"

// -------------------------------------------------
// Local Variables
// -------------------------------------------------

static char  errorfile[2048];
static char  errorline[2048];
static int   errortype;

// -------------------------------------------------
// ppASRLog
// -------------------------------------------------
int ppASRLog(std::string format, ... ) {
    va_list ap;
    string  str,str2;
    char    buf[MAX_TEXT_LEN] = "";
    int     fill1 = 26;
    time_t  cur_time;
    struct  tm* tm_time;

    switch (errortype) {
    case 0:
        str.append("INFO: ");
        break;
    case 1:
        str.append("WARN: ");
        break;
    case 2:
        str.append("ERROR: ");
        break;
    case 3:
        str.append("DEBUG: ");
        break;
    }

    cur_time = time(NULL);
    tm_time  = localtime(&cur_time);
    snprintf(buf, MAX_TEXT_LEN, "%02d-%02d %02d:%02d:%02d: ", 
        1+tm_time->tm_mon, tm_time->tm_mday,
        tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    buf[MAX_TEXT_LEN-1] = '\0';
    str.append(buf);

    str.append("[");
    str.append(errorfile);
    str.append(":");
    str.append(errorline);
    str.append("] ");

    // pretty-print
    int size = str.size();
    for (int i=0;i<fill1-size;i++)
    {
        str.append(" ");
    }
    va_start(ap,format);
    vsprintf(buf,format.c_str(),ap);
    va_end(ap);

    buf[MAX_TEXT_LEN-1] = '\0';
    str2.append(buf);

    str+=str2;

    if(str[str.length()-1] != '\n')
    {
        str += "\n";
    }
    cout<<str;
    cout.flush();
    return 0;
}

// -------------------------------------------------
// ppASRLog
// -------------------------------------------------
ppASRLogFct* ppASRLog(std::string Infile, int line, int type)
{
    int pos = 0;
    char *file = (char *)Infile.c_str();
    int len = Infile.size();
    if (len > 2048)
    {
        fprintf(stderr, "The buffer in error.cc is too samll\n.");
        return NULL;
    }
    for (int i=0; file[i] != '\0'; i++)
    {
        if ( file[i] == '/' || file[i] == '\\')
        {
            pos = i;
        }
    }
    if (pos > 0)
    {
        memcpy( errorfile, &file[pos+1], sizeof(char)*(len-pos) );
    }else
    {
        memcpy( errorfile, file, sizeof(char)*(len+1) );
    }
    errortype = type;
    sprintf(errorline,"%d",line);
    return &ppASRLog;
}

// -------------------------------------------------
// itfoutput
// -------------------------------------------------
int   _bufN = 0;
void *_bufA = NULL;
const char* itfoutput(const char *name)
{
    int itemN = 0;

    itemN = strlen(name);
    if (itemN+1 >= _bufN)
    {
        _bufN += itemN+1;
        _bufA= realloc(_bufA,_bufN);
        if (! _bufA)
        {
            ERROR("itfoutput : cannot copy result to itfBuffer itemN=%d.",itemN);
            return NULL;
        }
    }

    if (itemN>0)
    {
        memcpy(_bufA,name,itemN);
    }
    ((char*) _bufA)[itemN] =0;  //end mark of string 
    return (const char*) _bufA;
}
