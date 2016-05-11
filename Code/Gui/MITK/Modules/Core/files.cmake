#/*============================================================================
#
#  NifTK: A software platform for medical image computing.
#
#  Copyright (c) University College London (UCL). All rights reserved.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.
#
#  See LICENSE.txt in the top level directory for details.
#
#============================================================================*/

set(H_FILES
  Common/niftkImageOrientation.h
)

set(CPP_FILES
  Algorithms/mitkNifTKCoreObjectFactory.cxx
  Algorithms/mitkNifTKAffineTransformer.cxx
  Algorithms/mitkNifTKCMC33.cpp
  Algorithms/mitkNifTKImageToSurfaceFilter.cpp
  Algorithms/mitkNifTKMeshSmoother.cpp
  Common/niftkImageUtils.cxx
  Common/niftkImageOrientationUtils.cxx
  Common/mitkPointUtils.cxx
  Common/mitkMergePointClouds.cxx
  DataManagement/mitkDataNodeBoolPropertyFilter.cxx
  DataManagement/mitkDataNodeStringPropertyFilter.cxx
  DataManagement/mitkDataStorageUtils.cxx
  DataManagement/mitkDataStorageListener.cxx
  DataManagement/mitkDataNodePropertyListener.cxx
  DataManagement/mitkDataNodeVisibilityTracker.cxx
  DataManagement/mitkCoordinateAxesData.cxx
  DataManagement/mitkCoordinateAxesDataOpUpdate.cxx
  DataManagement/mitkBasicMesh.cpp
  DataManagement/mitkBasicTriangle.cpp
  DataManagement/mitkBasicVec3D.cpp
  DataManagement/mitkBasicVertex.cpp
  Rendering/mitkCoordinateAxesVtkMapper3D.cxx
  Rendering/mitkFastPointSetVtkMapper3D.cxx
  Rendering/vtkOpenGLMatrixDrivenCamera.cxx
  DataNodeProperties/mitkAffineTransformParametersDataNodeProperty.cxx
  DataNodeProperties/mitkAffineTransformDataNodeProperty.cxx
  DataNodeProperties/mitkITKRegionParametersDataNodeProperty.cxx
  DataNodeProperties/mitkNamedLookupTableProperty.cxx
  DataNodeProperties/mitkLabeledLookupTableProperty.cxx
  IO/mitkFileIOUtils.cxx
  Interactions/mitkPointSetUpdate.cxx
)

