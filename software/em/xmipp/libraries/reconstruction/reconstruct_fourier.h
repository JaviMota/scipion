/***************************************************************************
 *
 * Authors:     Roberto Marabini (roberto@cnb.csic.es)
 *              Carlos Oscar S. Sorzano (coss@cnb.csic.es)
 *              Jose Roman Bilbao-Castro (jrbcast@ace.ual.es)
 *              Vahid Abrishami (vabrishami@cnb.csic.es)
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

#ifndef __RECONSTRUCT_FOURIER_H
#define __RECONSTRUCT_FOURIER_H

#include <iostream>
#include <data/xmipp_fftw.h>
#include <data/xmipp_funcs.h>
#include <data/xmipp_image.h>
#include <data/projection.h>
#include <data/xmipp_threads.h>
#include <data/blobs.h>
#include <data/metadata.h>
#include <data/ctf.h>

#include <data/args.h>
#include <data/xmipp_fft.h>
#include <sys/time.h>

#include <data/metadata.h>
#include "recons.h"

#include <reconstruction/directions.h>
#include <reconstruction/symmetrize.h>
#define BLOB_TABLE_SIZE 5000
#define BLOB_TABLE_SIZE_SQRT 10000

#define MINIMUMWEIGHT 0.001
#define ACCURACY 0.001

#define EXIT_THREAD 0
#define PROCESS_IMAGE 1
#define PROCESS_WEIGHTS 2
#define PRELOAD_IMAGE 3

/**@defgroup FourierReconstruction Fourier reconstruction
   @ingroup ReconsLibrary */
//@{
class ProgRecFourier;

// static pthread_mutex_t mutexDocFile= PTHREAD_MUTEX_INITIALIZER;

struct imgData
{
	std::complex<float>** img;
	CTFDescription ctf;
	int imgIndex;
	double weight;
	Matrix2D<double> localAInv;
	bool skip;
};

struct ImageThreadParams
{
    int myThreadID;
    ProgRecFourier * parent;
    int startImageIndex;
    int endImageIndex;
    bool reprocessFlag;
    MetaData * selFile;
    imgData* dataA = NULL;
    imgData* dataB = NULL;
};

/** Fourier reconstruction parameters. */
class ProgRecFourier : public ProgReconsBase
{
public:
    /** Filenames */
    FileName fn_out, fn_sym, fn_sel, fn_doc, fn_fsc;

    /** SelFile containing all projections */
    MetaData SF;

    /** Flag whether to use the weights in the image metadata */
    bool do_weights;

    /** Use CTF */
    bool useCTF;

    /** Phase flipped.
     * True if the images have been phase flipped before entering.
     */
    bool phaseFlipped;

    /** Minimum CTF value to invert */
    double minCTF;

    /** Sampling rate */
    double Ts;

    /** Projection padding Factor */
    double padding_factor_proj;

    /** Volume padding Factor */
    double padding_factor_vol;

    /** Sampling rate in Angstroms/pixel */
    double sampling_rate;

    /// Max resolution in Angstroms
    double maxResolution;

    /// Number of iterations for the weight
    int NiterWeight;

    /// Number of threads to use in parallel to process a single image
    int numThreads;

    /// IDs for the threads
    pthread_t * th_ids;

    /// Contains parameters passed to each thread
    ImageThreadParams * th_args;

    /// Tells the threads what to do next
    int threadOpCode;

    /// Number of rows already processed on an image
    size_t rowsProcessed;

    /// Defines what a thread should do
    static void * processImageThread( void * threadArgs );

    /// Controls mutual exclusion on critical zones of code
    pthread_mutex_t workLoadMutex;

    /// To create a barrier synchronization for threads
    barrier_t barrier;

    /// A status array for each row in an image (processing, processed,etc..)
    int * statusArray;

    /// How many image rows are processed at a time by a single thread.
    int thrWidth;

public: // Internal members
    // Size of the original images
    int imgSize;

    // Table with blob values, lineal sampling
    Matrix1D<double>  Fourier_blob_table;

    // Table with blob values, squared samplinf
    Matrix1D<double> blobTableSqrt;

    // Inverse of the delta and deltaFourier used in the tables
    //double iDelta,
    double iDeltaFourier, iDeltaSqrt;

    // Maximum interesting resolution squared
    double maxResolution2;

    // Definition of the blob
    struct blobtype blob;

    // vector with R symmetry matrices
    std::vector <Matrix2D<double> > R_repository;

    // Fourier transformer for the volume
    FourierTransformer transformerVol;

    // An alias to the Fourier transform in transformerVol and also temporary to keep the weights
    MultidimArray< std::complex<double> > VoutFourier;

    // Volume of Fourier weights
    MultidimArray<double> FourierWeights;

    int paddedImgSize;

    // Output volume
    Image<double> Vout;

public:
    /// Read arguments from command line
    void readParams();

    /// Read arguments from command line
    void defineParams();

    /** Show. */
    void show();

    /** Run. */
    void run();

    /// Produce side info: fill arrays with relevant transformation matrices
    void produceSideinfo();

    void finishComputations( const FileName &out_name );

    /// Process one image
    void processImages( int firstImageIndex, int lastImageIndex, bool saveFSC=false, bool reprocessFlag=false);

    /// Method for the correction of the fourier coefficients
    void correctWeight();
	
	/// Force the weights to be symmetrized
    void forceWeightSymmetry(MultidimArray<double> &FourierWeights);

    ///Functions of common reconstruction interface
    virtual void setIO(const FileName &fn_in, const FileName &fn_out);

private:

    int availableMemory;
    static const int batchSize = 11;
    template<typename T>
    static T** allocate(T**& where, int xSize, int ySize);

    void loadImages(int startIndex, int endIndex, bool reprocess);
    void swapBuffers();
    void processBuffer(imgData* buffer,
    		std::complex<float>*** outputVolume,
    		float*** outputWeight,
    		bool saveFSC,
    		int FSCIndex);

    template<typename T>
    static T*** allocate(T***& where, int xSize, int ySize, int zSize);

    static std::complex<float>** clipAndShift(MultidimArray<std::complex<double> >& paddedFourier,
    		ProgRecFourier * parent);

    template<typename T>
    void crop(T***& inOut, int size);

    template<typename T>
    T*** applyBlob(T***& input, int size, float blobSize,
    		Matrix1D<double>& blobTableSqrt, float iDeltaSqrt);

    inline void allocateVoutFourier() {
    	if (NULL == VoutFourier.data) {
    		VoutFourier.initZeros(paddedImgSize, paddedImgSize, paddedImgSize/2 +1);
    	}
    }

    inline void allocateFourierWeights() {
    	if (NULL == FourierWeights.data) {
    		FourierWeights.initZeros(paddedImgSize, paddedImgSize, paddedImgSize/2 +1);
    	}
    }

    void saveIntermediateFSC(std::complex<float>*** outputVolume, float*** outputWeight, int volSize,
    		const std::string &weightsFileName, const std::string &fourierFileName, const FileName &resultFileName,
			bool storeResult);
    void saveFinalFSC(std::complex<float>*** outputVolume, float*** outputWeight, int volSize);

	static void processCube(
			int i,
			int j,
			const Matrix1D<int>& corner1,
			const Matrix1D<int>& corner2,
			const MultidimArray<float>& z2precalculated,
			const MultidimArray<int>& zWrapped,
			const MultidimArray<int>& zNegWrapped,
			const MultidimArray<float>& y2precalculated,
			float blobRadiusSquared,
			const MultidimArray<int>& yWrapped,
			const MultidimArray<int>& yNegWrapped,
			const MultidimArray<float>& x2precalculated,
			float iDeltaSqrt,
			float wModulator,
			const MultidimArray<int>& xWrapped,
			int xsize_1,
			const MultidimArray<int>& xNegWrapped,
			bool reprocessFlag,
			float wCTF,
			MultidimArray<std::complex<double> >& VoutFourier,
			Matrix1D<double>& blobTableSqrt,
			ImageThreadParams* threadParams,
			MultidimArray<double>& fourierWeights,
			double* ptrIn,
			float weight,
			ProgRecFourier * parent,
			Matrix1D<double>& real_position);
};
//@}
#endif
