// Subgraph generation in CPU accelerated by openmp
#include <omp.h>
#include <iostream>
#include <ctime>
#include <vector>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <numeric>
#include <random>
#include <algorithm>
#include <cstring>
using namespace std;

inline bool fileExists(string filename)
{
    struct stat st;
    return stat(filename.c_str(), &st) == 0;
}

inline long fileSize(string filename)
{
    struct stat st;
    assert(stat(filename.c_str(), &st) == 0);
    return st.st_size;
}

int readBinary(string filename, vector<int>& out)
{
    // convert the binary file to vector
    if (!fileExists(filename)) {
        fprintf(stderr, "file:%s not exist.\n", filename.c_str());
        exit(0);
    }
    long fs = fileSize(filename);
    // get the number of elements in the file and each element is 4 bytes 
    long ele_cnt = fs / sizeof(int);
    out.resize(ele_cnt);
    FILE *fp;
    fp = fopen(filename.c_str(), "r");
    // fread return the cnt of elements read
    if (fread(&out[0], sizeof(int), ele_cnt, fp) < ele_cnt) {
        fprintf(stderr, "Read failed.\n");
    }
    return ele_cnt;
}

void getDegree(vector<int>& rowPtr, vector<int>& degree)
{
    int vNum = rowPtr.size() - 1;
    
    #pragma omp parallel for
    for(int i = 0; i < vNum; i ++)
    {
        degree[i] = rowPtr[i + 1] - rowPtr[i];
    }
}

int subgraphGen(vector<int>& rowPtr, vector<int>& columns, vector<int>& degree, vector<int>& selectedVertices, vector<int>& subRowPtr, vector<int>& subColumns)
{
    int subVNum = selectedVertices.size();
    vector<int> subDegree(subVNum);
    // vector<int> subRowPtr(subVNum + 1);

    #pragma omp parallel for 
    for(int i = 0; i < subVNum; i ++)
    {
        subDegree[i] = degree[selectedVertices[i]];
    }

    cout << "subDegree Generation Done !!" << endl;

    int degreeSum = 0;
    for(int i = 0; i < subVNum; i ++) degreeSum += subDegree[i];
    subColumns.resize(degreeSum);
    cout << "degree generation done !!" << endl;

    // 串行执行
    for(int i = 0; i < subVNum; i ++) subRowPtr[i + 1] = subRowPtr[i] + degree[i];

    cout << "subgraph rowptr generation done !!" << endl;

    #pragma omp parallel for schedule(dynamic) 
    for(int i = 0; i < subVNum; i ++)
    {
        int vertex = selectedVertices[i];
        int start = rowPtr[vertex], end = rowPtr[vertex + 1];
        int sz = end - start;
        int substart = subRowPtr[i];
        // memcpy(subColumns.data() + subRowPtr[i], columns.data() + start, end - start);
        memcpy(&subColumns[substart], &columns[start], sz * sizeof(int));

        // int substart = subRowPtr[i];
        // while(start < end)
        // {
        //     subColumns[substart] = columns[start];
        //     start ++;
        //     substart ++;
        // }
    }

    return degreeSum;
}

// bool check(vector<int>& rowPtr, vector<int>& columns, vector<int>& degree, vector<int>& selectedVertices, vector<int>& subRowPtr, vector<int>& subColumns)
// {
//     int subNum = selectedVertices
//     vector<int> subDegree()
// }
int main(int argc, char** argv)
{
    /*
    vlistPath: the path of csr_vlist
    elistPath: the path of csr_elist
    vertexRatio: the ratio of selected vertices
    */
    
    clock_t c1 = clock();
    string vPath = argv[1], ePath = argv[2];
    float vertexRatio = atof(argv[3]);

    vector<int> rowPtr;
    vector<int> columns;

    int vNum = readBinary(vPath, rowPtr);
    int eNum = readBinary(ePath, columns);

    for(int i = 0; i < 10; i ++) std::cout << rowPtr[i] << " ";
    cout << endl;
    for(int i = 0; i < 10; i ++) std::cout << columns[i] << " ";

    std::cout << "vNum: " << vNum << " eNum: " << eNum << std::endl;
    clock_t c2 = clock();

    cout << "Graph Reading Time: " << double(c2 - c1) / CLOCKS_PER_SEC << " s" << endl;
    vNum --;
    vector<int> degree(vNum);
    vector<int> vertices(vNum);
    for(int i = 0; i < vNum; i ++) vertices[i] = i;

    // get the vertex degree
    getDegree(rowPtr, degree);

    clock_t c3 = clock();
    cout << "Get Degree Time: " << double(c3 - c2) / CLOCKS_PER_SEC <<  " s" << endl;

    int selectedVertexNum = vNum * vertexRatio;
    vector<int> selectedVertices(vertices.begin(), vertices.begin() + selectedVertexNum);
    
    vector<int> subRowPtr(selectedVertexNum + 1);
    vector<int> subColumns(selectedVertexNum);

    cout << "begin subgraph generation " << endl;
    int sum = subgraphGen(rowPtr, columns, degree, selectedVertices, subRowPtr, subColumns);

    clock_t c4 = clock();
    cout << "SubGraph Generation Time: " << double(c4 - c3) / CLOCKS_PER_SEC <<  " s" << "  subgraph size: " << sum << endl;
}