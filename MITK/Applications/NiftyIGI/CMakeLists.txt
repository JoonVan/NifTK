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

project(NiftyIGI)
set(MY_APP_NAME NiftyIGI)

# Create a cache entry for the provisioning file which is used to export
# the file name in the NifTKConfig.cmake file. This will keep external projects
# which rely on this file happy.
set(NIFTK_NIFTYIGI_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/NiftyIGI.provisioning" CACHE INTERNAL "NiftyIGI provisioning file" FORCE)

###########################################################
# Note: You need to explicitly turn OFF MITK/CTK plugins.
#
# i.e. they are assumed to be well tested/safe.
# So, turn them OFF if you DON'T want them in your app.
###########################################################
set(_exclude_plugins
  org_mitk_gui_qt_extapplication
  org_mitk_gui_qt_volumevisualization
  org_mitk_gui_qt_segmentation
  org_mitk_gui_qt_moviemaker
  org_mitk_gui_qt_imagecropper
  org_mitk_gui_qt_measurement
  org_mitk_gui_qt_basicimageprocessing
  org_mitk_gui_qt_measurementtoolbox
)

###########################################################
# Note: You need to explicitly turn ON NifTK plugins.
#
# i.e. YOU are explicitly stating (via the git commit log)
# that YOU have tested them on Windows, Linux and Mac,
# and are convinced that they work with this applications
# set of Editors and Views/Plugins.
###########################################################
set(_include_plugins
  uk.ac.ucl.cmic.commonapps
  uk.ac.ucl.cmic.niftyigi
  uk.ac.ucl.cmic.common
  uk.ac.ucl.cmic.affinetransform
  uk.ac.ucl.cmic.surfaceextractor
  uk.ac.ucl.cmic.snapshot
  uk.ac.ucl.cmic.igiultrasoundoverlayeditor
  uk.ac.ucl.cmic.igidatasources
  uk.ac.ucl.cmic.igisurfacerecon
  uk.ac.ucl.cmic.igitrackedimage
  uk.ac.ucl.cmic.igitrackedpointer
  uk.ac.ucl.cmic.igipointreg
  uk.ac.ucl.cmic.igisurfacereg
  uk.ac.ucl.cmic.igiundistort
  uk.ac.ucl.cmic.igirmserror
  uk.ac.ucl.cmic.igipivotcalibration
  uk.ac.ucl.cmic.igipointercalib
  uk.ac.ucl.cmic.igipointsetcropper
  uk.ac.ucl.cmic.igicameracal
  uk.ac.ucl.cmic.igimaskmerger
  uk.ac.ucl.cmic.igiultrasoundreconstruction
)

if(WIN32)
  set(_include_plugins
    ${_include_plugins}
    uk.ac.ucl.cmic.igifootpedalhotkey
  )
endif()

if(BUILD_VL AND NOT APPLE)

  # Issue #4757: If available, we should use VL, as the MITK/VTK viewer
  # goes wonky when you add points to point-sets.

  set(_include_plugins
    ${_include_plugins}
    uk.ac.ucl.cmic.igivlvideooverlayeditor
  )
else()
  set(_include_plugins
    ${_include_plugins}
    uk.ac.ucl.cmic.igivideooverlayeditor
  )
endif()

if (BUILD_Caffe)
  set(_include_plugins
    ${_include_plugins}
    uk.ac.ucl.cmic.caffesegmentor
  )
endif()

if(OPENCV_WITH_NONFREE)
  set(_include_plugins
    ${_include_plugins}
    uk.ac.ucl.cmic.igidistancemeasurer
  )
endif()

# Horrible work-around to force NiftyIGI to preload niftknvapi.
# Can be removed when we finally upgrade to CUDA 7.5 throughout.
if(WIN32 AND NVAPI_FOUND AND CUDA_FOUND AND NIFTK_USE_CUDA AND CUDA_VERSION_MAJOR LESS 7)
  add_definitions(-DNIFTYIGI_USE_NVAPI)
endif()

NIFTK_CREATE_GUI_APPLICATION(
  NAME NiftyIGI
  INCLUDE_PLUGINS ${_include_plugins}
  EXCLUDE_PLUGINS ${_exclude_plugins}
)
