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
  QmitkNiftyMIDASApplication.cxx
  QmitkNiftyMIDASAppWorkbenchAdvisor.cxx
  QmitkNiftyMIDASWorkbenchWindowAdvisor.cxx
)

SET(INTERNAL_CPP_FILES
  QmitkNiftyMIDASApplicationPlugin.cxx
)

SET(MOC_H_FILES
  src/QmitkNiftyMIDASApplication.h
  src/QmitkNiftyMIDASWorkbenchWindowAdvisor.h
  src/internal/QmitkNiftyMIDASApplicationPlugin.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/icon_ion.xpm
  resources/icon_ucl.xpm
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/QmitkNiftyMIDASApplication.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
