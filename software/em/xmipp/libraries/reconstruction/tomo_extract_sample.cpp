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


#include <data/xmipp_image.h>
#include "tomo_extract_sample.h"
#include <data/transformations.h>
#include <fstream>

using namespace std;

void ProgTomoExtractSample::defineParams()
{
	addParamsLine("  [--i1]        		     : reference volume");
	addParamsLine("  [--i2]        		     : volume to align");
	addParamsLine("  [--matrixAlign]            : doc file with matrix transformation parameters");

}

void ProgTomoExtractSample::readParams()
{

	// Get parameters =======================================================
	fn1 = getParam("--i1");
	fn2 = getParam("--i2");
	fnGeo = getParam("--matrixAlign");


}

void ProgTomoExtractSample::run()
{
	V1.read(fn1);
	V1().setXmippOrigin();
	V2.read(fn2);
	V2().setXmippOrigin();

	ifstream file;
	string line, value;
	file.open(fn1.c_str());

	 while (!file.eof())
	    {
	        // Read an ATOM line
	        getline(file, line);
	        if (line == "")
	        {
	            continue;
	        }
	        value = line.substr(0,9);
	        std::cout << value << std::endl;

	    }

	    // Close files
	    file.close();

	//applyGeometry(LINEAR, V1, V2, MatrixAlign, IS_NOT_INV, DONT_WRAP);

}
