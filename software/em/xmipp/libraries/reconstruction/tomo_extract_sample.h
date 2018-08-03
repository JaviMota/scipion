/***************************************************************************
 *
 * Authors:     Javier Mota Garcia (jmota@cnb.csic.es)
 *
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

#ifndef SOFTWARE_EM_XMIPP_LIBRARIES_RECONSTRUCTION_TOMO_EXTRACT_SAMPLE_H_
#define SOFTWARE_EM_XMIPP_LIBRARIES_RECONSTRUCTION_TOMO_EXTRACT_SAMPLE_H_

#include "fourier_filter.h"
#include <data/geometry.h>

class ProgTomoExtractSample : public XmippProgram
{

public:
	FileName fn1, fn2;
	FileName fnGeo;
	Image<double> V1;
	Image<double> V2;
	Image<double> Vout;
	MultidimArray<double> MatrixAlign;

	ProgTomoExtractSample()
	{
	}

public:
    /// Read parameters
    void readParams(XmippProgram *program);

    /// Define Parameters
    void defineParams(XmippProgram *program);

    /// Run program
    void run();
};



#endif /* SOFTWARE_EM_XMIPP_LIBRARIES_RECONSTRUCTION_TOMO_EXTRACT_SAMPLE_H_ */
