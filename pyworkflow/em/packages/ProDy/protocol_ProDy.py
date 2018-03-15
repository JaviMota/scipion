# **************************************************************************
# *
# * Authors:  Javier Mota Garcia (jmota@cnb.csic.es), February 2018
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

import math
import numpy as np
import sys

from pyworkflow.em import *
from prody import *
from pyworkflow.utils import *
from pyworkflow.protocol.constants import LEVEL_ADVANCED
from pyworkflow.utils.path import createLink
import commands
import matplotlib.pyplot as plt
from shutil import copyfile
import time

FILE = 0
PDB = 1

class ProdyProt(EMProtocol):

    """ Protocol to execute functions from ProDy software"""

    _label = "ProDy protocol"

    def _defineParams(self, form):

        form.addSection(label = "Prody NMA analysis and molecular dynamics")
        form.addParam('FilePdb', params.EnumParam, choices=['File','Pdb'],
                      default=0, important=True,
                      display=params.EnumParam.DISPLAY_HLIST,
                      label="Input file or pdb")
        form.addParam('inputStructure', PathParam, label="Input structure",
                      important=True,
                      condition='FilePdb == %s' % FILE,
                      help='The input structure can be an atomic model '
                           '(true PDB) or a pseudoatomic model(an EM volume '
                           'converted into pseudoatoms)')
        form.addParam('Pdb', PointerParam, pointerClass='PdbFile',
                      label='Input Pdb', important=True,
                      condition='FilePdb == %s' % PDB)
        form.addParam('initTrajectory', PathParam,
                     label="Initial Trajectory",
                      important=True)
        form.addParam('finTrajectory', PathParam, label="Final Trajectory",
                      important=True)

    def _insertAllSteps(self):
        self._insertFunctionStep('prodyWrapper')

    def prodyWrapper(self):
        # '''createLink(self.inputStructure.get(), 'inputPdb.pdb')
        # createLink(self.initTrajectory.get(), 'initTraj.dcd')
        # createLink(self.finTrajectory.get(), 'finTraj.dcd')'''
        #
        file = open(self._getExtraPath("paths.txt"), "w")
        if self.Pdb.get() == None:
            file.write(self.inputStructure.get() + '\n')
            self.inputPdb = self.inputStructure.get()
        else:
            file.write(self.inputPdb.get().getFileName() + '\n')
            self.inputPdb = self.Pdb.get().getFileName()
        #file.write(self.initTrajectory.get() + '\n')
        #file.write(self.finTrajectory.get() + '\n')
        file.close()

        if self.FilePdb == FILE:
            print('1')
            self.inputPdb = self.inputStructure.get()
        else:
            print('2')
            self.inputPdb = self.Pdb.get().getFileName()

        print self.inputStructure.get()
        sys.stdout.flush()
        print self.inputPdb
        sys.stdout.flush()
        self._computeANM()
        self._computePCA()

    def _computeANM(self):

        print("En _computeANM")
        sys.stdout.flush()
        pdb_ca = parsePDB(self.inputPdb, subset = 'ca')
        print("En _computeANM")
        sys.stdout.flush()
        print pdb_ca.getTitle()
        sys.stdout.flush()
        self.anm = calcANM(pdb_ca)
        print("En _computeANM")
        sys.stdout.flush()
        fnOutAnm = self._getExtraPath("anmModes")
        saveModel(self.anm, fnOutAnm)

    def _computePCA(self):
        sim = parsePDB(self.inputPdb)
        initialTrajectory = self.initTrajectory.get()
        finalTrajectory = self.finTrajectory.get()
        combined_traj = Trajectory(initialTrajectory)
        combined_traj.setCoords(sim)
        combined_traj.setAtoms(sim.ca)
        combined_traj.addFile(finalTrajectory)
        self.pca = PCA('AMPAR trajectories')
        self.pca.buildCovariance(combined_traj)
        self.pca.calcModes()
        fnOutPca = self._getExtraPath("pcaModes")
        saveModel(self.pca, fnOutPca)

    '''def _summary(self):
        try:
            anm = self._getExtraPath("anmModes.anm.npz")
            pca = self._getExtraPath("pcaModes.pca.npz")

            anm = loadModel(anm)
            pca = loadModel(pca)

            table = getOverlapTable(pca[:7], anm[:7])
            return table
        except:
            summary = []
            summary.append("At the end of the process it will be shown an "
                           "overlap table between anm and pca.")
            return summary'''

    def _citations(self):
        return ['Kurkcuoglu2016']

    '''def _validate(self):
        errors = []
        if which('ProDy') is '':
            errors.append('You should have the program ProDy in the PATH')
        return errors'''

    '''def _warnings(self):
        warnings = []
        text = commands.getoutput("ProDy --version")
        print text
        version = text[14:19]

        if version != '1.9.3':
            warnings.append('Warning: Prody was tested with version 1.9.3 and '
                            'your version is different.')
        return warnings'''