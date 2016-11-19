/*
* k_means.h
* Author: fuxiaoyin
* Created on: 2016-10-22
* Copyright (c) Ainirobot.com, Inc. All Rights Reserved
*/

#ifndef K_MEANS_H
#define K_MEANS_H

#include <stdio.h>
#include <stdlib.h>
#include "commons.h"
#include "parameter.h"

ANS_BEG

class K_means
{
public:
    K_means();
    ~K_means();
    
    int init(Parameter &param);
    int do_cluster();
    int write();
private:
    int load_data_list(char *file_name, char *type);
    int load_txt_data(char *file_name);
    int load_bin_data(char *file_name);
    //
    float assign_centroid();
    int   calculate_new_centroid();
    float calculate_distance(
            vector<float> &data_vec, vector<float> &cent_vec);
public:
    int _feat_dim;
    int _sample_num;
    int _class_dim;
    int _epoch;
    vector< vector<float> > _data_mat;
    vector< vector<float> > _cent_mat;
    vector< vector<float> > _old_cent_mat;
    //
    vector<int> _assigned_vec;     // 记录样本聚到哪个类中
    vector<int> _centroid_cnt_vec; // 记录每个类的样本数目

    vector<string> _name_vec;
    vector<int>    _sample_vec;
};

ANS_END

#endif
