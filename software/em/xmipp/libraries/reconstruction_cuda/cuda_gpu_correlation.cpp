
//Host includes
#include "cuda_gpu_correlation.h"

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <algorithm>


//CUDA includes
#include "cuda_basic_math.h"
#include "cuda_utils.h"
#include <time.h>
#include <sys/time.h>
#include <vector>

#define PI 3.14159265



__global__ void sumRadiusKernel(float *d_in, float *d_out, float *d_out_max, float *d_out_zero, size_t dim, size_t radius, size_t ndim){

	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned int numIm = floorf(idx/360);
	unsigned int angle = idx%360;

	if(idx>=dim)
		return;

	d_out[idx]=0.0;
	d_out_max[idx]=-100000;
	int idxRead=360*radius*numIm;
	for(int i=0; i<radius; i++){
		if(d_in[idxRead+(360*i)+angle]==-1.0){
			continue;
		}
		d_out[idx] += d_in[idxRead+(360*i)+angle];
		if(d_in[idxRead+(360*i)+angle]>d_out_max[idx])
			d_out_max[idx] = d_in[idxRead+(360*i)+angle];

		if(i==0)
			d_out_zero[idx] = d_in[idxRead+angle];
	}

}


__global__ void calculateMax(float *d_in, float *d_out, float *position, size_t yxdim, int Ndim, bool firstCall){

	extern __shared__ float sdata[];

	unsigned int idx = threadIdx.x;
	unsigned int blockSize = blockDim.x;

	//Version 6
	unsigned int i = blockIdx.x * blockSize*2 + idx;
	int index = 0;
	for(int imN=0; imN<Ndim; imN++){

		if(i<yxdim){
			if(i+blockSize < yxdim)
				sdata[idx]=fmaxf(d_in[i+index], d_in[i+blockSize+index]);
			else
				sdata[idx]=d_in[i+index];

			if(firstCall)
				sdata[idx+blockSize] = (sdata[idx]==d_in[i+index]) ? (float)i : (float)(i+blockSize); //AJ position
			else
				sdata[idx+blockSize] = (sdata[idx]==d_in[i+index]) ? position[i+index] : position[i+blockSize+index];
		}

		__syncthreads();

		if(i>=yxdim)
			sdata[idx]=-1.0;
		__syncthreads();


		if(blockSize >= 1024){
			if(idx<512){
				sdata[idx] = fmaxf(sdata[idx], sdata[idx+512]);
				sdata[idx+blockSize] = (sdata[idx]==sdata[idx+512]) ? sdata[idx+blockSize+512] : sdata[idx+blockSize];
			}
			__syncthreads();
		}
		if(blockSize >= 512){
			if(idx<256){
				sdata[idx] = fmaxf(sdata[idx], sdata[idx+256]);
				sdata[idx+blockSize] = (sdata[idx]==sdata[idx+256]) ? sdata[idx+blockSize+256] : sdata[idx+blockSize];
			}
			__syncthreads();
		}
		if(blockSize >= 256){
			if(idx<128){
				sdata[idx] = fmaxf(sdata[idx], sdata[idx+128]);
				sdata[idx+blockSize] = (sdata[idx]==sdata[idx+128]) ? sdata[idx+blockSize+128] : sdata[idx+blockSize];
			}
			__syncthreads();
		}
		if(blockSize >= 128){
			if(idx<64){
				sdata[idx] = fmaxf(sdata[idx], sdata[idx+64]);
				sdata[idx+blockSize] = (sdata[idx]==sdata[idx+64]) ? sdata[idx+blockSize+64] : sdata[idx+blockSize];
			}
			__syncthreads();
		}
		if(idx<32){
			if(blockSize>=64){
				if(idx<32){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+32]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+32]) ? sdata[idx+blockSize+32] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=32){
				if(idx<16){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+16]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+16]) ? sdata[idx+blockSize+16] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=16){
				if(idx<8){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+8]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+8]) ? sdata[idx+blockSize+8] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=8){
				if(idx<4){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+4]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+4]) ? sdata[idx+blockSize+4] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=4){
				if(idx<2){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+2]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+2]) ? sdata[idx+blockSize+2] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=2){
				if(idx<1){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+1]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+1]) ? sdata[idx+blockSize+1] : sdata[idx+blockSize];
				}
			}
		}

		if(idx==0){
			d_out[blockIdx.x+(gridDim.x*imN)] = sdata[0];
			position[blockIdx.x+(gridDim.x*imN)] = sdata[blockSize];
		}

		__syncthreads();

		index = index+(int)yxdim;

	}

}



__global__ void calculateMax2(float *d_in, float *d_out, float *position, size_t yxdim, int Ndim, bool firstCall){

	extern __shared__ float sdata[];

	unsigned int idx = threadIdx.x;
	unsigned int blockSize = blockDim.x;


	//Version 6
	unsigned int i = blockIdx.x * blockSize + idx;

	//printf("d_in[%i] %f \n", i, d_in[i]);

	int index = 0;
	for(int imN=0; imN<Ndim; imN++){

		if(i<yxdim*gridDim.x){
			sdata[idx]=d_in[i+index];

			if(firstCall)
				sdata[idx+blockSize] = (float)idx; //AJ position
			else
				sdata[idx+blockSize] = position[i+index];
		}
		//if (idx==0)
			//printf("i %i, sdata %f \n", i, sdata[idx]);

		__syncthreads();

		if(i>=yxdim*gridDim.x)
			sdata[idx]=-1.0;
		__syncthreads();


		if(blockSize >= 1024){
			if(idx<512){
				sdata[idx] = fmaxf(sdata[idx], sdata[idx+512]);
				sdata[idx+blockSize] = (sdata[idx]==sdata[idx+512]) ? sdata[idx+blockSize+512] : sdata[idx+blockSize];
			}
			__syncthreads();
		}
		if(blockSize >= 512){
			if(idx<256){
				sdata[idx] = fmaxf(sdata[idx], sdata[idx+256]);
				sdata[idx+blockSize] = (sdata[idx]==sdata[idx+256]) ? sdata[idx+blockSize+256] : sdata[idx+blockSize];
			}
			__syncthreads();
		}
		if(blockSize >= 256){
			if(idx<128){
				sdata[idx] = fmaxf(sdata[idx], sdata[idx+128]);
				sdata[idx+blockSize] = (sdata[idx]==sdata[idx+128]) ? sdata[idx+blockSize+128] : sdata[idx+blockSize];
			}
			__syncthreads();
		}
		if(blockSize >= 128){
			if(idx<64){
				sdata[idx] = fmaxf(sdata[idx], sdata[idx+64]);
				sdata[idx+blockSize] = (sdata[idx]==sdata[idx+64]) ? sdata[idx+blockSize+64] : sdata[idx+blockSize];
			}
			__syncthreads();
		}
		if(idx<32){
			if(blockSize>=64){
				if(idx<32){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+32]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+32]) ? sdata[idx+blockSize+32] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=32){
				if(idx<16){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+16]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+16]) ? sdata[idx+blockSize+16] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=16){
				if(idx<8){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+8]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+8]) ? sdata[idx+blockSize+8] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=8){
				if(idx<4){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+4]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+4]) ? sdata[idx+blockSize+4] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=4){
				if(idx<2){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+2]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+2]) ? sdata[idx+blockSize+2] : sdata[idx+blockSize];
				}
			}
			if(blockSize>=2){
				if(idx<1){
					sdata[idx] = fmaxf(sdata[idx], sdata[idx+1]);
					sdata[idx+blockSize] = (sdata[idx]==sdata[idx+1]) ? sdata[idx+blockSize+1] : sdata[idx+blockSize];
				}
			}
		}

		if(idx==0){
			//printf("idx %i sdata[0] %f sdata[blockSize] %f \n", blockIdx.x+(gridDim.x*imN), sdata[0], sdata[blockSize]);
			d_out[blockIdx.x+(gridDim.x*imN)] = sdata[0];
			position[blockIdx.x+(gridDim.x*imN)] = sdata[blockSize];
		}

		__syncthreads();

		index = index+(int)yxdim;

	}
	//printf("d_out0 %f, position0 %f \n", d_out[0], position[0]);
	//printf("d_out1 %f, position1 %f \n", d_out[1], position[1]);

}

__global__ void matrixMultiplication (float* newMat, float* lastMat, float* result, size_t n, double maxShift,
		float *maxGpu, float *NCC, size_t NCC_yxdim){

	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	if(idx>=n)
		return;

	double maxShift2 = maxShift*maxShift;

	int idx9 = idx*9;
	float shiftx = newMat[idx9]*lastMat[idx9+2] + newMat[idx9+1]*lastMat[idx9+5] + newMat[idx9+2]*lastMat[idx9+8];
	float shifty = newMat[idx9+3]*lastMat[idx9+2] + newMat[idx9+4]*lastMat[idx9+5] + newMat[idx9+5]*lastMat[idx9+8];
	float radShift = shiftx*shiftx + shifty*shifty;
	if(radShift > maxShift2){
		result[idx9] = lastMat[idx9];
		result[idx9+1] = lastMat[idx9+1];
		result[idx9+2] = lastMat[idx9+2];
		result[idx9+3] = lastMat[idx9+3];
		result[idx9+4] = lastMat[idx9+4];
		result[idx9+5] = lastMat[idx9+5];
		maxGpu[idx] = NCC[idx*NCC_yxdim];
	}else{
		result[idx9] = newMat[idx9]*lastMat[idx9] + newMat[idx9+1]*lastMat[idx9+3] + newMat[idx9+2]*lastMat[idx9+6];
		result[idx9+2] = shiftx;
		result[idx9+1] = newMat[idx9]*lastMat[idx9+1] + newMat[idx9+1]*lastMat[idx9+4] + newMat[idx9+2]*lastMat[idx9+7];
		result[idx9+3] = newMat[idx9+3]*lastMat[idx9] + newMat[idx9+4]*lastMat[idx9+3] + newMat[idx9+5]*lastMat[idx9+6];
		result[idx9+4] = newMat[idx9+3]*lastMat[idx9+1] + newMat[idx9+4]*lastMat[idx9+4] + newMat[idx9+5]*lastMat[idx9+7];
		result[idx9+5] = shifty;
	}

}

__global__ void pointwiseMultiplicationComplexOneManyKernel(cufftComplex *M, cufftComplex *manyF, cufftComplex *MmanyF,
		size_t nzyxdim, size_t yxdim)
{
	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned long int idxLow = idx%yxdim;

	if (idx>=nzyxdim)
		return;

	float normFactor = (1.0/yxdim);

	cuComplex mulOut = cuCmulf(manyF[idx], M[idxLow]);

	MmanyF[idx] = mulOut*normFactor;
}

__global__ void calculateDenomFunctionKernel(float *MFrealSpace, float *MF2realSpace, float *maskAutocorrelation, float *out,
		size_t nzyxdim, size_t yxdim)
{
	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned long int idxLow = idx%yxdim;

	if (idx>=nzyxdim)
		return;

	out[idx] = sqrt(MF2realSpace[idx] - (MFrealSpace[idx]*MFrealSpace[idx]/maskAutocorrelation[idxLow]));

}


__global__ void calculateNccKernel(float *RefExpRealSpace, float *MFrealSpaceRef, float *MFrealSpaceExp,
		float *MF2realSpaceRef, float *MF2realSpaceExp, float *mask, float *NCC,
		size_t nzyxdim, size_t yxdim, size_t xdim, size_t ydim, size_t maskCount, int max_shift)
{

	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned long int idxLow = idx % yxdim;

	if(idx>=nzyxdim)
		return;

	int idx_x = idxLow%xdim;
	int idx_y=idxLow/xdim;
	if(idx_x>=max_shift && idx_x<xdim-max_shift){
		NCC[idx] = -1;
		return;
	}
	if(idx_y>=max_shift && idx_y<ydim-max_shift){
		NCC[idx] = -1;
		return;
	}

	float den1 = sqrt(MF2realSpaceRef[idx] - (MFrealSpaceRef[idx]*MFrealSpaceRef[idx]/mask[idxLow]));
	float den2 = sqrt(MF2realSpaceExp[idx] - (MFrealSpaceExp[idx]*MFrealSpaceExp[idx]/mask[idxLow]));

	if(den1!=0.0 && den2!=0.0 && !isnan(den1) && !isnan(den2) && mask[idxLow]>maskCount*0.9){
		float num = (RefExpRealSpace[idx] - ((MFrealSpaceRef[idx]*MFrealSpaceExp[idx])/(mask[idxLow])) );
		NCC[idx] = num/(den1*den2);
	}else
		NCC[idx] = -1;

}

__device__ void wrapping (int &x, int &y, size_t xdim, size_t ydim){
		//mirror wrapping
		if(x<0)
			x=-x;
		else if(x>=xdim)
			x=xdim-(x-xdim)-1;
		if(y<0)
			y=-y;
		else if(y>=ydim)
			y=ydim-(y-ydim)-1;
}

__global__ void applyTransformKernel(float *d_in, float *d_out, float *transMat, size_t nzyxdim, size_t yxdim,
		size_t xdim, size_t ydim)
{

	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned int idxIm = idx%yxdim;
	unsigned int numIm = idx/yxdim;


	if(idx>=nzyxdim)
		return;

	float x = idxIm%xdim;
	float y = idxIm/xdim;
	float x_orig = 0;
	float y_orig = 0;

	x -= transMat[2+(numIm*9)];
	y -= transMat[5+(numIm*9)];

	x = x - xdim/2;
	y = y - ydim/2;

	x_orig += transMat[(numIm*9)]*x - transMat[1+(numIm*9)]*y + xdim/2;
	y_orig += -transMat[3+(numIm*9)]*x + transMat[4+(numIm*9)]*y + ydim/2;

	int x_orig00 = (int)floorf(x_orig);
	int y_orig00 = (int)floorf(y_orig);
	int x_orig01 = x_orig00+1;
	int y_orig01 = y_orig00;
	int x_orig10 = x_orig00;
	int y_orig10 = y_orig00+1;
	int x_orig11 = x_orig00+1;
	int y_orig11 = y_orig00+1;

	float x_x_low=x_orig-x_orig00;
	float y_y_low=y_orig-y_orig00;
	float one_x=1.0-x_x_low;
	float one_y=1.0-y_y_low;
	float w00=one_y*one_x;
	float w01=one_y*x_x_low;
	float w10=y_y_low*one_x;
	float w11=y_y_low*x_x_low;

	wrapping (x_orig00, y_orig00, xdim, ydim);
	wrapping (x_orig01, y_orig01, xdim, ydim);
	wrapping (x_orig10, y_orig10, xdim, ydim);
	wrapping (x_orig11, y_orig11, xdim, ydim);

	int imgIdx00=y_orig00 * xdim + x_orig00;
	int imgIdx01=y_orig01 * xdim + x_orig01;
	int imgIdx10=y_orig10 * xdim + x_orig10;
	int imgIdx11=y_orig11 * xdim + x_orig11;

	int imgOffset = numIm*yxdim;
	float I00 = d_in[imgIdx00+imgOffset];
	float I01 = d_in[imgIdx01+imgOffset];
	float I10 = d_in[imgIdx10+imgOffset];
	float I11 = d_in[imgIdx11+imgOffset];
	float imVal = I00*w00 + I01*w01 + I10*w10 + I11*w11;

	d_out[idx] = imVal;

}



__global__ void calculateNccRotationKernel(float *RefExpRealSpace, cufftComplex *polarFFTRef, cufftComplex *polarFFTExp,
		cufftComplex *polarSquaredFFTRef, cufftComplex *polarSquaredFFTExp,	float maskFFTPolarReal, float *NCC,
		size_t yxdimFFT, size_t nzyxdim, size_t yxdim)
{

	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned long int idxLow = (idx/(int)yxdim)*(int)yxdimFFT;

	if(idx>=nzyxdim)
		return;

	float normValue = 1.0/yxdimFFT;
	float maskNorm = maskFFTPolarReal*normValue;


	float M1M2Polar = maskFFTPolarReal*maskNorm;
	float polarValRef = cuCrealf(polarFFTRef[idxLow])*maskNorm;
	float polarSqValRef = cuCrealf(polarSquaredFFTRef[idxLow])*maskNorm;

	float polarValExp = cuCrealf(polarFFTExp[idxLow])*maskNorm;
	float polarSqValExp = cuCrealf(polarSquaredFFTExp[idxLow])*maskNorm;

	float num = (RefExpRealSpace[idx] - (polarValRef*polarValExp/M1M2Polar) );
	float den1 = sqrt(polarSqValRef - (polarValRef*polarValRef/M1M2Polar) );
	float den2 = sqrt(polarSqValExp - (polarValExp*polarValExp/M1M2Polar) );

	if(den1!=0.0 && den2!=0.0 && !isnan(den1) && !isnan(den2))
		NCC[idx] = num/(den1*den2);
	else
		NCC[idx] = -1.0;

}


__global__ void pointwiseMultiplicationComplexKernel(cufftComplex *reference, cufftComplex *experimental,
		cufftComplex *RefExpFourier, size_t nzyxdim, size_t yxdim)
{
	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;

	if(idx>=nzyxdim)
		return;

	float normFactor = (1.0/yxdim);

	cuComplex mulOut = cuCmulf(reference[idx], experimental[idx]);
	RefExpFourier[idx] = mulOut*normFactor;
}

__global__ void maskingPaddingKernel(float *d_in, float *mask, float *padded_image_gpu,
		float *padded_image2_gpu, float *padded_mask_gpu, size_t xdim, size_t ydim, size_t yxdim,
		size_t numImag, size_t pad_xdim, size_t pad_ydim, size_t pad_yxdim, bool experimental){

	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;

	if(idx>=yxdim)
		return;

	unsigned int x_idx1 = idx%(int)xdim;
	unsigned int y_idx1 = idx/(int)xdim;
	unsigned int idxWriteToMask;
	if(experimental)
		idxWriteToMask = (ydim-1 - y_idx1)*xdim + (xdim-1 - x_idx1);
	else
		idxWriteToMask = y_idx1*xdim + x_idx1;

	int xdim2Im = (int)floorf((pad_xdim-xdim)/2);
	int ydim2Im = (int)floorf((pad_ydim-ydim)/2);
	int xdim2Mask = xdim2Im;
	int ydim2Mask = ydim2Im;
	if(experimental && xdim%2==0){
		xdim2Im+=1;
		ydim2Im+=1;
	}

	unsigned int x_idx = idxWriteToMask%(int)xdim;
	unsigned int y_idx = idxWriteToMask/(int)xdim;
	unsigned int idxWrite;
	unsigned int idxWriteMask;
	float d_out, d_out2;

	int offset=0;
	for(int j=0; j<numImag; j++){

		d_out = d_in[idx+offset]*mask[idx];
		d_out2 = d_out*d_out;

		idxWrite = (pad_yxdim*j) + (ydim2Im*pad_xdim) + (y_idx*pad_xdim) + xdim2Im + x_idx;
		if(xdim%2==0)
			idxWriteMask = (pad_yxdim*j) + (ydim2Mask*pad_xdim) + (y_idx*pad_xdim) + xdim2Mask + x_idx;
		else
			idxWriteMask = idxWrite;
		padded_image_gpu[idxWrite] = d_out;
		padded_image2_gpu[idxWrite] = d_out2;
		if(j==0 && padded_mask_gpu!=NULL)
			padded_mask_gpu[idxWriteMask] = mask[idx];
		offset += yxdim;
	}
}


__global__ void buildTranslationMatrix (float *d_pos, float *newMat, float* lastMat, float* result,
		float *maxGpu, float *NCC, size_t Xdim, size_t Ydim, size_t Ndim, size_t NCC_yxdim,
		int fixPadding, double maxShift){

	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;

	if(idx>=Ndim)
		return;

	int position = (int)d_pos[idx];

	float posX_aux = (float)(position%Xdim);
	float posY_aux = (float)(position/Xdim);
	float Xdim2 = (float)(Xdim/2);
	float Ydim2 = (float)(Ydim/2);
	float posX, posY;

	if(posX_aux>=Xdim2 && posY_aux>=Ydim2){
		posX = Xdim-1-posX_aux;
		posY = Ydim-1-posY_aux;
	}else if(posX_aux<Xdim2 && posY_aux>=Ydim2){
		posX = -(posX_aux+1);
		posY = Ydim-1-posY_aux;
	}else if(posX_aux<Xdim2 && posY_aux<Ydim2){
		posX = -(posX_aux+1);
		posY = -(posY_aux+1);
	}else if(posX_aux>=Xdim2 && posY_aux<Ydim2){
		posX = Xdim-1-posX_aux;
		posY = -(posY_aux+1);
	}

	//printf("Idx %i, posX %f, posY %f, fixPadding %i, Xdim %i, Ydim %i \n", idx, posX, posY, fixPadding, Xdim, Ydim);

	//Fixing padding problem
	posX+=fixPadding;
	posY+=fixPadding;

	int idx9 = idx*9;
	newMat[idx9]=1;
	newMat[idx9+1]=0;
	newMat[idx9+2]=-posX;
	newMat[idx9+3]=0;
	newMat[idx9+4]=1;
	newMat[idx9+5]=-posY;
	newMat[idx9+6]=0;
	newMat[idx9+7]=0;
	newMat[idx9+8]=1;

	double maxShift2 = maxShift*maxShift;

	float shiftx = newMat[idx9]*lastMat[idx9+2] + newMat[idx9+1]*lastMat[idx9+5] + newMat[idx9+2]*lastMat[idx9+8];
	float shifty = newMat[idx9+3]*lastMat[idx9+2] + newMat[idx9+4]*lastMat[idx9+5] + newMat[idx9+5]*lastMat[idx9+8];
	float radShift = shiftx*shiftx + shifty*shifty;
	if(radShift > maxShift2){
		result[idx9] = lastMat[idx9];
		result[idx9+1] = lastMat[idx9+1];
		result[idx9+2] = lastMat[idx9+2];
		result[idx9+3] = lastMat[idx9+3];
		result[idx9+4] = lastMat[idx9+4];
		result[idx9+5] = lastMat[idx9+5];
		maxGpu[idx] = NCC[idx*NCC_yxdim];
	}else{
		result[idx9] = newMat[idx9]*lastMat[idx9] + newMat[idx9+1]*lastMat[idx9+3] + newMat[idx9+2]*lastMat[idx9+6];
		result[idx9+2] = shiftx;
		result[idx9+1] = newMat[idx9]*lastMat[idx9+1] + newMat[idx9+1]*lastMat[idx9+4] + newMat[idx9+2]*lastMat[idx9+7];
		result[idx9+3] = newMat[idx9+3]*lastMat[idx9] + newMat[idx9+4]*lastMat[idx9+3] + newMat[idx9+5]*lastMat[idx9+6];
		result[idx9+4] = newMat[idx9+3]*lastMat[idx9+1] + newMat[idx9+4]*lastMat[idx9+4] + newMat[idx9+5]*lastMat[idx9+7];
		result[idx9+5] = shifty;
	}

}



__global__ void buildRotationMatrix (float *d_pos, float *newMat, float* lastMat, float* result,
		float *maxGpu, float *auxMax, float *NCC, size_t Xdim, size_t Ndim, size_t NCC_yxdim,
		int fixPadding, double maxShift){

	unsigned long int idx = blockDim.x * blockIdx.x + threadIdx.x;

	if(idx>=Ndim)
		return;

	float posX;
	int position = (int)d_pos[idx];
	maxGpu[idx] = auxMax[position+(idx*360)];

	float posX_aux = (float)(position%Xdim);
	float Xdim2 = (float)(Xdim/2);


	if(posX_aux<Xdim2){
		posX = -(posX_aux+1);
	}else if(posX_aux>=Xdim2){
		posX = Xdim-1-posX_aux;
	}

	//Fixing padding problem
	posX+=fixPadding;

	float rad = (float)(-posX*PI/180);

	int idx9 = idx*9;
	newMat[idx9]=cosf(rad);
	newMat[idx9+1]=-sinf(rad);
	newMat[idx9+2]=0;
	newMat[idx9+3]=sinf(rad);
	newMat[idx9+4]=cosf(rad);
	newMat[idx9+5]=0;
	newMat[idx9+6]=0;
	newMat[idx9+7]=0;
	newMat[idx9+8]=1;

	double maxShift2 = maxShift*maxShift;

	float shiftx = newMat[idx9]*lastMat[idx9+2] + newMat[idx9+1]*lastMat[idx9+5] + newMat[idx9+2]*lastMat[idx9+8];
	float shifty = newMat[idx9+3]*lastMat[idx9+2] + newMat[idx9+4]*lastMat[idx9+5] + newMat[idx9+5]*lastMat[idx9+8];
	float radShift = shiftx*shiftx + shifty*shifty;
	if(radShift > maxShift2){
		result[idx9] = lastMat[idx9];
		result[idx9+1] = lastMat[idx9+1];
		result[idx9+2] = lastMat[idx9+2];
		result[idx9+3] = lastMat[idx9+3];
		result[idx9+4] = lastMat[idx9+4];
		result[idx9+5] = lastMat[idx9+5];
		maxGpu[idx] = NCC[idx*NCC_yxdim];
	}else{
		result[idx9] = newMat[idx9]*lastMat[idx9] + newMat[idx9+1]*lastMat[idx9+3] + newMat[idx9+2]*lastMat[idx9+6];
		result[idx9+2] = shiftx;
		result[idx9+1] = newMat[idx9]*lastMat[idx9+1] + newMat[idx9+1]*lastMat[idx9+4] + newMat[idx9+2]*lastMat[idx9+7];
		result[idx9+3] = newMat[idx9+3]*lastMat[idx9] + newMat[idx9+4]*lastMat[idx9+3] + newMat[idx9+5]*lastMat[idx9+6];
		result[idx9+4] = newMat[idx9+3]*lastMat[idx9+1] + newMat[idx9+4]*lastMat[idx9+4] + newMat[idx9+5]*lastMat[idx9+7];
		result[idx9+5] = shifty;
	}

}


void padding_masking(GpuMultidimArrayAtGpu<float> &d_orig_image, GpuMultidimArrayAtGpu<float> &mask, GpuMultidimArrayAtGpu<float> &padded_image_gpu,
		GpuMultidimArrayAtGpu<float> &padded_image2_gpu, GpuMultidimArrayAtGpu<float> &padded_mask_gpu, bool experimental){

    int numTh = 1024;
	int numBlk = d_orig_image.yxdim/numTh;
	if(d_orig_image.yxdim%numTh > 0)
		numBlk++;

	gpuErrchk(cudaMemsetAsync(padded_image_gpu.d_data, 0, padded_image_gpu.nzyxdim*sizeof(float)));
	gpuErrchk(cudaMemsetAsync(padded_image2_gpu.d_data, 0, padded_image2_gpu.nzyxdim*sizeof(float)));
	if(padded_mask_gpu.d_data!=NULL)
		gpuErrchk(cudaMemsetAsync(padded_mask_gpu.d_data, 0, padded_mask_gpu.nzyxdim*sizeof(float)));

	maskingPaddingKernel<<< numBlk, numTh >>>(d_orig_image.d_data, mask.d_data,
			padded_image_gpu.d_data, padded_image2_gpu.d_data, padded_mask_gpu.d_data,
			d_orig_image.Xdim, d_orig_image.Ydim, d_orig_image.yxdim, d_orig_image.Ndim,
			padded_image_gpu.Xdim, padded_image_gpu.Ydim, padded_image_gpu.yxdim, experimental);

}


void pointwiseMultiplicationFourier(const GpuMultidimArrayAtGpu< std::complex<float> > &M, const GpuMultidimArrayAtGpu < std::complex<float> >& manyF,
		GpuMultidimArrayAtGpu< std::complex<float> > &MmanyF)
{
    int numTh = 1024;
    XmippDim3 blockSize(numTh, 1, 1), gridSize;
    manyF.calculateGridSizeVectorized(blockSize, gridSize);

    pointwiseMultiplicationComplexOneManyKernel <<< CONVERT2DIM3(gridSize), CONVERT2DIM3(blockSize) >>>
			((cufftComplex*)M.d_data, (cufftComplex*)manyF.d_data, (cufftComplex*) MmanyF.d_data, manyF.nzyxdim, manyF.yxdim);

}

void calculateDenomFunction(const GpuMultidimArrayAtGpu< float > &MFrealSpace, const GpuMultidimArrayAtGpu < float >& MF2realSpace,
		const GpuMultidimArrayAtGpu < float >& maskAutocorrelation, GpuMultidimArrayAtGpu< float > &out)
{
    int numTh = 1024;
    XmippDim3 blockSize(numTh, 1, 1), gridSize;
    MFrealSpace.calculateGridSizeVectorized(blockSize, gridSize);

    calculateDenomFunctionKernel <<< CONVERT2DIM3(gridSize), CONVERT2DIM3(blockSize) >>>
			(MFrealSpace.d_data, MF2realSpace.d_data, maskAutocorrelation.d_data, out.d_data, MFrealSpace.nzyxdim, MFrealSpace.yxdim);

}



void GpuCorrelationAux::produceSideInfo(mycufftHandle &myhandlePaddedB, mycufftHandle &myhandleMaskB, StructuresAux &myStructureAux)
{
	myStructureAux.MF.resize(d_projFFT);
	myStructureAux.MF2.resize(d_projSquaredFFT);

	pointwiseMultiplicationFourier(d_maskFFT, d_projFFT, myStructureAux.MF);
	pointwiseMultiplicationFourier(d_maskFFT, d_projSquaredFFT, myStructureAux.MF2);

	MF2realSpace.resize(Xdim, Ydim, d_projFFT.Zdim, d_projFFT.Ndim);
	MFrealSpace.resize(Xdim, Ydim, d_projFFT.Zdim, d_projFFT.Ndim);

	myStructureAux.MF.ifft(MFrealSpace, myhandlePaddedB);
	myStructureAux.MF2.ifft(MF2realSpace, myhandlePaddedB);

	GpuMultidimArrayAtGpu< std::complex<float> > maskAux(d_projFFT.Xdim, d_projFFT.Ydim);
	pointwiseMultiplicationFourier(d_maskFFT, d_maskFFT, maskAux);
	maskAutocorrelation.resize(Xdim, Ydim);
	maskAux.ifft(maskAutocorrelation, myhandleMaskB);
	maskAux.clear();

}



void GpuCorrelationAux::produceSideInfo(mycufftHandle &myhandlePaddedB, mycufftHandle &myhandleMaskB, StructuresAux &myStructureAux,
		GpuMultidimArrayAtGpu<float> &maskAutocorr)
{
	myStructureAux.MF.resize(d_projFFT);
	myStructureAux.MF2.resize(d_projSquaredFFT);

	pointwiseMultiplicationFourier(d_maskFFT, d_projFFT, myStructureAux.MF);
	pointwiseMultiplicationFourier(d_maskFFT, d_projSquaredFFT, myStructureAux.MF2);

	MF2realSpace.resize(Xdim, Ydim, d_projFFT.Zdim, d_projFFT.Ndim);
	MFrealSpace.resize(Xdim, Ydim, d_projFFT.Zdim, d_projFFT.Ndim);

	myStructureAux.MF.ifft(MFrealSpace, myhandlePaddedB);
	myStructureAux.MF2.ifft(MF2realSpace, myhandlePaddedB);

	maskAutocorr.copyGpuToGpu(maskAutocorrelation);

}



void calculateMaxNew1D(int xdim, int Ndim, float *d_data,
		GpuMultidimArrayAtGpu<float> &d_out, GpuMultidimArrayAtGpu<float> &d_pos){

    int numTh = 1024;
    int numBlk = xdim/1024;
    if(xdim%1024!=0)
    	numBlk++;
    numBlk=ceil((float)numBlk/2);
    int numBlk2, size_aux2;

    d_out.resize(numBlk*Ndim);
    d_pos.resize(numBlk*Ndim);

	calculateMax<<<numBlk, numTh, 2*numTh * sizeof(float)>>>(d_data, d_out.d_data, d_pos.d_data, xdim, Ndim, true);

	numBlk2=numBlk;
	size_aux2=numBlk;
	while(1){
		if(numBlk2>numTh){
		   numBlk2=numBlk2/numTh;
		   if(numBlk2%numTh!=0)
			   numBlk2++;
		   numBlk=ceil((float)numBlk/2);
		}else{
			numTh=ceil((float)size_aux2/2);
			float aux1 = log((float)numTh)/log(2.0);
			int aux2 = (int)aux1;
			float error = aux1-(float)aux2;
			if(error>0.001)
				aux2++;
			numTh=pow(2,aux2);
			numBlk2=1;
		}
		calculateMax<<<numBlk2, numTh, 2*numTh * sizeof(float)>>> (d_out.d_data, d_out.d_data, d_pos.d_data, size_aux2, Ndim, false);
		size_aux2=numBlk2;
		if(numBlk2==1)
			break;
   }

/*	float *h_pos = new float[Ndim];
	cudaMemcpy(h_pos, d_pos.d_data, Ndim*sizeof(float), cudaMemcpyDeviceToHost);
	float *h_auxMax = new float[xdim*Ndim];
	cudaMemcpy(h_auxMax, auxMax, xdim*Ndim*sizeof(float), cudaMemcpyDeviceToHost);

	for(int i=0; i<Ndim; i++){

		int position = (int)h_pos[i];
		max_values[i] = h_auxMax[position+(i*360)];

		float posX_aux = (float)(position%xdim);
		float Xdim2 = (float)(xdim/2);


		if(posX_aux<Xdim2){
			posX[i] = -(posX_aux+1);
		}else if(posX_aux>=Xdim2){
			posX[i] = xdim-1-posX_aux;
		}

		//Fixing padding problem
		posX[i]+=fixPadding;

	}*/

}


__global__ void calculateMaxThreads (float *d_in, float *d_out, float *position,
		size_t yxdim, int Ndim){

	unsigned int idx = threadIdx.x;
	unsigned int nIm = blockIdx.x;
	unsigned int blockSize = blockDim.x;

	unsigned int posTh = nIm*yxdim + idx;
	int n = ceilf(((float)yxdim/(float)blockSize));

	//printf("n %i\n", n);
	float tmp, tmpPos;

	if(idx>=yxdim){
		tmp = -1.0;
		tmpPos = -1.0;
	}else{
		tmp = d_in[posTh];
		tmpPos = idx;
		for (int i=1; i<n; i++){
			//printf("posTh+i*blockSize %i, yxdim*(nIm+1) %i\n", posTh+i*blockSize, yxdim*(nIm+1));
			if (posTh+i*blockSize < yxdim*(nIm+1)){

					//printf("i %i posTh %i tmp %f posTh+i*blockSize %i d_in[posTh+i*blockSize] %f \n", i, posTh, tmp, posTh+i*blockSize, d_in[posTh+i*blockSize]);

				tmp = fmaxf(tmp, d_in[posTh+i*blockSize]);
				tmpPos = (tmp==d_in[posTh+i*blockSize]) ? idx+i*blockSize : tmpPos;
			}
		}
	}
	unsigned int posOut = nIm*blockSize + idx;
	d_out[posOut] = tmp;
	position[posOut] = tmpPos;
	//if(nIm==0){
		//printf("posOut %i d_out[posOut] %f position[posOut] %f \n", posOut, d_out[posOut], position[posOut]);
	//}

}

void calculateMaxNew2DNew(int yxdim, int Ndim, float *d_data,
		GpuMultidimArrayAtGpu<float> &d_out, GpuMultidimArrayAtGpu<float> &d_pos){

    int numTh = 1024;
    int numBlk = Ndim;

    d_out.resize(numTh * numBlk);
    d_pos.resize(numTh * numBlk);

    calculateMaxThreads<<<numBlk, numTh>>>(d_data, d_out.d_data, d_pos.d_data, yxdim, Ndim);
    cudaDeviceSynchronize();

    calculateMax2<<<numBlk, numTh, 2*numTh * sizeof(float)>>> (d_out.d_data, d_out.d_data, d_pos.d_data, numTh, 1, false);
    cudaDeviceSynchronize();

}


void calculateMaxNew2D(int yxdim, int Ndim, float *d_data,
		GpuMultidimArrayAtGpu<float> &d_out, GpuMultidimArrayAtGpu<float> &d_pos){

    int numTh = 1024;
    int numBlk = yxdim/1024;
    if(yxdim%1024!=0)
    	numBlk++;
    numBlk=ceil((float)numBlk/2);
    int numBlk2, size_aux2;

    d_out.resize(numBlk*Ndim);
    d_pos.resize(numBlk*Ndim);

	calculateMax<<<numBlk, numTh, 2*numTh * sizeof(float)>>>(d_data, d_out.d_data, d_pos.d_data, yxdim, Ndim, true);

	numBlk2=numBlk;
	size_aux2=numBlk;
	while(1){
		if(numBlk2>numTh){
		   numBlk2=numBlk2/numTh;
		   if(numBlk2%numTh!=0)
			   numBlk2++;
		   numBlk=ceil((float)numBlk/2);
		}else{
			numTh=ceil((float)size_aux2/2);
			float aux1 = log((float)numTh)/log(2.0);
			int aux2 = (int)aux1;
			float error = aux1-(float)aux2;
			if(error>0.001)
				aux2++;
			numTh=pow(2,aux2);
			numBlk2=1;
		}
		calculateMax<<<numBlk2, numTh, 2*numTh * sizeof(float)>>> (d_out.d_data, d_out.d_data, d_pos.d_data, size_aux2, Ndim, false);
		size_aux2=numBlk2;
		if(numBlk2==1)
			break;
   }

	/*float *h_pos = new float[Ndim];
	cudaMemcpy(h_pos, d_pos.d_data, Ndim*sizeof(float), cudaMemcpyDeviceToHost);
	//cudaMemcpy(max_values, d_out.d_data, Ndim*sizeof(float), cudaMemcpyDeviceToHost);

	for(int i=0; i<Ndim; i++){

		int position = (int)h_pos[i];

		float posX_aux = (float)(position%Xdim);
		float posY_aux = (float)(position/Xdim);
		float Xdim2 = (float)(Xdim/2);
		float Ydim2 = (float)(Ydim/2);

		if(posX_aux>=Xdim2 && posY_aux>=Ydim2){
			posX[i] = Xdim-1-posX_aux;
			posY[i] = Ydim-1-posY_aux;
		}else if(posX_aux<Xdim2 && posY_aux>=Ydim2){
			posX[i] = -(posX_aux+1);
			posY[i] = Ydim-1-posY_aux;
		}else if(posX_aux<Xdim2 && posY_aux<Ydim2){
			posX[i] = -(posX_aux+1);
			posY[i] = -(posY_aux+1);
		}else if(posX_aux>=Xdim2 && posY_aux<Ydim2){
			posX[i] = Xdim-1-posX_aux;
			posY[i] = -(posY_aux+1);
		}

		//Fixing padding problem
		posX[i]+=fixPadding;
		posY[i]+=fixPadding;

	}*/

}


void cuda_calculate_correlation_rotation(GpuCorrelationAux &referenceAux, GpuCorrelationAux &experimentalAux, TransformMatrix<float> &transMat,
		float *max_vector, int maxShift, mycufftHandle &myhandlePadded, bool mirror, StructuresAux &myStructureAux)
{
	myStructureAux.RefExpFourierPolar.resize(referenceAux.d_projPolarFFT.Xdim, referenceAux.d_projPolarFFT.Ydim,
			referenceAux.d_projPolarFFT.Zdim, referenceAux.d_projPolarFFT.Ndim);

    int numTh = 1024;
    XmippDim3 blockSize(numTh, 1, 1), gridSize;
    referenceAux.d_projPolarFFT.calculateGridSizeVectorized(blockSize, gridSize);

    pointwiseMultiplicationComplexKernel<<< CONVERT2DIM3(gridSize), CONVERT2DIM3(blockSize) >>>
			((cufftComplex*)referenceAux.d_projPolarFFT.d_data, (cufftComplex*)experimentalAux.d_projPolarFFT.d_data,
					(cufftComplex*)myStructureAux.RefExpFourierPolar.d_data, referenceAux.d_projPolarFFT.nzyxdim,
					referenceAux.d_projPolarFFT.yxdim);

    myStructureAux.RefExpRealSpacePolar.resize(referenceAux.XdimPolar, referenceAux.YdimPolar, referenceAux.d_projPolarFFT.Zdim,
    		referenceAux.d_projPolarFFT.Ndim);
    myStructureAux.RefExpFourierPolar.ifft(myStructureAux.RefExpRealSpacePolar, myhandlePadded);

    XmippDim3 blockSize2(numTh, 1, 1), gridSize2;
    myStructureAux.RefExpRealSpacePolar.calculateGridSizeVectorized(blockSize2, gridSize2);

    myStructureAux.d_NCCPolar.resize(referenceAux.XdimPolar, referenceAux.YdimPolar, referenceAux.d_projPolarFFT.Zdim,
				referenceAux.d_projPolarFFT.Ndim);

	double maskFFTPolar = (referenceAux.XdimPolar*referenceAux.YdimPolar);
	calculateNccRotationKernel<<< CONVERT2DIM3(gridSize2), CONVERT2DIM3(blockSize2) >>>
			(myStructureAux.RefExpRealSpacePolar.d_data, (cufftComplex*)referenceAux.d_projPolarFFT.d_data, (cufftComplex*)experimentalAux.d_projPolarFFT.d_data,
					(cufftComplex*)referenceAux.d_projPolarSquaredFFT.d_data, (cufftComplex*)experimentalAux.d_projPolarSquaredFFT.d_data,
					maskFFTPolar, myStructureAux.d_NCCPolar.d_data, referenceAux.d_projPolarFFT.yxdim, myStructureAux.RefExpRealSpacePolar.nzyxdim,
					myStructureAux.RefExpRealSpacePolar.yxdim);

	//AJ sum along the radius
    numTh = 1024;
    int numBlk = (myStructureAux.d_NCCPolar.Xdim*myStructureAux.d_NCCPolar.Ndim)/numTh;
    if((myStructureAux.d_NCCPolar.Xdim*myStructureAux.d_NCCPolar.Ndim)%numTh!=0)
    	numBlk++;

    myStructureAux.d_NCCPolar1D.resize(myStructureAux.d_NCCPolar.Xdim,1,1,myStructureAux.d_NCCPolar.Ndim);
    myStructureAux.auxMax.resize(myStructureAux.d_NCCPolar.Xdim,1,1,myStructureAux.d_NCCPolar.Ndim);
    myStructureAux.auxZero.resize(myStructureAux.d_NCCPolar.Xdim,1,1,myStructureAux.d_NCCPolar.Ndim);
    sumRadiusKernel<<< numBlk, numTh >>>(myStructureAux.d_NCCPolar.d_data, myStructureAux.d_NCCPolar1D.d_data, myStructureAux.auxMax.d_data,
    		myStructureAux.auxZero.d_data, myStructureAux.d_NCCPolar.Xdim*myStructureAux.d_NCCPolar.Ndim, myStructureAux.d_NCCPolar.Ydim,
			myStructureAux.d_NCCPolar.Ndim);


	//float *max_values = new float[myStructureAux.d_NCCPolar.Ndim];
	//float *posX = new float[myStructureAux.d_NCCPolar.Ndim];
	//float *posY = new float[myStructureAux.d_NCCPolar.Ndim];

    //printf("myStructureAux.d_NCCPolar1D.Xdim %i \n", myStructureAux.d_NCCPolar1D.Xdim);
	//printf("myStructureAux.d_NCCPolar1D.Ydim %i \n", myStructureAux.d_NCCPolar1D.Ydim);
	//printf("myStructureAux.d_NCCPolar1D.Ndim %i \n", myStructureAux.d_NCCPolar1D.Ndim);

	//struct timeval begin, end;
	//gettimeofday(&begin, NULL);

	calculateMaxNew2DNew(myStructureAux.d_NCCPolar1D.Xdim, myStructureAux.d_NCCPolar1D.Ndim, myStructureAux.d_NCCPolar1D.d_data,
			myStructureAux.d_out_polar_max, myStructureAux.d_pos_polar_max);

	//gettimeofday(&end, NULL);
	//double elapsed = (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec)/1000000.0);
	//printf("Calculate Max 1D time %lf \n", elapsed);


    TransformMatrix<float> result(transMat.Ndim);
	TransformMatrix<float> newMat(transMat.Ndim);
	//newMat.setRotation(posX);

	numTh = 1024;
	numBlk = transMat.Ndim/numTh;
	if(transMat.Ndim%numTh > 0)
		numBlk++;

	GpuMultidimArrayAtGpu<float> maxGpu(myStructureAux.d_NCCPolar1D.Ndim);
	buildRotationMatrix<<<numBlk, numTh>>> (myStructureAux.d_pos_polar_max.d_data, newMat.d_data, transMat.d_data,
			result.d_data, maxGpu.d_data, myStructureAux.auxMax.d_data, myStructureAux.auxZero.d_data,
			myStructureAux.d_NCCPolar1D.Xdim, myStructureAux.d_NCCPolar1D.Ndim,
			myStructureAux.d_NCCPolar1D.yxdim, 0, 2*maxShift);

	/*numTh = 1024;
		numBlk = transMat.Ndim/numTh;
		if(transMat.Ndim%numTh > 0)
			numBlk++;

	GpuMultidimArrayAtGpu<float> maxGpu(myStructureAux.d_NCCPolar1D.Ndim);
	gpuErrchk(cudaMemcpy(maxGpu.d_data, max_values, myStructureAux.d_NCCPolar1D.Ndim*sizeof(float), cudaMemcpyHostToDevice));
	matrixMultiplication<<<numBlk, numTh>>> (newMat.d_data, transMat.d_data, result.d_data, transMat.Ndim, 2*maxShift,
			maxGpu.d_data, myStructureAux.auxZero.d_data, myStructureAux.d_NCCPolar1D.yxdim);*/
	result.copyMatrix(transMat);

	gpuErrchk(cudaMemcpyAsync(max_vector, maxGpu.d_data, myStructureAux.d_NCCPolar1D.Ndim*sizeof(float), cudaMemcpyDeviceToHost));

	//delete[] max_values;
	//delete[] posX;
	//delete[] posY;

}


void cuda_calculate_correlation(GpuCorrelationAux &referenceAux, GpuCorrelationAux &experimentalAux, TransformMatrix<float> &transMat,
		float *max_vector, int maxShift, mycufftHandle &myhandlePadded, bool mirror, StructuresAux &myStructureAux)
{

    myStructureAux.RefExpFourier.resize(referenceAux.d_projFFT.Xdim, referenceAux.d_projFFT.Ydim,
			referenceAux.d_projFFT.Zdim, referenceAux.d_projFFT.Ndim);

    int numTh = 1024;
    XmippDim3 blockSize(numTh, 1, 1), gridSize;
    referenceAux.d_projFFT.calculateGridSizeVectorized(blockSize, gridSize);

    pointwiseMultiplicationComplexKernel<<< CONVERT2DIM3(gridSize), CONVERT2DIM3(blockSize) >>>
			((cufftComplex*)referenceAux.d_projFFT.d_data, (cufftComplex*)experimentalAux.d_projFFT.d_data, (cufftComplex*)myStructureAux.RefExpFourier.d_data,
					referenceAux.d_projFFT.nzyxdim, referenceAux.d_projFFT.yxdim);


    myStructureAux.RefExpRealSpace.resize(referenceAux.Xdim, referenceAux.Ydim, referenceAux.d_projFFT.Zdim,
    		referenceAux.d_projFFT.Ndim);

    myStructureAux.RefExpFourier.ifft(myStructureAux.RefExpRealSpace, myhandlePadded);


 	XmippDim3 blockSize2(numTh, 1, 1), gridSize2;
 	myStructureAux.RefExpRealSpace.calculateGridSizeVectorized(blockSize2, gridSize2);

    myStructureAux.d_NCC.resize(referenceAux.Xdim, referenceAux.Ydim, referenceAux.d_projFFT.Zdim,
			referenceAux.d_projFFT.Ndim);

	calculateNccKernel<<< CONVERT2DIM3(gridSize2), CONVERT2DIM3(blockSize2) >>>
			(myStructureAux.RefExpRealSpace.d_data, referenceAux.MFrealSpace.d_data, experimentalAux.MFrealSpace.d_data, referenceAux.MF2realSpace.d_data,
					experimentalAux.MF2realSpace.d_data, referenceAux.maskAutocorrelation.d_data, myStructureAux.d_NCC.d_data, referenceAux.MFrealSpace.nzyxdim,
					referenceAux.MFrealSpace.yxdim, referenceAux.MFrealSpace.Xdim, referenceAux.MFrealSpace.Ydim, referenceAux.maskCount, maxShift);

	int fixPadding=0;
	if(referenceAux.XdimOrig%2==0 && referenceAux.Xdim%2==0)
		fixPadding=1;
	if(referenceAux.XdimOrig%2==0 && referenceAux.Xdim%2!=0)
		fixPadding=0;
	if(referenceAux.XdimOrig%2!=0 && referenceAux.Xdim%2==0)
		fixPadding=-1;
	if(referenceAux.XdimOrig%2!=0 && referenceAux.Xdim%2!=0)
		fixPadding=0;

    //AJ new maximum calculation
	//float *max_values = new float[myStructureAux.d_NCC.Ndim];
	//float *posX = new float[myStructureAux.d_NCC.Ndim];
	//float *posY = new float[myStructureAux.d_NCC.Ndim];

	/*float *mio = new float [2000*2];
	for (int i=0; i<2000*2; i++)
		mio[i]=i;

	gpuErrchk(cudaMemcpy(myStructureAux.d_NCC.d_data, mio, 2048*2*sizeof(float), cudaMemcpyHostToDevice));
*/


	//printf("myStructureAux.d_NCC.Xdim %i \n", myStructureAux.d_NCC.Xdim);
	//printf("myStructureAux.d_NCC.Ydim %i \n", myStructureAux.d_NCC.Ydim);
	//printf("myStructureAux.d_NCC.Ndim %i \n", myStructureAux.d_NCC.Ndim);

	//struct timeval begin, end;
	//gettimeofday(&begin, NULL);

	calculateMaxNew2DNew(myStructureAux.d_NCC.yxdim, myStructureAux.d_NCC.Ndim,
			myStructureAux.d_NCC.d_data, myStructureAux.d_out_max, myStructureAux.d_pos_max);

	//gettimeofday(&end, NULL);
	//double elapsed = (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec)/1000000.0);
	//printf("Calculate Max 2D time %lf \n", elapsed);



	TransformMatrix<float> result(transMat.Ndim);
	TransformMatrix<float> newMat(transMat.Ndim);
	/*newMat.setTranslation(posX, posY, myStructureAux.d_out_max.d_data);

	numTh = 1024;
	int numBlk = transMat.Ndim/numTh;
	if(transMat.Ndim%numTh > 0)
		numBlk++;

	//GpuMultidimArrayAtGpu<float> maxGpu(myStructureAux.d_NCC.Ndim);
	//gpuErrchk(cudaMemcpy(maxGpu.d_data, max_values, myStructureAux.d_NCC.Ndim*sizeof(float), cudaMemcpyHostToDevice));
	matrixMultiplication<<<numBlk, numTh>>> (newMat.d_data, transMat.d_data, result.d_data, transMat.Ndim, 2*maxShift,
			myStructureAux.d_out_max.d_data, myStructureAux.d_NCC.d_data, myStructureAux.d_NCC.yxdim);
	result.copyMatrix(transMat);*/

	numTh = 1024;
		int numBlk = transMat.Ndim/numTh;
		if(transMat.Ndim%numTh > 0)
			numBlk++;

	buildTranslationMatrix<<<numBlk, numTh>>> (myStructureAux.d_pos_max.d_data, newMat.d_data, transMat.d_data, result.d_data,
			myStructureAux.d_out_max.d_data, myStructureAux.d_NCC.d_data, myStructureAux.d_NCC.Xdim, myStructureAux.d_NCC.Ydim,
			myStructureAux.d_NCC.Ndim, myStructureAux.d_NCC.yxdim, fixPadding, 2*maxShift);
	result.copyMatrix(transMat);

	gpuErrchk(cudaMemcpyAsync(max_vector, myStructureAux.d_out_max.d_data, myStructureAux.d_NCC.Ndim*sizeof(float), cudaMemcpyDeviceToHost));

	//delete[] max_values;
	//delete[] posX;
	//delete[] posY;

}

void apply_transform(GpuMultidimArrayAtGpu<float> &d_original_image, GpuMultidimArrayAtGpu<float> &d_transform_image, TransformMatrix<float> &transMat){

	int numTh = 1024;
	XmippDim3 blockSize(numTh, 1, 1), gridSize;
	d_original_image.calculateGridSizeVectorized(blockSize, gridSize);

	applyTransformKernel<<< CONVERT2DIM3(gridSize), CONVERT2DIM3(blockSize) >>>
			(d_original_image.d_data, d_transform_image.d_data, transMat.d_data,
					d_original_image.nzyxdim, d_original_image.yxdim, d_original_image.Xdim, d_original_image.Ydim);

}


__global__ void cart2polar(float *image, float *polar, float *polar2, int maxRadius, int maxAng,
		int Nimgs, int Ydim, int Xdim, bool rotate)
{
	int angle = blockDim.x * blockIdx.x + threadIdx.x;
	int radius = blockDim.y * blockIdx.y + threadIdx.y;

	if (radius>=maxRadius || angle>=maxAng)
		return;

	float x = (float)(radius*cosf((float)(angle*PI/180))) + Xdim/2;
	float y = (float)(radius*sinf((float)(angle*PI/180))) + Ydim/2;

	float dx_low = floor(x);
	float dy_low = floor(y);
	int x_low = (int)dx_low;
	int y_low = (int)dy_low;
	float x_x_low=x-dx_low;
	float y_y_low=y-dy_low;
	float one_x=1.0-x_x_low;
	float one_y=1.0-y_y_low;
	float w00=one_y*one_x;
	float w01=one_y*x_x_low;
	float w10=y_y_low*one_x;
	float w11=y_y_low*x_x_low;

	int NXY=Xdim*Ydim;
	int NXYpolar=maxAng*maxRadius;
	int imgIdx00=y_low * Xdim + x_low;
	int imgIdx01=imgIdx00+1;
	int imgIdx10=imgIdx00+Xdim;
	int imgIdx11=imgIdx10+1;
	int imgOffset=0;
	int polarOffset=0;
	int polarIdx;
	if(!rotate)
		polarIdx=angle+(radius*maxAng);
	else
		polarIdx = (maxAng-angle-1)+((maxRadius-radius-1)*maxAng);

	for (int n=0; n<Nimgs; n++)
	{
		float I00 = image[imgIdx00+imgOffset];
		float I01 = image[imgIdx01+imgOffset];
		float I10 = image[imgIdx10+imgOffset];
		float I11 = image[imgIdx11+imgOffset];
		float imVal = I00*w00 + I01*w01 + I10*w10 + I11*w11;
		int finalPolarIndex=polarIdx+polarOffset;
		polar[finalPolarIndex] = imVal;
		polar2[finalPolarIndex] = imVal*imVal;

		imgOffset+=NXY;
		polarOffset+=NXYpolar;
	}

}

void cuda_cart2polar(GpuMultidimArrayAtGpu<float> &image, GpuMultidimArrayAtGpu<float> &polar_image, GpuMultidimArrayAtGpu<float> &polar2_image, bool rotate)
{
    int numTh = 32;
    XmippDim3 blockSize(numTh, numTh, 1), gridSize;
    polar_image.calculateGridSize(blockSize, gridSize);
    cart2polar <<< CONVERT2DIM3(gridSize), CONVERT2DIM3(blockSize) >>>
    		(image.d_data, polar_image.d_data, polar2_image.d_data, polar_image.Ydim, polar_image.Xdim, polar_image.Ndim, image.Ydim, image.Xdim, rotate);
}

