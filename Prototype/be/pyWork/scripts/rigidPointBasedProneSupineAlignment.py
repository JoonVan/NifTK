#! /usr/bin/env python 
# -*- coding: utf-8 -*-

import numpy as np
import nibabel as nib
import orthogonalProcurstesPointRegistration as opr
import mayaviPlottingWrap as plotWrap
import conversions as cvs
import commandExecution as cmdEx
from rigidHelpers import rotMatZ
import scipy.linalg as la
import aladinRegistrationTask as aladinTask
import rregRegistrationTask as rregTask
import sys


def readFileAsArray( strFileNameIn ):
    f       = file(strFileNameIn, 'r')
    lines   = f.readlines()
    numCols = len( lines[1].split() )
    numRows = len( lines ) - 1
    
    results = np.zeros( (numRows, numCols) )
    
    for i in range( 1, len( lines ) ) :
        results[i-1,:]  = lines[i].split()

    return results


fileNameSupinePoints = 'W:/philipsBreastProneSupine/Results_supinePointsOnCostalCartilage.txt'
fileNamePronePoints  = 'W:/philipsBreastProneSupine/Results_pronePointsOnCostalCartilage.txt'

imgFileNameProne  = 'W:/philipsBreastProneSupine/prone1k.nii'
imgFileNameSupine = 'W:/philipsBreastProneSupine/supine1k.nii'


# Get the image spacing
mProne  = readFileAsArray( fileNamePronePoints  )[:,-3:]
mSupine = readFileAsArray( fileNameSupinePoints )[:,-3:]

proneImg  = nib.load( imgFileNameProne  )
supineImg = nib.load( imgFileNameSupine )

proneQMat  = proneImg.get_header().get_qform()
supineQMat = supineImg.get_header().get_qform()

# now get the measured points... (these should already be in real world coordinatess)
reg = opr.orthogonalProcrustesPointRegistration( mSupine.T, mProne.T )
reg.register()


# homogeneous supine points
sPH = np.array( [mSupine[:,0], mSupine[:,1], mSupine[:,2], np.ones(mSupine.shape[0])] ).reshape( 4, mSupine.shape[0] )

supineReg = np.dot( reg.homRigTransfromMat, sPH )


corMat    = rotMatZ( np.pi )
homNiiMat = np.dot( corMat, np.dot( la.inv(reg.homRigTransfromMat), corMat ) )

plotWrap.plotArrayAs3DPoints( mProne, (1,0,0) )
plotWrap.plotArrayAs3DPoints( mSupine, (0,1,0) )
plotWrap.plotArrayAs3DPoints( supineReg.T, (0,0,1) )

#
# Check the point distance
#

displacement = mProne - supineReg.T[:,0:3]
distance = []
for d in displacement : 
    distance.append( np.linalg.norm(d) )
distance = np.array( distance )


meanDist = np.mean( distance )
maxDist  = np.max ( distance )
minDist  = np.min ( distance )
stdDist  = np.std ( distance )
print( 'Accuracy achieved: ' )
print( '  - mean distance: %.4f '  %meanDist ) 
print( '  - min  distance: %.4f '  %minDist  ) 
print( '  - max  distance: %.4f '  %maxDist  ) 
print( '  - std  distance: %.4f '  %stdDist  ) 

sys.exit()

# Write the result into a file
strDOFFileOut    = 'Z:/documents/Project/philipsBreastProneSupine/rigidAlignment/rotMat.txt'
strImgFileOut    = 'Z:/documents/Project/philipsBreastProneSupine/rigidAlignment/supineTransform.nii'
outPath          = 'Z:/documents/Project/philipsBreastProneSupine/rigidAlignment/'
f  = file( strDOFFileOut,    'w' )
f.write ( cvs.numpyArrayToStr( homNiiMat , True, '' ) )
f.close()

# now resample the supine image into the prone-space
resampleCmd     = 'reg_resample'
resampleParams  = ' -target ' +  imgFileNameProne
resampleParams += ' -source ' +  imgFileNameSupine
resampleParams += ' -aff '    +  strDOFFileOut
resampleParams += ' -result ' +  strImgFileOut

cmdEx.runCommand( resampleCmd, resampleParams )

# Try subsequent registration with reg_aladin
#regCmd = 'reg_aladin'
#
#aReg = aladinTask.aladinRegistrationTask( targetIn=imgFileNameProne, sourceIn=imgFileNameSupine, maskIn='', 
#                                          outputPath=outPath, registrationID='aladin', affineInit=strDOFFileOut, rigOnly=True )
#aReg.run()
#paramFile = 'Z:/documents/Project/philipsBreastProneSupine/rigidAlignment/ArrineRegn.params'
#
#rReg = rregTask.rregRegistrationTask(targetIn=imgFileNameProne, sourceIn=imgFileNameSupine, outputPath=outPath, registrationID='rreg', parameterFile=paramFile, targetThreshold=-1001 )
#rReg.run()

print( 'Done.' )
















