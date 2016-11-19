/*
* k_means.cc
* Author: fuxiaoyin
* Created on: 2016-10-22
* Copyright (c) Ainirobot.com, Inc. All Rights Reserved
*/

#include "k_means.h"

K_means::K_means()
{
    _feat_dim   = 0;
    _class_dim  = 0;
    _sample_num = 0;
}

K_means::~K_means()
{
}

int K_means::init(Parameter &param)
{
    _feat_dim  = param._feat_dim;
    _class_dim = param._class_dim;
    _epoch     = param._epoch;
    //
    _sample_num = load_data_list(param._input_list, param._input_type);
    // choose initial centroids
    _cent_mat.resize(_class_dim);
    for (int ii = 0; ii < _class_dim; ii++) {
        _cent_mat[ii].resize(_feat_dim);
        for (int jj = 0; jj < _feat_dim; jj++)
        _cent_mat[ii][jj] = _data_mat[ii][jj];
    }
    assign_centroid();
}

int K_means::load_data_list(char *file_name, char *type)
{
    FILE *fp = fopen(file_name, "rt");
    if (fp == NULL) {
        ERROR("Cannot open file %s to read!\n", file_name);
        return RET_ERROR;
    }
    int sample_num = 0;
    char content[1024];
    while (!feof(fp)) {
        memset(content, '\0', 1024);
        fgets(content, 1024, fp);
        char *temp = strtok(content, " \r\n");
        if (temp == NULL) {
            continue;
        }
        if (strcmp(type, "txt") == 0) {
            sample_num += load_txt_data(temp);
        }
        else {
            sample_num += load_bin_data(temp);
        }
        //
        _name_vec.push_back(temp);
        _sample_vec.push_back(sample_num);
    }
    fclose(fp);
    return sample_num;
}

int K_means::load_txt_data(char *file_name)
{
    FILE *fp = fopen(file_name, "rt");
    if (fp == NULL) {
        ERROR("Cannot open file %s to read!\n", file_name);
        return RET_ERROR;
    }
    vector<float> data_vec;
    data_vec.resize(_feat_dim);
    char content[10240];
    while (!feof(fp)) {
        memset(content, '\0', 10240);
        fgets(content, 10240, fp);
        char *temp = strtok(content, " \r\n");
        if (temp == NULL) {
            continue;
        }
        int idx = 0;
        while (temp) {
            data_vec[idx++] = atof(temp);
            temp = strtok(NULL, " \r\n");
            if (idx == _feat_dim && temp != NULL) {
                ERROR("too many features!\n");
                return RET_OK;
            }
        }
        if (idx != _feat_dim) {
            ERROR("idx = %d vs feat_dim = %d\n", idx, _feat_dim);
            return RET_OK;
        }
        _data_mat.push_back(data_vec);
    }
    fclose(fp);
    //
    int sample_num = _data_mat.size();
    //
    INFO("Load samples successfully! sample_num = %d\n", sample_num);
    return sample_num;
}

int K_means::load_bin_data(char *file_name)
{
    FILE *fp = fopen(file_name, "rb");
    if (fp == NULL) {
        ERROR("Cannot open file %s to read!\n", file_name);
        return RET_ERROR;
    }
    int feat_dim = 0;
    int sample_num = 0;
    fread(&feat_dim, sizeof(int), 1, fp);
    fread(&sample_num, sizeof(int), 1, fp);

    if (feat_dim != _feat_dim) {
        ERROR("wrong feat dim %d vs %d\n", feat_dim, _feat_dim);
    }
    
    _data_mat.resize(sample_num);
    for (int ii = 0; ii < sample_num; ii++) {
        _data_mat[ii].resize(_feat_dim);
        fread(&_data_mat[ii][0], sizeof(float), _feat_dim, fp);
    }
    fclose(fp);
    //
    _sample_num = _data_mat.size();
    //
    INFO("Load samples successfully! sample_num = %d\n", _sample_num);
    //
    return RET_OK;
}

int K_means::do_cluster()
{
    for (int ii = 0; ii < _epoch; ii++) {
        calculate_new_centroid();
        assign_centroid();
    }
    return RET_OK;
}

float K_means::calculate_distance(
        vector<float> &data_vec, vector<float> &cent_vec)
{
    float distance = 0.0f;
    for (int ii = 0; ii < _feat_dim; ii++) {
        distance += (data_vec[ii] - cent_vec[ii]) * (data_vec[ii] - cent_vec[ii]);
    }
    return distance;
}

int K_means::assign_centroid()
{
    float min_distance = FLT_MAX_VALUE;
    int   choose_id    = INT_MAX_VALUE;
    for (int ii = 0; ii < _sample_num; ii++) {
        for (int jj = 0; jj < _class_dim; jj++) {
            float distance_to_centroid = calculate_distance(
                    _data_mat[ii], _cent_mat[jj]);
            if (distance_to_centroid < min_distance) {
                min_distance = distance_to_centroid;
                choose_id    = jj;
            }
        }
        _assigned_vec[ii] = choose_id;
        _centroid_cnt_vec[choose_id]++;
    }
    return RET_OK;
}

int K_means::calculate_new_centroid()
{
    _old_cent_mat = _cent_mat;

    for (int ii = 0; ii < _class_dim; ii++) {
        int count = 0;
        vector<float> data_sum_vec;
        data_sum_vec.resize(_feat_dim, 0.0f);
        for (int jj = 0; jj < _sample_num; jj++) {
            if (_assigned_vec[jj] == ii) {
                for (int kk = 0; kk < _feat_dim; kk++) {
                    data_sum_vec[kk] += _data_mat[jj][kk];
                }
                count++;
            }
        }
        for (int jj = 0; jj < _feat_dim; jj++) {
            _cent_mat[ii][jj] = data_sum_vec[jj] / (float)count;
        }
    }
}

int K_means::write()
{
    char out_file_name[1024];
    for (int ii = 0; ii < _sample_vec.size(); ii++) {
        sprintf(out_file_name, "%s.class", _name_vec[ii].c_str());
        FILE *fp = fopen(out_file_name, "wt");
        if (fp == NULL) {
            ERROR("Cannot open file %s to write!\n", out_file_name);
            return RET_ERROR;
        }
        int st = (ii == 0) ? 0 : _sample_vec[ii - 1];
        int ed = _sample_vec[ii];
        for (int jj = st; jj < ed; jj++) {
            fprintf(fp, "%d\n", _assigned_vec[jj]);
        }
        fclose(fp);
    }
    return RET_OK;
}
