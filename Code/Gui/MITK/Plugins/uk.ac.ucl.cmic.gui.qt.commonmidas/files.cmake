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

SET(SRC_CPP_FILES
  QmitkMIDASBaseSegmentationFunctionality.cxx
  QmitkMIDASSegmentationViewWidget.cxx
)

SET(INTERNAL_CPP_FILES
  MIDASActivator.cxx
)

SET(UI_FILES
  src/QmitkMIDASSegmentationViewWidget.ui
)

SET(MOC_H_FILES
  src/internal/MIDASActivator.h
  src/QmitkMIDASBaseSegmentationFunctionality.h
  src/QmitkMIDASSegmentationViewWidget.h
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

# todo: add some qt style sheet resources
SET(QRC_FILES
  resources/CommonMIDASResources.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})