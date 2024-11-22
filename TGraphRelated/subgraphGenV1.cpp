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
#include <thread>
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

int* readBinary(string filename, int& elementNum)
{
    // convert the binary file to vector
    if (!fileExists(filename)) {
        fprintf(stderr, "file:%s not exist.\n", filename.c_str());
        exit(0);
    }
    long fs = fileSize(filename);
    // get the number of elements in the file and each element is 4 bytes 
    long ele_cnt = fs / sizeof(int);
    elementNum = ele_cnt;
    int* out = new int[ele_cnt];
    FILE *fp;
    fp = fopen(filename.c_str(), "r");
    // fread return the cnt of elements read
    if (fread(&out[0], sizeof(int), ele_cnt, fp) < ele_cnt) {
        fprintf(stderr, "Read failed.\n");
    }
    return out;
}

int* getdegree(int* rowPtr, int vNum)
{
    cout << "1" << endl;
    int* degree = new int[vNum];

    cout << "1" << endl;

    for(int i = 0; i < vNum; i ++)
    {
        degree[i] = rowPtr[i + 1] - rowPtr[i];
    }
    return degree;

}

void threadwork(int tid, int numThreads, int* rowPtr, int* columns, int vNum, int* degree, int* selectedVertices, int subVNum, int* subRowPtr, int* subColumns)
{
    int chunksize = (subVNum + numThreads - 1) / numThreads;
    int left = 0, right = 0;
    left = tid * chunksize;
    right = min((tid + 1) * chunksize, subVNum);
    for(int i = left; i < right; i ++)
    {
        int currentVertex = selectedVertices[i];
        int currentDegree = degree[currentVertex];
        int globalStart = rowPtr[currentVertex], localStart = subRowPtr[i];
        for(int j = 0; j < currentDegree; j ++)
        {
            subColumns[localStart + j] = columns[globalStart + j];
        }
    }
}
int main(int argc, char** argv)
{
    /*
    vlistPath: the path of csr_vlist
    elistPath: the path of csr_elist
    vertexRatio: the ratio of selected vertices
    */
    string vPath = argv[1], ePath = argv[2];
    float vertexRatio = atof(argv[3]);
    int threadNum = atoi(argv[4]);

    int vNum = 0, eNum = 0;
    int* rowPtr = readBinary(vPath, vNum);
    int* columns = readBinary(ePath, eNum);
    vNum --;
    cout << "Graph Reading Done !!" << " vNum: " << vNum <<  " eNum: " << eNum << endl;

    int* degree = new int[vNum];
    for(int i = 0; i < vNum; i ++)
    {
        degree[i] = rowPtr[i + 1] - rowPtr[i];
    }

    cout << "1" << endl;
    // generate the selected vertices in the subgraph
    int selectedVertexNum = vNum * vertexRatio;
    int* selectedVertices = new int[selectedVertexNum];
    cout << "1" << endl;
    for(int i = 0; i < selectedVertexNum; i ++)
    {
        selectedVertices[i] = i;
    }

    cout << "Selected Vertex Num: " << selectedVertexNum << endl;

    int subVNum = selectedVertexNum, subENum = 0;
    int* subRowPtr = new int[subVNum + 1];

    clock_t c0 = clock();
    // generate the sub rowptr
    for(int i = 0; i < subVNum; i ++)
    {
        subRowPtr[i] = subENum;
        subENum += degree[selectedVertices[i]];
    }
    subRowPtr[subVNum] = subENum;
    int* subColumns = new int[subENum];

    clock_t c1 = clock();
    cout << "Subgraph Preparation Done !!  Time: " << (c1 - c0) * 1.0 / CLOCKS_PER_SEC  <<endl;

    thread workThreads[threadNum];
    for(int i = 0; i < threadNum; i ++)
    {
        workThreads[i] = thread(threadwork, i, threadNum, rowPtr, columns, vNum, degree, selectedVertices, subVNum, subRowPtr, subColumns);
    }

    for(int i = 0; i < threadNum; i ++)
    {
        workThreads[i].join();
    }
    clock_t c2 = clock();
    cout << "Subgraph Generation Time: " << (c2 - c1) * 1.0 / CLOCKS_PER_SEC << "s" << endl;
    cout << "Subgraph Size: " << subENum << endl;
    
}