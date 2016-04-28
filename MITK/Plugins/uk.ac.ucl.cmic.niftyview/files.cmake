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
  QmitkNiftyViewApplication.cxx
  QmitkNiftyViewAppWorkbenchAdvisor.cxx
  QmitkNiftyViewWorkbenchWindowAdvisor.cxx
)

set(INTERNAL_CPP_FILES
  QmitkNiftyViewApplicationPlugin.cxx
)

set(MOC_H_FILES
  src/QmitkNiftyViewApplication.h
  src/QmitkNiftyViewWorkbenchWindowAdvisor.h
  src/internal/QmitkNiftyViewApplicationPlugin.h
)

set(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/icon_cmic.xpm
  resources/icon_ion.xpm
  resources/icon_ucl.xpm
)

set(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/QmitkNiftyViewApplication.qrc
)

set(CPP_FILES )

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})