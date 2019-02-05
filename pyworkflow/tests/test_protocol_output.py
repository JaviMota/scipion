# **************************************************************************
# *
# * Authors:     Pablo Conesa (pconesa@cnb.csic.es)
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
from pyworkflow.em.protocol.protocol_tests import ProtOutputTest
from tests import *
from pyworkflow.mapper import SqliteMapper
from pyworkflow.protocol.constants import STATUS_FINISHED
from pyworkflow.protocol.executor import StepExecutor


# Protocol to output of basic scipion objects

class TestProtocolOutputs(BaseTest):

    @classmethod
    def setUpClass(cls):
        setupTestProject(cls)
        setupTestOutput(cls)

    def test_basicObjectOutput(self):
        """Test the list with several Complex"""
        fn = self.getOutputPath("protocol.sqlite")
        mapper = SqliteMapper(fn, globals())
        prot = ProtOutputTest(mapper=mapper, n=2,
                              workingDir=self.getOutputPath(''))
        prot._stepsExecutor = StepExecutor(hostConfig=None)
        prot.run()

        self.assertEqual(prot._steps[0].getStatus(), STATUS_FINISHED)

    def test_basicObjectInProject(self):

        prot = self.newProtocol(ProtOutputTest,
                                objLabel='to generate basic input')

        self.launchProtocol(prot)
        # Default value is 10 so output is 20
        self.assertOutput(prot)

        # Second protocol to test linking
        prot2 = self.newProtocol(ProtOutputTest,
                                 objLabel='to read basic input')

        # Set the pointer for the integer
        prot2.iBoxSize.setPointer(prot, "oBoxSize")
        self.launchProtocol(prot2)
        self.assertOutput(prot2, value=40)

    def assertOutput(self, prot, value=20):

        # Check there is an output
        self.assertTrue(hasattr(prot, 'oBoxSize'),
                        msg="Protocol output boxSize (OInteger) not registered"
                            " as attribute.")

        self.assertEqual(value, prot.oBoxSize.get(),
                         "oBoxSize value is wrong: %s , expected %s" %
                         (prot.oBoxSize, value))
