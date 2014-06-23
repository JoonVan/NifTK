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
  DataManagement/mitkMIDASDataNodeNameStringFilter.cxx
  Algorithms/mitkMIDASMorphologicalSegmentorPipelineManager.cxx
  Interactions/mitkMIDASTool.cxx
  Interactions/mitkMIDASContourToolEventInterface.cxx
  Interactions/mitkMIDASContourToolOpAccumulateContour.cxx
  Interactions/mitkMIDASContourTool.cxx
  Interactions/mitkMIDASDrawToolEventInterface.cxx
  Interactions/mitkMIDASDrawToolOpEraseContour.cxx
  Interactions/mitkMIDASDrawTool.cxx
  Interactions/mitkMIDASEventFilter.cxx
  Interactions/mitkMIDASPointSetInteractor.cxx
  Interactions/mitkMIDASPolyToolEventInterface.cxx
  Interactions/mitkMIDASPolyToolOpAddToFeedbackContour.cxx
  Interactions/mitkMIDASPolyToolOpUpdateFeedbackContour.cxx
  Interactions/mitkMIDASPolyTool.cxx
  Interactions/mitkMIDASSeedTool.cxx
  Interactions/mitkMIDASPosnTool.cxx
  Interactions/mitkMIDASPaintbrushToolEventInterface.cxx
  Interactions/mitkMIDASPaintbrushToolOpEditImage.cxx
  Interactions/mitkMIDASPaintbrushTool.cxx
  Interactions/mitkMIDASRendererFilter.cxx
  Interactions/mitkMIDASStateMachine.cxx
  Interactions/mitkMIDASToolKeyPressStateMachine.cxx
)

set(RESOURCE_FILES
  Interactions/DisplayConfigMIDASTool.xml
  Interactions/DisplayConfigMIDASPaintbrushTool.xml
  Interactions/MIDASDrawToolConfig.xml
  Interactions/MIDASDrawToolStateMachine.xml
  Interactions/MIDASPolyToolConfig.xml
  Interactions/MIDASPolyToolStateMachine.xml
  Interactions/MIDASPaintbrushToolConfig.xml
  Interactions/MIDASPaintbrushToolStateMachine.xml
  Interactions/MIDASSeedDropperConfig.xml
  Interactions/MIDASSeedDropperStateMachine.xml
  Interactions/MIDASSeedToolConfig.xml
  Interactions/MIDASSeedToolStateMachine.xml
  Interactions/MIDASToolConfig.xml
  Interactions/MIDASToolStateMachine.xml
)
