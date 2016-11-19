/*
* parameter.h
* Author: fuxiaoyin
* Created on: 2016-10-24
* Copyright (c) Ainirobot.com, Inc. All Rights Reserved
*/
#ifndef PARAMETER_H
#define PARAMETER_H

#include "commons.h"
#include "error.h"

ANS_BEG

class Parameter
{
public:
    Parameter() {};
    ~Parameter() {};

    int load_conf(char *config_file);
    int show();
private:
    int load(char *config_file);
public:
    char _input_type[MAX_PATH_LEN];
    char _input_file[MAX_PATH_LEN];
    char _output_file[MAX_PATH_LEN];
    int  _feat_dim;
    int  _class_dim;
    int  _epoch;
private:
    map<string, string> _map_vec; 
};

ANS_END

#endif
