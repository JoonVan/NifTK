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
)

SET(INTERNAL_CPP_FILES
  MIDASGeneralSegmentorViewActivator.cxx
  MIDASGeneralSegmentorViewPreferencePage.cxx
  MIDASGeneralSegmentorViewHelper.cxx
  MIDASGeneralSegmentorViewControlsWidget.cxx
  MIDASGeneralSegmentorViewEventInterface.cxx
  MIDASGeneralSegmentorView.cxx
)

SET(UI_FILES
  src/internal/MIDASGeneralSegmentorViewControls.ui
)

SET(MOC_H_FILES
  src/internal/MIDASGeneralSegmentorViewActivator.h
  src/internal/MIDASGeneralSegmentorViewControlsWidget.h
  src/internal/MIDASGeneralSegmentorView.h
  src/internal/MIDASGeneralSegmentorViewPreferencePage.h
)

SET(CACHED_RESOURCE_FILES
  resources/MIDASGeneralSegmentor.png
  plugin.xml
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
#  resources/MIDASGeneralSegmentorView.qrc
)

SET(CPP_FILES 
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
