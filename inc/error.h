/*
* error.h
* Author: 
* Created on: 2016-10-22
* Copyright (c) Ainirobot.com, Inc. All Rights Reserved
*/

#ifndef ERROR_H
#define ERROR_H
//
#include "commons.h"
//
ANS_BEG

typedef int      ppASRLogFct   (std::string, ... );
/**
 * @brief write log
 */
int              ppASRLog(std::string, ... );
ppASRLogFct*     ppASRLog(std::string file, int line, int type);

//
#define INFO     ppASRLog(__FILE__,__LINE__,0)
#define WARN     ppASRLog(__FILE__,__LINE__,1)
#define ERROR    ppASRLog(__FILE__,__LINE__,2)
#define DEBUG    ppASRLog(__FILE__,__LINE__,3)

const char* itfoutput(const char* string);

ANS_END

#endif
