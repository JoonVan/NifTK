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
  Interfaces/niftkIGIDataSourceI.cxx
  Interfaces/niftkIGILocalDataSourceI.cxx
  Interfaces/niftkIGISaveableDataSourceI.cxx
  Interfaces/niftkIGIDataSourceFactoryServiceI.cxx
  Interfaces/niftkIGIDataSourceFactoryServiceRAII.cxx
  DataType/niftkIGIDataType.cxx
  DataType/niftkIGITrackerDataType.cxx
  DataSource/niftkIGIDataSource.cxx
  DataSource/niftkIGIDataSourceBuffer.cxx
  DataSource/niftkIGIWaitForSavedDataSourceBuffer.cxx
  Threads/niftkIGITimerBasedThread.cxx
  Threads/niftkIGIDataSourceGrabbingThread.cxx
  Threads/niftkIGIDataSourceBackgroundSaveThread.cxx
  Threads/niftkIGIDataSourceBackgroundDeleteThread.cxx
  Dialogs/niftkIGIInitialisationDialog.cxx
  Dialogs/niftkIGIConfigurationDialog.cxx
  Dialogs/niftkMITKTrackerDialog.cxx
  Dialogs/niftkIPPortDialog.cxx
  Dialogs/niftkIPHostPortDialog.cxx
  Dialogs/niftkLagDialog.cxx
  Conversion/niftkQImageToMitkImageFilter.cxx
  mitkIGIDataType.cxx
  mitkIGIDataSource.cxx
  mitkIGIOpenCVDataType.cxx
  QmitkIGITimerBasedThread.cxx
#  QmitkFiducialRegistrationWidgetDialog.cxx
  QmitkIGINiftyLinkDataType.cxx
  QmitkIGINiftyLinkDataSource.cxx
  QmitkIGINiftyLinkDataSourceGui.cxx
  QmitkIGIDataSource.cxx
  QmitkIGIDataSourceBackgroundSaveThread.cxx
  QmitkIGIDataSourceGui.cxx
  QmitkIGILocalDataSource.cxx
  QmitkIGILocalDataSourceGrabbingThread.cxx
  QmitkIGITrackerSource.cxx
  QmitkIGITrackerSourceGui.cxx
  QmitkIGIUltrasonixTool.cxx
  QmitkIGIUltrasonixToolGui.cxx
  QmitkIGIOpenCVDataSource.cxx
  QmitkIGIOpenCVDataSourceGui.cxx
)

set(MOC_H_FILES
  Threads/niftkIGITimerBasedThread.h
  Dialogs/niftkIGIInitialisationDialog.h
  Dialogs/niftkIGIConfigurationDialog.h
  Dialogs/niftkMITKTrackerDialog.h
  Dialogs/niftkIPPortDialog.h
  Dialogs/niftkIPHostPortDialog.h
  Dialogs/niftkLagDialog.h
  QmitkIGITimerBasedThread.h
#  QmitkFiducialRegistrationWidgetDialog.h
  QmitkIGINiftyLinkDataSource.h
  QmitkIGINiftyLinkDataSourceGui.h
  QmitkIGIDataSource.h
  QmitkIGIDataSourceGui.h
  QmitkIGILocalDataSource.h
  QmitkIGITrackerSource.h
  QmitkIGITrackerSourceGui.h
  QmitkIGIUltrasonixTool.h
  QmitkIGIUltrasonixToolGui.h
  QmitkIGIOpenCVDataSource.h
  QmitkIGIOpenCVDataSourceGui.h
)

set(UI_FILES
  Dialogs/niftkMITKTrackerDialog.ui
  Dialogs/niftkIPPortDialog.ui
  Dialogs/niftkIPHostPortDialog.ui
  Dialogs/niftkLagDialog.ui
#  QmitkFiducialRegistrationWidgetDialog.ui
  QmitkIGITrackerSourceGui.ui
  QmitkIGIUltrasonixToolGui.ui
)

set(QRC_FILES
)

# optional audio data source depends on qt-multimedia, which may not be available.
if(QT_QTMULTIMEDIA_INCLUDE_DIR)
  set(CPP_FILES
    ${CPP_FILES}
    AudioDataSource.cxx
    AudioDataSourceGui.cxx
  )
  set(MOC_H_FILES
    ${MOC_H_FILES}
    AudioDataSource.h
    AudioDataSourceGui.h
  )
  set(UI_FILES
    ${UI_FILES}
    AudioDataSourceGui.ui
  )
endif()
