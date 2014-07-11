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

###############################################################################
# Note: This is for creating a BlueBerry Application, not a generic Qt one.
###############################################################################

macro(NIFTK_CREATE_GUI_APPLICATION)
  MACRO_PARSE_ARGUMENTS(_APP
                        "NAME;INCLUDE_PLUGINS;EXCLUDE_PLUGINS"
                        ""
                        ${ARGN}
                        )

  if(NOT _APP_NAME)
    message(FATAL_ERROR "NAME argument cannot be empty.")
  endif()
                        
  set(MY_APP_NAME ${_APP_NAME})

  # ... and here we are specifying additional link time dependencies.
  set(_link_libraries
    niftkCore
    qtsingleapplication
  )

  set(_app_options)
  if(${NIFTK_SHOW_CONSOLE_WINDOW})
    list(APPEND _app_options SHOW_CONSOLE)
  endif()

  set(_include_plugins
    ${_APP_INCLUDE_PLUGINS}
  )
  set(_exclude_plugins
    ${_APP_EXCLUDE_PLUGINS}
  )
  
  # NOTE: Check CMake/PackageDepends for any additional dependencies.
  set(_library_dirs
    ${NiftyLink_LIBRARY_DIRS}
    ${Boost_LIBRARY_DIRS}
    ${aruco_DIR}/lib
  )

  # FIXME
  if(BUILD_IGI)
    include(${CMAKE_SOURCE_DIR}/CMake/PackageDepends/MITK_NiftyLink_Config.cmake)
    list(APPEND _library_dirs ${NiftyLink_LIBRARY_DIRS})
  endif()

  #############################################################################
  # Watch out for this:
  # In the top level CMakeLists, MACOSX_BUNDLE_NAMES will contain all the apps
  # that we have available in this project. This is so that when you create a
  # Module, or a Plugin, under the hood, you are using an MITK macro, which 
  # takes care of copying said Module, Plugin into ALL bundles. HOWEVER, within
  # the FunctionCreateBlueBerryApplication, this has the side effect of
  # copying all MITK and CTK plugins into ALL bundles. This breaks the build at
  # install time, as you package up the first application and then when the 
  # second application is created, this function will again copy all the MITK 
  # and CTK plugins back into the first app, which overwrites all the library
  # @executable path settings leading to an invalid bundle for all but the 
  # last executable. So, we save the variable here, and restore it at the end
  # of this macro.
  #############################################################################
  
  if(APPLE)
    set(TMP_MACOSX_BUNDLE_NAMES ${MACOSX_BUNDLE_NAMES})
    set(MACOSX_BUNDLE_NAMES ${MY_APP_NAME})
  endif()
  
  FunctionCreateBlueBerryApplication(
    NAME ${MY_APP_NAME}
    SOURCES ${MY_APP_NAME}.cxx
    PLUGINS ${_include_plugins}
    EXCLUDE_PLUGINS ${_exclude_plugins}
    LINK_LIBRARIES ${_link_libraries}
    LIBRARY_DIRS ${_library_dirs}
    ${_app_options}
  )

  mitk_use_modules(TARGET ${MY_APP_NAME} MODULES niftkCore qtsingleapplication)

  #############################################################################
  # Restore this MACOSX_BUNDLE_NAMES variable. See long-winded note above.
  #############################################################################
  if(APPLE)
    set(MACOSX_BUNDLE_NAMES ${TMP_MACOSX_BUNDLE_NAMES})
    set_target_properties(${MY_APP_NAME} PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER ${MY_APP_NAME})
    set_target_properties(${MY_APP_NAME} PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING "${NIFTK_VERSION_STRING}_${NIFTK_REVISION_SHORTID}_${NIFTK_DATE_TIME}")
    set_target_properties(${MY_APP_NAME} PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING ${NIFTK_VERSION_STRING})
    set_target_properties(${MY_APP_NAME} PROPERTIES MACOSX_BUNDLE_COPYRIGHT ${NIFTK_COPYRIGHT})
  endif()
  
endmacro()
