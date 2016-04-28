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
  Dialogs/QmitkMIDASNewSegmentationDialog.cxx
  QmitkMIDASToolSelectorWidget.cxx
  QmitkMIDASImageAndSegmentationSelectorWidget.cxx
  QmitkMIDASDrawToolGUI.cxx
  QmitkMIDASPaintbrushToolGUI.cxx
)

set(MOC_H_FILES 
  Dialogs/QmitkMIDASNewSegmentationDialog.h
  QmitkMIDASToolSelectorWidget.h
  QmitkMIDASImageAndSegmentationSelectorWidget.h
  QmitkMIDASDrawToolGUI.h
  QmitkMIDASPaintbrushToolGUI.h
)

set(UI_FILES
  Resources/UI/QmitkMIDASImageAndSegmentationSelector.ui
  Resources/UI/QmitkMIDASToolSelector.ui
)