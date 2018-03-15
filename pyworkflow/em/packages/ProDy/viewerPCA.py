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
"""
This module implement the wrappers around ProDy protocol
visualization program.
"""

from pyworkflow.viewer import ProtocolViewer, DESKTOP_TKINTER, WEB_DJANGO
from pyworkflow.em import *
from computePCATrajectory import computeModesPcaPdb
from prody import *
import matplotlib.pyplot as plt


class ProdyViewerPca(Viewer):
    """ Wrapper to visualize Pdb to SAXS. """
    _targets = [computeModesPcaPdb]
    _environments = [DESKTOP_TKINTER, WEB_DJANGO]

    def __init__(self, **args):
        Viewer.__init__(self, **args)

    def visualize(self, obj, **args):

        cls = type(obj)

        if issubclass(cls, computeModesPcaPdb):
            fnPca = obj._getExtraPath("pcaModes.pca.npz")
            pca = loadModel(fnPca)
            pdb = parsePDB(obj._getExtraPath("inputPdb.pdb"))

            lenTraj = len(obj.Trajectory.get())

            for i, traj in enumerate(obj.Trajectory.get()):
                trajectory = Trajectory(traj.getFileName())
                trajectory.setCoords(pdb)
                trajectory.setAtoms(pdb.ca)

                c = str(float(i) / float(lenTraj))
                if i == 0:
                    showProjection(trajectory, pca[:2], new_fig=True,
                                   color=[c]*len(trajectory))
                else:
                    showProjection(trajectory, pca[:2], new_fig=False,
                                   color=[c]*len(trajectory))





