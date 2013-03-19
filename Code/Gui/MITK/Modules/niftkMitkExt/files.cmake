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

set(CPP_FILES
  Algorithms/mitkNifTKCoreObjectFactory.cpp
  Algorithms/mitkMIDASMorphologicalSegmentorPipelineManager.cpp
  Common/mitkMIDASImageUtils.cpp
  Common/mitkMIDASOrientationUtils.cpp
  Common/mitkPointUtils.cpp
  DataManagement/mitkDataNodeBoolPropertyFilter.cpp
  DataManagement/mitkDataNodeStringPropertyFilter.cpp
  DataManagement/mitkDataStorageUtils.cpp
  DataManagement/mitkDataStorageListener.cpp
  DataManagement/mitkDataStoragePropertyListener.cpp
  DataManagement/mitkDataStorageVisibilityTracker.cpp
  DataManagement/mitkMIDASNodeAddedVisibilitySetter.cpp
  DataManagement/mitkMIDASDataNodeNameStringFilter.cpp
  DataManagement/mitkCoordinateAxesData.cpp
  Rendering/mitkCoordinateAxesVtkMapper3D.cpp
  DataNodeProperties/mitkAffineTransformParametersDataNodeProperty.cpp
  DataNodeProperties/mitkAffineTransformDataNodeProperty.cpp
  DataNodeProperties/mitkITKRegionParametersDataNodeProperty.cpp
  DataNodeProperties/mitkNamedLookupTableProperty.cpp
  Interactions/mitkMIDASTool.cpp
  Interactions/mitkMIDASContourToolEventInterface.cpp
  Interactions/mitkMIDASContourToolOpAccumulateContour.cpp
  Interactions/mitkMIDASContourTool.cpp
  Interactions/mitkMIDASDrawToolEventInterface.cpp
  Interactions/mitkMIDASDrawToolOpEraseContour.cpp
  Interactions/mitkMIDASDrawTool.cpp
  Interactions/mitkMIDASPointSetInteractor.cpp
  Interactions/mitkMIDASPolyToolEventInterface.cpp
  Interactions/mitkMIDASPolyToolOpAddToFeedbackContour.cpp
  Interactions/mitkMIDASPolyToolOpUpdateFeedbackContour.cpp
  Interactions/mitkMIDASPolyTool.cpp
  Interactions/mitkMIDASSeedTool.cpp
  Interactions/mitkMIDASPosnTool.cpp
  Interactions/mitkMIDASPaintbrushToolEventInterface.cpp
  Interactions/mitkMIDASPaintbrushToolOpEditImage.cpp
  Interactions/mitkMIDASPaintbrushTool.cpp
  Interactions/mitkMIDASViewKeyPressStateMachine.cpp
  Interactions/mitkMIDASToolKeyPressStateMachine.cpp
  IO/itkAnalyzeImageIO3160.cpp
  IO/itkDRCAnalyzeImageIO3160.cpp
  IO/itkNiftiImageIO3201.cpp
  IO/mitkNifTKItkImageFileReader.cpp
  IO/mitkNifTKItkImageFileIOFactory.cpp
  IO/itkPNMImageIOFactory.cpp
  IO/itkPNMImageIO.cpp
)
