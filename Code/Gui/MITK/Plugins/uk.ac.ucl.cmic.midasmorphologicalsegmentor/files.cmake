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

set(SRC_CPP_FILES  
)

set(INTERNAL_CPP_FILES
  niftkMorphologicalSegmentorActivator.cxx
  niftkMorphologicalSegmentorPreferencePage.cxx
  niftkMorphologicalSegmentorView.cxx
)

set(UI_FILES
)

set(MOC_H_FILES
  src/internal/niftkMorphologicalSegmentorActivator.h
  src/internal/niftkMorphologicalSegmentorPreferencePage.h
  src/internal/niftkMorphologicalSegmentorView.h
)

set(CACHED_RESOURCE_FILES
  resources/niftkMorphologicalSegmentor.png
  plugin.xml
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
)

set(QRC_FILES
)

set(CPP_FILES 
)

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})
