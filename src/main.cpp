 

//#include <iostream>
//#include <numeric>
#include <stdlib.h>

#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>
#include <inttypes.h>


#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include "main.h"

using namespace std;

void readGraphDIMACS(char* filePath, int32_t** prmoff, int32_t** prmind, int32_t* prmnv, int32_t* prmne);

// This will output the proper CUDA error strings in the event that a CUDA host call returns an error
#ifndef checkCudaErrors
#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

// These are the inline versions for all of the SDK helper functions
inline void __checkCudaErrors(cudaError_t err, const char *file, const int line)
{   
    if (cudaSuccess != err)
    {   
        std::cerr << "CUDA Error = " << err << ": " << cudaGetErrorString(err) << " from file " << file  << ", line " << line << std::endl;
        exit(EXIT_FAILURE);
    }
}
#endif

//Note: Times are returned in seconds
void start_clock(cudaEvent_t &start, cudaEvent_t &end)
{
	checkCudaErrors(cudaEventCreate(&start));
	checkCudaErrors(cudaEventCreate(&end));
	checkCudaErrors(cudaEventRecord(start,0));
}

float end_clock(cudaEvent_t &start, cudaEvent_t &end)
{
	float time;
	checkCudaErrors(cudaEventRecord(end,0));
	checkCudaErrors(cudaEventSynchronize(end));
	checkCudaErrors(cudaEventElapsedTime(&time,start,end));
	checkCudaErrors(cudaEventDestroy(start));
	checkCudaErrors(cudaEventDestroy(end));

	return time/(float)1000;
}


void generateEdgeUpdates(int32_t nv, int32_t numEdges, int32_t* edgeSrc, int32_t* edgeDst){

	for(int32_t e=0; e<numEdges; e++){
		edgeSrc[e] = rand()%nv;
		edgeDst[e] = rand()%nv;
	}

}

int main(const int argc, char *argv[])
{
    cudaSetDevice(0);
	// cudaDeviceProp prop;
	// cudaGetDeviceProperties(&prop, 0);
 	// printf("  Device name: %s\n", prop.name);

    int32_t nv, ne,*off,*adj;

    cout << argv[1] << endl;

	int numEdges=10000;
	if(argc>2)
		numEdges=atoi(argv[2]);

	srand(100);


    readGraphDIMACS(argv[1],&off,&adj,&nv,&ne);

	cout << "Vertices " << nv << endl;
	cout << "Edges " << ne << endl;

	int32_t *d_adjSizeUsed,*d_adjSizeMax,**d_adjArray;

	cudaEvent_t ce_start,ce_stop;
	start_clock(ce_start, ce_stop);
		allocGPUMemory(nv, ne, off, adj, &d_adjArray, &d_adjSizeUsed, &d_adjSizeMax);
	cout << "Allocation time : " << end_clock(ce_start, ce_stop) << endl;

	// hostMakeGPUStinger(nv,ne,off, adj,*d_adjArray,*d_adjSizeUsed,*d_adjSizeMax);
	start_clock(ce_start, ce_stop);
		hostMakeGPUStinger(nv,ne,off, adj,d_adjArray,d_adjSizeUsed,d_adjSizeMax);
	cout << "Copy time       : " << end_clock(ce_start, ce_stop) << endl;

	// int32_t* h_edgesSrc=(int32_t*)allocHostArray(numEdges,sizeof(int32_t));	
	// int32_t* h_edgesDst=(int32_t*)allocHostArray(numEdges,sizeof(int32_t));	

	// BatchUpdate bu(numEdges);
	// generateEdgeUpdates(nv, numEdges, bu.getHostSrcArray(),bu.getHostDstArray());
	// bu.resetDeviceIndCount();
	// bu.copyHostToDevice();


	// start_clock(ce_start, ce_stop);
	// 	update(nv,ne,d_adjArray,d_adjSizeUsed,d_adjSizeMax,numEdges, 
	// 		&bu);
	cout << "Update time     : " << end_clock(ce_start, ce_stop) << endl;


	// freeHostArray(h_edgesSrc);
	// freeHostArray(h_edgesDst);

	int32_t** h_adjArray = (int32_t**)allocHostArray(nv, sizeof(int32_t*));
	copyArrayDeviceToHost(d_adjArray,h_adjArray,nv, sizeof(int32_t*));
	for(int v = 0; v < nv; v++){
        freeDeviceArray(h_adjArray[v]); 
    }

	freeDeviceArray(d_adjArray);
	freeDeviceArray(d_adjSizeUsed);
	freeDeviceArray(d_adjSizeMax);

    return 0;	cout << "baabaa" << endl;

}       


