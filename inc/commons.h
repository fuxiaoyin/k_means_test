/*
* commons.h
* Author: fuxiaoyin
* Created on: 2016-10-22
* Copyright (c) Ainirobot.com, Inc. All Rights Reserved
*/
#ifndef COMMONS_H
#define COMMONS_H

#include "ans.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
using namespace std;

ANS_BEG

const int MAX_PATH_LEN  = 1024;
const int MAX_TEXT_LEN  = 1024;

const int RET_ERROR     = 0;
const int RET_OK        = 1;

#ifndef INT_MAX_VALUE   
#define INT_MAX_VALUE 2147483645
#endif

#ifndef FLT_MAX_VALUE
#define FLT_MAX_VALUE 1e+37
#endif

ANS_END

#endif
