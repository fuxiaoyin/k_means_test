#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "commons.h"
#include "parameter.h"
#include "cuMatrix.h"

class cukmeans
{
public:
    cukmeans();
    ~cukmeans();
    
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


    cuMatrix<float>* _data_mat;
    cuMatrix<float>* _cent_mat;

    cuMatrix<int>* _assigned_vec; // 记录样本聚到哪个类中

    cuMatrix<float>* _min_distance; // 记录每个点到最近的中心的距离
    cuMatrix<float>* _distance;// 记录每个点到每个中心的距离
    cuMatrix<float>* _ave_distance; // 平均距离

    vector<string> _name_vec;//多个文件读入相关的变量
    vector<int>    _sample_vec;
};
