set(SRC_CPP_FILES
  QmitkThumbnailViewPreferencePage.cpp
  QmitkThumbnailRenderWindow.cpp  
)

set(INTERNAL_CPP_FILES
  ThumbnailViewActivator.cpp
  ThumbnailView.cpp
)

set(UI_FILES
  src/internal/ThumbnailViewControls.ui
)

set(MOC_H_FILES
  src/QmitkThumbnailViewPreferencePage.h
  src/QmitkWheelEventEater.h
  src/QmitkMouseEventEater.h
  src/QmitkThumbnailRenderWindow.h
  src/internal/ThumbnailViewActivator.h
  src/internal/ThumbnailView.h
)

# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
set(CACHED_RESOURCE_FILES
  resources/icon.xpm
  plugin.xml
)

# list of Qt .qrc files which contain additional resources
# specific to this plugin
set(QRC_FILES

)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

