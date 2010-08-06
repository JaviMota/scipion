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

#ifndef RWSPE_H_
#define RWSPE_H_

// Princeton Instruments CCD camera

// I/O prototypes
int readSPE(int img_select,bool isStack=false)
{
#undef DEBUG
    //#define DEBUG
#ifdef DEBUG
    printf("DEBUG readSPE: Reading SPE file\n");
#endif

    int _xDim,_yDim,_zDim, __depth;
    unsigned long int _nDim;
    bool __is_signed;

    FileName fn_inf;

    fn_inf = filename.add_extension("inf");
    FILE *fh_inf = fopen(fn_inf.c_str(), "r");
    if (!fh_inf)
        REPORT_ERROR(1, (std::string)"Micrograph::open_micrograph: Cannot find " +
                     fn_inf);
    _xDim = textToInteger(getParameter(fh_inf, "Xdim"));
    _yDim = textToInteger(getParameter(fh_inf, "Ydim"));
    __depth = textToInteger(getParameter(fh_inf, "bitspersample"));
    if (checkParameter(fh_inf, "offset"))
        offset = textToInteger(getParameter(fh_inf, "offset"));
    else
        offset = 0;
    if (checkParameter(fh_inf, "is_signed"))
        __is_signed = (getParameter(fh_inf, "is_signed") == "true" ||
                       getParameter(fh_inf, "is_signed") == "TRUE");
    else
        __is_signed = false;
    if (checkParameter(fh_inf, "endianess"))
        if(getParameter(fh_inf, "endianess") == "big" || getParameter(fh_inf, "endianess") == "BIG")
            swap = true;
        else
            swap = false;

    if (IsBigEndian())
        swap = !swap;

    fclose(fh_inf);

    _zDim = (int) 1;
    _nDim = (int) 1;

    // Map the parameters
    data.setDimensions(_xDim, _yDim, _zDim, _nDim);

    unsigned long   imgStart=0;
    unsigned long   imgEnd =_nDim;
    if (img_select != -1)
    {
        imgStart=img_select;
        imgEnd=img_select+1;
    }

    DataType datatype;

    switch ( __depth )
    {
    case 8:
        datatype = UChar;
        break;
    case 16:
        if (__is_signed)
            datatype = Short;
        else
            datatype = UShort;
        break;
    case 32:
        datatype = Float;
        break;
    default:
        REPORT_ERROR(1, "Micrograph::open_micrograph: depth is not 8, 16 nor 32");
    }

    MDMainHeader.removeObjects();
    MDMainHeader.setColumnFormat(false);
    MDMainHeader.addObject();
    MDMainHeader.setValue(MDL_SAMPLINGRATEX,(double) -1);
    MDMainHeader.setValue(MDL_SAMPLINGRATEY,(double) -1);
    MDMainHeader.setValue(MDL_DATATYPE,(int)datatype);

    if( dataflag == -2 )
    {
        //        fclose(fimg);
        return 0;
    }

    MD.removeObjects();
    for ( i=imgStart; i<imgEnd; i++ )
        //for(int i=0;i< Ndim;i++)
    {
        MD.addObject();
        MD.setValue(MDL_ORIGINX, zeroD);
        MD.setValue(MDL_ORIGINY, zeroD);
        MD.setValue(MDL_ORIGINZ,  zeroD);
        MD.setValue(MDL_ANGLEROT, zeroD);
        MD.setValue(MDL_ANGLETILT,zeroD);
        MD.setValue(MDL_ANGLEPSI, zeroD);
        MD.setValue(MDL_WEIGHT,   oneD);
        MD.setValue(MDL_FLIP,     falseb);
    }

    //#define DEBUG
#ifdef DEBUG

    MDMainHeader.write(std::cerr);
    MD.write(std::cerr);
#endif

    FILE        *fimg;
    if ( ( fimg = fopen(filename.c_str(), "r") ) == NULL )
        return(-1);

    size_t pad = 0;

    readData(fimg, img_select, datatype, pad);

    if ( !mmapOn )
        fclose(fimg);

    return(0);
}

int writeSPE(int img_select, bool isStack=false, int mode=WRITE_OVERWRITE)
{
    //#define DEBUG
#ifdef DEBUG
    printf("DEBUG writeSPE: Writing SPE file\n");
    printf("DEBUG writeSPE: File %s\n", filename.c_str());
#endif
#undef DEBUG

    int Xdim = XSIZE(data);
    int Ydim = YSIZE(data);
    int Zdim = ZSIZE(data);
    int Ndim = NSIZE(data);

    int _depth;
    bool _is_signed;

    // Volumes and stacks are not supported
    if (Zdim > 1 || Ndim > 1)
        REPORT_ERROR(1000, "ERROR: rwSPE does not support neither volumes nor stacks.");


    DataType wDType;

    if (typeid(T)==typeid(double)||typeid(T)==typeid(float))
    {
        wDType = Float;
        _is_signed = true;
    }
    else if (typeid(T)==typeid(int))
    {
        wDType = Int;
        _is_signed = true;
    }
    else if (typeid(T)==typeid(unsigned int))
    {
        wDType = UInt;
        _is_signed = false;

    }
    else if (typeid(T)==typeid(short))
    {
        wDType = Short;
        _is_signed = true;
    }
    else if (typeid(T)==typeid(unsigned short))
    {
        wDType = UShort;
        _is_signed = false;
    }
    else if (typeid(T)==typeid(char))
    {
        wDType = SChar;
        _is_signed = true;
    }
    else if (typeid(T)==typeid(unsigned char))
    {
        wDType = UChar;
        _is_signed = false;
    }
    else
        REPORT_ERROR(1000,(std::string)"ERROR: rwSPE does not write from " + typeid(T).name() + "type.");


_depth = gettypesize(wDType);

/* Write INF file ==================================*/

FileName fn_inf;

fn_inf = filename.add_extension("inf");
FILE *fh_inf = fopen(fn_inf.c_str(), "w");
if (!fh_inf)
    REPORT_ERROR(1, (std::string)"rwSPE::write: Error opening file " + fn_inf);

fprintf(fh_inf,"# Bits per sample\n");
fprintf(fh_inf,"bitspersample= %d\n",_depth*8);
fprintf(fh_inf,"# Samples per pixel\n");
fprintf(fh_inf,"samplesperpixel= 1\n");
fprintf(fh_inf,"# Image width\n");
fprintf(fh_inf,"Xdim= %d\n", Xdim);
fprintf(fh_inf,"# Image length\n");
fprintf(fh_inf,"Ydim= %d\n",Ydim);
fprintf(fh_inf,"# offset in bytes (zero by default)\n");
fprintf(fh_inf,"offset= 0\n");
fprintf(fh_inf,"# Is a signed or Unsigned int (by default true)\n");
if (_is_signed)
    fprintf(fh_inf,"is_signed = true\n");
else
    fprintf(fh_inf,"is_signed = false\n");
fprintf(fh_inf,"# Byte order\n");
if (IsBigEndian())
    fprintf(fh_inf,"endianess = big\n");
else
    fprintf(fh_inf,"endianess = little\n");

if (fclose(fh_inf)!=0)
    REPORT_ERROR(6001, "rwSPE::write: Error creating output info file.");


/* Write Image file ==================================*/

FILE  *fimg;
if ( ( fimg = fopen(filename.c_str(), "w") ) == NULL )
    REPORT_ERROR(1,(std::string)"Cannot create file " + filename);

size_t datasize, datasize_n;
datasize_n = Xdim*Ydim*Zdim;

writePageAsDatatype(fimg, wDType, datasize_n);


if( fclose(fimg) !=0 )
    REPORT_ERROR(1,(std::string)"Can not close file "+ filename);

return(0);


//    REPORT_ERROR(6001, "ERROR: writeSPE is not implemented.");
//    return(-1);
}


#endif /* RWSPE_H_ */
