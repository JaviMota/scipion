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

import sys
from os.path import join, exists, getmtime
from datetime import datetime

from pyworkflow.object import Set, String
import pyworkflow.em as em
from pyworkflow.em.protocol import EMProtocol
import pyworkflow.em.metadata as md
import pyworkflow.protocol.params as params
import pyworkflow.protocol.constants as pwconst
import pyworkflow.utils as pwutils
from pyworkflow.em.protocol import ProtImportFiles

from pyworkflow.em.packages.xmipp3.utils import isMdEmpty
from pyworkflow.em.packages.xmipp3.convert import mdToCTFModel, readCTFModel

class ProtVolumeCut(EMProtocol):


    def _defineProcessParams(self, form):

        form.addParam('pdbReference', params.PointerParam, pointerClass='SetOfPDBs')
        form.addParam('Volume', params.PointerParam, pointerClass='SetOfVolumes',
                      label='Volume to cut')
        form.addParam('radius', params.IntParam, default=0.0,
                      label='radius to cut')
        form.addParam('chainID', params.StringParam, label='chain which you are interested on')

    def _insertAllSteps(self):
        self._insertFunctionStep('_cutVolume')


    def _cutVolume(self):

        self._params = {'pdb':self.pdbReference.get(),
                        'volume': self.Volume.get()
                       }


