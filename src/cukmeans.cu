#include "cukmeans.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
cukmeans::cukmeans()
{
    _feat_dim   = 0;
    _class_dim  = 0;
    _sample_num = 0;
}

cukmeans::~cukmeans()
{
}

int cukmeans::init(Parameter &param)
{
    _feat_dim  = param._feat_dim;
    _class_dim = param._class_dim;
    _epoch     = param._epoch;
    //
    _sample_num = load_data_list(param._input_list, param._input_type);
    _data_mat->toGpu();
    _cent_mat = new cuMatrix<float>(_class_dim, _feat_dim, 1);
    // choose initial centroids
    memcpy(_cent_mat->getHost(), _data_mat->getHost(), sizeof(float) * _class_dim * _feat_dim);
    _cent_mat->toGpu();

    _assigned_vec = new cuMatrix<int>(_sample_num, 1, 1);
    _min_distance = new cuMatrix<float>(_sample_num, 1, 1);
    _distance = new cuMatrix<float>(_sample_num, _class_dim, 1);
    _ave_distance = new cuMatrix<float>(1, 1, 1);

    float ave_dist = assign_centroid();
    INFO("init average distance = %f\n", ave_dist);
}

int cukmeans::load_data_list(char *file_name, char *type)
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

int cukmeans::load_txt_data(char *file_name)
{
    FILE *fp = fopen(file_name, "rt");
    if (fp == NULL) {
        ERROR("Cannot open file %s to read!\n", file_name);
        return RET_ERROR;
    }
    vector<float> data_vec;
    vector<vector<float> > data_mat;

    data_vec.resize(_feat_dim);
    int sample_num = 0;
    char content[10240];
    while (!feof(fp)) {
        memset(content, '\0', 10240);
        fgets(content, 10240, fp);
        char *temp = strtok(content, " \t\r\n");
        if (temp == NULL) {
            continue;
        }
        int idx = 0;
        while (temp) {
            data_vec[idx++] = atof(temp);
            temp = strtok(NULL, " \t\r\n");
            if (idx == _feat_dim && temp != NULL) {
                ERROR("too many features!\n");
                return RET_OK;
            }
        }
        if (idx != _feat_dim) {
            ERROR("idx = %d vs feat_dim = %d\n", idx, _feat_dim);
            return RET_OK;
        }
        data_mat.push_back(data_vec);
        sample_num++;
    }
    fclose(fp);

    //copy to _data_mat
    _data_mat = new cuMatrix<float>(data_mat.size(), _feat_dim, 1);
    for(size_t i = 0; i < data_mat.size(); i++){
        for(size_t j = 0; j < data_mat[i].size(); j++){
            _data_mat->getHost()[i * _feat_dim + j] = data_mat[i][j];
        }
    }

    INFO("Load samples successfully! sample_num = %d\n", sample_num);
    return sample_num;
}

int cukmeans::load_bin_data(char *file_name)
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
    _data_mat = new cuMatrix<float>(sample_num, feat_dim, 1);
    for (int ii = 0; ii < sample_num; ii++) {
        fread(_data_mat->getHost() + ii * feat_dim, sizeof(float), _feat_dim, fp);
    }
    fclose(fp);

    _sample_num = _data_mat->rows;
    //
    INFO("Load samples successfully! sample_num = %d\n", _sample_num);
    //
    return RET_OK;
}

int cukmeans::do_cluster()
{
    for (int ii = 0; ii < _epoch; ii++) {
        calculate_new_centroid();
        float ave_dist = assign_centroid();
        INFO("epoch%d average distance = %f\n", ii, ave_dist);
    }
    return RET_OK;
}

float cukmeans::calculate_distance(
        vector<float> &data_vec, vector<float> &cent_vec)
{
    float distance = 0.0f;
    for (int ii = 0; ii < _feat_dim; ii++) {
        distance += (data_vec[ii] - cent_vec[ii]) * (data_vec[ii] - cent_vec[ii]);
    }
    return distance;
}

/*
* assigned_vec 每个点指向每个中心
*/
__global__ void g_distance(float* data_mat, float* cent_mat, float* distance,
        int sample_num, int class_dim, int feat_dim){
    //  extern __shared__ float _sum[];
    int sample_id = blockIdx.x;
    int class_id = threadIdx.x;
    float* cur_cent_mat = cent_mat + class_id * feat_dim;
    float* cur_data_mat = data_mat + sample_id * feat_dim;
    float dis = 0;
    for(int i = 0; i < feat_dim; i++){
        float cent = cur_cent_mat[i];
        float data = cur_data_mat[i];
        dis += (cent - data) * (cent - data);
    }
    distance[sample_id * class_dim + class_id] = dis;
}

/*
*根据距离计算出每个点离那个中心点最近，并且将距离记录在min_distance中
*注意　class_dim 不要大于1024
*/
__global__ void g_assign_centroid(
        float* distance, 
        float* min_distance,
        int* assign_centroid,
        int sample_num,
        int class_dim){
    extern __shared__ float block_min[];
    int* block_assign_centroid = (int*)block_min + class_dim;

    int class_id = threadIdx.x;
    int sample_id = blockIdx.x;
    block_min[class_id] = distance[sample_id * class_dim + class_id];
    block_assign_centroid[class_id] = class_id;
    //reduce 
    int len = blockDim.x;
    int tid = threadIdx.x;
    while(len != 1)
    {
        __syncthreads();
        int skip = (len + 1) >> 1;
        if(tid < (len >> 1))
        {
            float value1 = block_min[tid];
            float value2 = block_min[tid + skip];
            //TODO 是大于等于还是大于？
            if(value1 > value2){
                block_min[tid] = value2;
                block_assign_centroid[tid] = block_assign_centroid[tid + skip];
            }
        }
        else{
            return;
        }
        len = (len + 1) >> 1;
//        if(blockIdx.x == 4){
//            printf("tix %d assign %d dis %f len %d\n", class_id, block_assign_centroid[class_id], 
//                    block_min[class_id], len);
//        }
    }
    if(tid == 0)
    {
        min_distance[sample_id] = block_min[0];
        assign_centroid[sample_id] = block_assign_centroid[0];
    }
}

/*
   reduce sum 操作
 */
__global__ void g_avr_distance(
        float* min_distance,
        float* ave_distance,
        int sample_num){
    extern __shared__ float sum[];
    sum[threadIdx.x] = 0;
    //先累加到sum中
    for(int i = 0; i < sample_num; i += blockDim.x){
        int idx = i + threadIdx.x;
        if(idx < sample_num){
            sum[threadIdx.x] += min_distance[idx];
        }
    }
    //reduce
    __syncthreads();
    int len = blockDim.x;
    while(len != 1)
    {
        __syncthreads();
        int skip = (len + 1) >> 1;
        if(threadIdx.x < (len >> 1))
        {
            sum[threadIdx.x] += sum[threadIdx.x + skip];
        }
        else{
            return;
        }
        len = (len + 1) >> 1;
    }
    if(threadIdx.x == 0)
    {
        ave_distance[0] = sum[0];
    }
}

/*
 * 将每个节点指向距离他最近的中心节点
 * 复杂度O(n^3)
 * 1)计算优化，先计算出每个点到中心的计算
 */
float cukmeans::assign_centroid()
{
    dim3 block_dim(_sample_num);
    dim3 thread_dim(_class_dim);
    if(_class_dim >= 1024){
        printf("error class_dim > 1024");
        exit(0);
    }
    /*
     * TODO 通过shared_memery进行访存优化
     */
    g_distance<<<block_dim, thread_dim, _class_dim * sizeof(float),0>>>(
            _data_mat->getDev(),
            _cent_mat->getDev(),
            _distance->getDev(),
            _sample_num, 
            _class_dim, 
            _feat_dim);
    checkCudaErrors(cudaStreamSynchronize(0));
    getLastCudaError("g_distance");
    //    _distance->toCpu();
    //    _distance->print();
    //    exit(0);

    block_dim = dim3(_sample_num);
    thread_dim = dim3(_class_dim);

    g_assign_centroid<<<block_dim, thread_dim, (2 * sizeof(float)) * _class_dim,0>>>(
            _distance->getDev(),
            _min_distance->getDev(),
            _assigned_vec->getDev(),
            _sample_num,
            _class_dim);
    checkCudaErrors(cudaStreamSynchronize(0));
    getLastCudaError("g_assign_centroid");
    //_assigned_vec->toCpu();
    //_assigned_vec->print();
    //_min_distance->toCpu();
   // _min_distance->print();
    //exit(0);

    g_avr_distance<<<dim3(1), dim3(256), sizeof(float) * 256,0>>>(
            _min_distance->getDev(),
            _ave_distance->getDev(),
            _sample_num);
    checkCudaErrors(cudaStreamSynchronize(0));
    getLastCudaError("g_avr_distance");
    _ave_distance->toCpu();

    return _ave_distance->getHost()[0] / (float)_sample_num;
}

/*
 * 根据每个节点的指向，算出距离的平均值作为新的中心
 */
__global__ void g_calculate_new_centroid(float* data_mat, float* cent_mat, int* assign_centroid,
        int sample_num, int class_dim, int feat_dim){
    int class_id = blockIdx.x;
    int feat_id = threadIdx.x;
    int count = 0;
    extern __shared__ float sum[];
    sum[feat_id] = 0;
    for(int i = 0; i < sample_num; i++){
        if(class_id == assign_centroid[i]){
            sum[feat_id] += data_mat[i * feat_dim + feat_id];
            count += 1;
        }
    }
    if(count == 0 && feat_id == 0){
        printf("error count == 0\n");
    }
    cent_mat[class_id * feat_dim + feat_id] = sum[feat_id] / count;
}

int cukmeans::calculate_new_centroid()
{
    dim3 block_dim = dim3(_class_dim);
    dim3 thread_dim = dim3(_feat_dim);
    //TODO
    if(_feat_dim >= 1024){
        printf("feat_dim > 1024");
        exit(0);
    }
    //_cent_mat->toCpu();
    //_cent_mat->print();
    //printf("\n");
    g_calculate_new_centroid<<<block_dim, thread_dim, sizeof(float) * _feat_dim, 0>>>(
            _data_mat->getDev(),
            _cent_mat->getDev(),
            _assigned_vec->getDev(),
            _sample_num,
            _class_dim,
            _feat_dim);
    checkCudaErrors(cudaStreamSynchronize(0));
    getLastCudaError("g_calculate_new_centroid");
//    _cent_mat->toCpu();
//    _cent_mat->print();
//    exit(0);
}

int cukmeans::write()
{
    char out_file_name[1024];
    _assigned_vec->toCpu();
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
            fprintf(fp, "%d\n", _assigned_vec->getHost()[jj]);
        }
        fclose(fp);
    }
    return RET_OK;
}
