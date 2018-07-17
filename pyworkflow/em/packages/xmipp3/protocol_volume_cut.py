# **************************************************************************
# *
# * Authors:     Javier Mota Garcia (jmota@cnb.csic.es)
# *
# * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# * 02111-1307  USA
# *
# *  All comments concerning this program package may be sent to the
# *  e-mail address 'scipion@cnb.csic.es'
# *
# **************************************************************************

from os.path import join, exists, getmtime
from datetime import datetime

from pyworkflow.em.protocol import EMProtocol
import pyworkflow.protocol.params as params
from pyworkflow.em.data import PdbFile, Volume

import time
from shutil import  copyfile

class ProtVolumeCut(EMProtocol):

    _label = 'volumecut'

    def _defineParams(self, form):
        form.addSection(label='Input')
        form.addParam('pdbReference', params.PointerParam, pointerClass='PdbFile',
                      label='PDB reference', important=True)
        form.addParam('Volume', params.PointerParam, pointerClass='Volume',
                      label='Volume to cut', important=True)
        form.addParam('radius', params.FloatParam, default=0.0,
                      label='Radius', help='radius from the center of the region to cut')
        form.addParam('helice', params.StringParam, label='Cut off the '
                                                          'whole chain or helices ' 
                                                          'with specific AA? (C or H)',
                      important=True)
        form.addParam('chainID', params.StringParam, label= 'ChainID',
                      important=True,
                      help='chain which you are interested on')
        form.addParam('apixX', params.FloatParam, default=1.0,
                      label='ApixX',
                      help='spacing distance along X coordinate between two '
                           'neighboring voxels')
        form.addParam('apixY', params.FloatParam, default=1.0,
                      label='ApixY',
                      help='spacing distance along Y coordinate between two '
                           'neighboring voxels')
        form.addParam('apixZ', params.FloatParam, default=1.0,
                      label='ApixZ',
                      help='spacing distance along Z coordinate between two '
                           'neighboring voxels')


    def _insertAllSteps(self):
        self._insertFunctionStep('_cutVolume')
        self._insertFunctionStep('_createOutputStep')


    def _cutVolume(self):
        pathPdb = self.pdbReference.get().getFileName()
        pathVol = self.Volume.get().getFileName()

        copyfile(pathPdb, self._getExtraPath('protein.pdb'))
        self._args = "-i %s -o %s" %(pathVol, self._getExtraPath('protein.mrc'))
        self.runJob('xmipp_image_convert', self._args)

        pathPdb = self._getExtraPath('protein.pdb').split('.')
        self._params = {'pdb':pathPdb[0],
                        'chainID': self.chainID.get(),
                        'helice': self.helice.get(),
                        'radius': self.radius.get(),
                        'apixX': self.apixX.get(),
                        'apixY': self.apixY.get(),
                        'apixZ':self.apixZ.get()
                       }

        self._args = """ << eof
        %(pdb)s
        %(chainID)s
        %(helice)s
        %(radius)f
        %(apixX)f
        %(apixY)f
        %(apixZ)f"""

        self.runJob('/home/javiermota/scipion2/scipion/software/em/VolumeCut/LinuxCompiled/VolumeCut',
                    self._args % self._params)

    def _createOutputStep(self):

        samplingRate = self.Volume.get().getSamplingRate()
        volume = Volume()
        volume.setFileName(self._getExtraPath(
            "protein_%s_cut")%self.chainID.get())
        volume.setSamplingRate(samplingRate)
        pdb = PdbFile(self._getPath('protein_%s_cut.pdb')%(self.chainID.get()),
                      pdb=True)
        pdb.setVolume(volume)
        self._defineOutputs(outputPdb=pdb)
        self._defineSourceRelation(self.pdbReference, pdb)





