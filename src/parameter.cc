/*
* parameter.cc
* Author: fuxiaoyin
* Created on: 2016-10-24
* Copyright (c) Ainirobot.com, Inc. All Rights Reserved
*/
#include "parameter.h"

int Parameter::load_conf(char *config_file)
{
    if (config_file == NULL || config_file[0] == '\0') {
        ERROR("config file not set!\n");
        return RET_ERROR;
    }
    int ret = load(config_file);
    if (ret == RET_ERROR) {
        ERROR("load config file %s failed!\n", config_file);
        return RET_ERROR;
    }

    map<string, string>::iterator iter;

    iter = _map_vec.find("INPUT_TYPE");
    if (iter != _map_vec.end()) {
        strcpy(_input_type, iter->second.c_str());
        _input_type[MAX_PATH_LEN-1] = '\0';
    }
    else {
        ERROR("Cannot find input type!\n");
        return RET_ERROR; 
    }
    
    iter = _map_vec.find("INPUT_FILE");
    if (iter != _map_vec.end()) {
        strcpy(_input_file, iter->second.c_str());
        _input_file[MAX_PATH_LEN-1] = '\0';
    }
    else {
        ERROR("Cannot find input file!\n");
        return RET_ERROR; 
    }
    
    iter = _map_vec.find("OUTPUT_FILE");
    if (iter != _map_vec.end()) {
        strcpy(_output_file, iter->second.c_str());
        _output_file[MAX_PATH_LEN-1] = '\0';
    }
    else {
        ERROR("Cannot find output file!\n");
        return RET_ERROR; 
    }
    
    iter = _map_vec.find("FEAT_DIM");
    if (iter != _map_vec.end()) {
        _feat_dim = atoi(iter->second.c_str());
    }
    else {
        ERROR("Cannot find feat dim!\n");
        return RET_ERROR; 
    }
    
    iter = _map_vec.find("CLASS_DIM");
    if (iter != _map_vec.end()) {
        _class_dim = atoi(iter->second.c_str());
    }
    else {
        ERROR("Cannot find class dim!\n");
        return RET_ERROR; 
    }
    
    iter = _map_vec.find("EPOCH");
    if (iter != _map_vec.end()) {
        _epoch = atoi(iter->second.c_str());
    }
    else {
        ERROR("Cannot find epoch!\n");
        return RET_ERROR; 
    }
    return RET_OK;
}

int Parameter::show()
{
    INFO("----------------config start------------------");
    INFO("INPUT_TYPE    : %s", _input_type);
    INFO("INPUT_FILE    : %s", _input_file);
    INFO("OUTPUT_FILE   : %s", _output_file);
    INFO("FEAT_DIM      : %s", _feat_dim);
    INFO("CLASS_DIM     : %s", _class_dim);
    INFO("EPOCH         : %s", _epoch);
    INFO("----------------config end--------------------");
    return RET_OK;
}

int Parameter::load(char *config_file)
{
    FILE *fp = fopen(config_file, "rt");
    if (fp == NULL) {
        ERROR("Cannot open file %s to read!\n", config_file);
        return RET_ERROR;
    }
    char content[1024];
    char res[1024];
    while(!feof(fp)) {
        memset(content, '\0', 1024);
        memset(res,     '\0', 1024);
        fgets(content, 1024, fp);
        char *temp = strtok(content, "\r\n");
        if (temp == NULL) {
            continue;
        }
        // clear space
        int len = strlen(temp);
        int idx = 0;
        for (int ii = 0; ii < len; ii++) {
            if (temp[ii] == ' ') {
                continue;
            }
            res[idx] = temp[ii];
            idx++;
        }
        string str(res);
        int pos = str.find(":");
        if (pos == -1) {
            ERROR("Wrong config format: %s\n", temp);
            return RET_ERROR;
        }
        _map_vec.insert(pair<string, string>(str.substr(0, pos), str.substr(pos+1, -1-pos-1)));
    }
    fclose(fp);
    return RET_OK;
}
