/*
* ans.h
* Author:
* Created on: 2016-10-22
* Copyright (c) Ainirobot.com, Inc. All Rights Reserved
*/

#ifndef ANS_H
#define ANS_H

#ifdef ANS

#define ANS_BEG namespace ppASR {
#define ANS_END }
#define ANS_USE using namespace ppASR;
#define ANS_PRE ppASR::

#else

#define ANS_BEG 
#define ANS_END 
#define ANS_USE 
#define ANS_PRE 
namespace ppASR {}

#endif

#endif
