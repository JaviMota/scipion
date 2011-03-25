/***************************************************************************
 * Authors:     Joaquin Oton (joton@cnb.csic.es)
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

#include <data/program.h>
#include <data/transformations.h>

class ProgResliceVol : public XmippProgram
{
private:

    FileName fnImgIn, fnImgOut;
    ImageGeneric imgIn;   // Input Volume
    ImageGeneric imgOut;  // Output Volume
    String      face;

    void defineParams()
    {
        addUsageLine("Reslice a volume aligning any X or Y direction with Z axis.");
        addUsageLine("The output volume also has the dimensions rotated, so all the information in the volume is kept.");
        addSeeAlsoLine("transform_geometry");

        addParamsLine("  -i <input_file>    : Input volume file");
        addParamsLine("  [-o <output_file>] : Output volume file");
        addParamsLine("  --face <new_front_face>  : Face to be brought to front.");
        addParamsLine("     where <new_front_face>");
        addParamsLine("                 top    : Align -Y axis to Z axis, rotating 90 degrees around X axis");
        addParamsLine("                 left   : Align -X axis to Z axis, rotating -90 degrees around Y axis");
        addParamsLine("                 bottom : Align Y axis to Z axis, rotating -90 degrees around X axis");
        addParamsLine("                 right  : Align X axis to Z axis, rotating 90 degrees around Y axis");

        addExampleLine("Reslice a volume showing left face as new front face:", false);
        addExampleLine("xmipp_volume_reslice -i original.vol -o resliced.vol --face left ");
    }

    void readParams()
    {
        fnImgIn = getParam("-i");
        fnImgOut = (checkParam("-o"))? getParam("-o") : fnImgIn;
        face = getParam("--face");
    }

    void show()
    {
        std::cout
        << "Input file     : " << fnImgIn   << std::endl
        << "Output file    : " << fnImgOut  << std::endl
        << "New front face : " << face      << std::endl
        ;
    }

    void run()
    {
        show();

        imgIn.readMapped(fnImgIn);

        int xDim, yDim, zDim, XdimOut, yDimOut, zDimOut;
        imgIn.getDimensions(xDim, yDim, zDim);

        char axis;
        bool reverse;

        if (face == "top" || face == "bottom")
        {
            axis = 'Y';
            XdimOut = xDim;
            yDimOut = zDim;
            zDimOut = yDim;
            reverse = (face == "top");
        }
        else if (face == "left" || face == "right")
        {
            axis = 'X';
            XdimOut = zDim;
            yDimOut = yDim;
            zDimOut = xDim;
            reverse = (face == "left");
        }

        // Create output file
        imgOut.setDatatype(imgIn.getDatatype());
        imgOut.mapFile2Write(XdimOut, yDimOut, zDimOut, fnImgOut, fnImgIn == fnImgOut);

        MultidimArrayGeneric imTemp;
        int index;

        init_progress_bar(zDim);

        for (int k = 0; k < zDim; k++)
        {
            imTemp.aliasSlice(imgOut(), k);
            index = k + (zDim - 1 - 2*k) * (int)reverse;
            imgIn().getSlice(index, &imTemp, axis, !reverse);

            progress_bar(k+1);
        }
        imgOut.write(fnImgOut);
    }
};

int main(int argc, char **argv)
{
    ProgResliceVol program;
    program.read(argc, argv);
    return program.tryRun();
}
