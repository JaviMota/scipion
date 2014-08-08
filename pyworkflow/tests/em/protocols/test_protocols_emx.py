"""
/***************************************************************************
 * Authors:     Roberto Marabini (roberto@cnb.csic.es)
 *              J.M. De la Rosa Trevin (jmdelarosa@cnb.csic.es)
 *
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
MODIFICATION ADVICE:

Please,  do not  generate or  distribute 
a modified version of this file under its original name. 
"""
import unittest, sys
from itertools import izip
from pyworkflow.em import *
from pyworkflow.tests import *


class TestEmxBase(BaseTest):
    @classmethod
    def setUpClass(cls):
        setupTestProject(cls)
        cls.dataset = DataSet.getDataSet('emx')
    
    def test_coodinatesTest1(self):
        """ Import an EMX file with just one micrograph 
        and a few coordinates.
        """
        emxFn = self.dataset.getFile('coordinatesT1')
        protEmxImport = self.newProtocol(ProtEmxImport, 
                                         inputEMX=emxFn,
                                         amplitudeContrast=0.1,
                                         sphericalAberration=2.,
                                         voltage=100)
        self.launchProtocol(protEmxImport)

        # Reference coordinates
        coords = SetOfCoordinates(filename=self.dataset.getFile('coordinatesGoldT1'))
        BaseTest.compareSets(self, protEmxImport.outputCoordinates, coords)
        
    def test_particleDefocus(self):
        """ Import an EMX file with a stack of particles
        that has defocus
        """
        #SCIPION_TESTS=/home/roberto/Scipion/pyworkflow-code/data/tests
        #SCIPION_USER_DATA=/home/roberto/ScipionUserData

        print "self.folder", self.dataset.folder
        #os.chdir(os.environ['SCIPION_TESTS'])
        print "os.environ['SCIPION_HOME']", os.environ['SCIPION_HOME']
        emxFn = self.dataset.getFile('defocusParticleT2')
        print "emxFn1",emxFn
        protEmxImport = self.newProtocol(ProtEmxImport, 
                                         inputEMX=emxFn
                                         )
        self.launchProtocol(protEmxImport)
        micFn =self.dataset.getFile('micrographsGoldT2')
        mics  = SetOfMicrographs(filename = micFn)
            
        for mic1, mic2 in izip(mics, protEmxImport.outputMicrographs):
            # Remove the absolute path in the micrographs to 
            # really check that the attributes should be equal
            mic1.setFileName(os.path.basename(mic1.getFileName()))
            mic2.setFileName(os.path.basename(mic2.getFileName()))
            mic1.printAll()
            mic2.printAll()
            self.assertTrue(mic1.equalAttributes(mic2))




