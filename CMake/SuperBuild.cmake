#/*================================================================================
#
#  NifTK: An image processing toolkit jointly developed by the
#              Dementia Research Centre, and the Centre For Medical Image Computing
#              at University College London.
#
#  See:        http://dementia.ion.ucl.ac.uk/
#              http://cmic.cs.ucl.ac.uk/
#              http://www.ucl.ac.uk/
#
#  Copyright (c) UCL : See LICENSE.txt in the top level directory for details.
#
#  Last Changed      : $LastChangedDate: 2011-05-06 11:40:44 +0100 (Fri, 06 May 2011) $
#  Revision          : $Revision: 6088 $
#  Last modified by  : $Author: mjc $
#
#  Original author   : m.clarkson@ucl.ac.uk
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notices for more information.
#
#=================================================================================*/

INCLUDE(ExternalProject)

SET(EP_BASE "${CMAKE_BINARY_DIR}/CMakeExternals")
SET_PROPERTY(DIRECTORY PROPERTY EP_BASE ${EP_BASE})

# For external projects like ITK, VTK we always want to turn their testing targets off.
SET(EP_BUILD_TESTING OFF)
SET(EP_BUILD_EXAMPLES OFF)
SET(EP_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

IF(MSVC90 OR MSVC10)
  SET(EP_COMMON_C_FLAGS "${CMAKE_C_FLAGS} /bigobj /MP /W0")
  SET(EP_COMMON_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP /W0")
  SET(CMAKE_CXX_WARNING_LEVEL 0)
ELSE()
  IF(${BUILD_SHARED_LIBS})
    SET(EP_COMMON_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX_EXTRA")
    SET(EP_COMMON_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLINUX_EXTRA")
  ELSE()
    SET(EP_COMMON_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -DLINUX_EXTRA")
    SET(EP_COMMON_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DLINUX_EXTRA")
  ENDIF()
ENDIF()

SET(EP_COMMON_ARGS
  -DBUILD_TESTING:BOOL=${EP_BUILD_TESTING}
  -DBUILD_SHARED_LIBS:BOOL=${EP_BUILD_SHARED_LIBS}
  -DDESIRED_QT_VERSION:STRING=4
  -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_FLAGS:STRING=${EP_COMMON_C_FLAGS}
  -DCMAKE_CXX_FLAGS:STRING=${EP_COMMON_CXX_FLAGS}
  #debug flags
  -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
  -DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}
  #release flags
  -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
  -DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}
  #relwithdebinfo
  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
  -DCMAKE_C_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_C_FLAGS_RELWITHDEBINFO}
)

# Compute -G arg for configuring external projects with the same CMake generator:
IF(CMAKE_EXTRA_GENERATOR)
  SET(GEN "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
ELSE()
  SET(GEN "${CMAKE_GENERATOR}")
ENDIF()

######################################################################
# Include NifTK macro for md5 checking
######################################################################

include(niftkMacroGetChecksum)

######################################################################
# Loop round for each external project, compiling it
######################################################################

SET(EXTERNAL_PROJECTS
  BOOST
  VTK
  GDCM
  ITK
  SlicerExecutionModel  
  DCMTK
  CTK
  NiftyLink
  MITK
  curl
  CGAL
  NiftySim
  NiftyReg
  NiftyRec
  NiftySeg
  NifTKData
)

FOREACH(p ${EXTERNAL_PROJECTS})
  INCLUDE("CMake/CMakeExternals/${p}.cmake")
ENDFOREACH()

######################################################################
# Now compile NifTK, using the packages we just provided.
######################################################################
IF(NOT DEFINED SUPERBUILD_EXCLUDE_NIFTKBUILD_TARGET OR NOT SUPERBUILD_EXCLUDE_NIFTKBUILD_TARGET)

  SET(proj NIFTK)
  SET(proj_DEPENDENCIES ${BOOST_DEPENDS} ${GDCM_DEPENDS} ${ITK_DEPENDS} ${SlicerExecutionModel_DEPENDS} ${VTK_DEPENDS} ${MITK_DEPENDS} )

  IF(BUILD_GUI)
    LIST(APPEND proj_DEPENDENCIES curl)
  ENDIF(BUILD_GUI)

  IF(BUILD_TESTING)
    LIST(APPEND proj_DEPENDENCIES ${NifTKData_DEPENDS})
  ENDIF(BUILD_TESTING)

  IF(BUILD_IGI)
    LIST(APPEND proj_DEPENDENCIES ${NIFTYLINK_DEPENDS})
  ENDIF(BUILD_IGI)

  IF(BUILD_NIFTYREG)
    LIST(APPEND proj_DEPENDENCIES ${NIFTYREG_DEPENDS})
  ENDIF(BUILD_NIFTYREG)

  IF(BUILD_NIFTYSEG)
    LIST(APPEND proj_DEPENDENCIES ${NIFTYSEG_DEPENDS})
  ENDIF(BUILD_NIFTYSEG)

  IF(BUILD_NIFTYSIM)
    LIST(APPEND proj_DEPENDENCIES ${NIFTYSIM_DEPENDS})
  ENDIF(BUILD_NIFTYSIM)

  IF(BUILD_NIFTYREC)
    LIST(APPEND proj_DEPENDENCIES ${NIFTYREC_DEPENDS})
  ENDIF(BUILD_NIFTYREC)

  ExternalProject_Add(${proj}
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ""
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINARY_DIR ${CMAKE_BINARY_DIR}/NifTK-build
    CMAKE_GENERATOR ${GEN}
    CMAKE_ARGS
      ${EP_COMMON_ARGS}
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
      -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
      -DBUILD_TESTING:BOOL=${BUILD_TESTING} # The value set in EP_COMMON_ARGS normally forces this off, but we may need NifTK to be on.
      -DBUILD_SUPERBUILD:BOOL=OFF           # Must force this to be off, or else you will loop forever.
      -DBUILD_COMMAND_LINE_PROGRAMS:BOOL=${BUILD_COMMAND_LINE_PROGRAMS}
      -DBUILD_COMMAND_LINE_SCRIPTS:BOOL=${BUILD_COMMAND_LINE_SCRIPTS}
      -DBUILD_GUI:BOOL=${BUILD_GUI}
      -DBUILD_IGI:BOOL=${BUILD_IGI}
      -DBUILD_PROTOTYPE:BOOL=${BUILD_PROTOTYPE}
      -DBUILD_PROTOTYPE_JHH:BOOL=${BUILD_PROTOTYPE_JHH}
      -DBUILD_PROTOTYPE_TM:BOOL=${BUILD_PROTOTYPE_TM}
      -DBUILD_PROTOTYPE_GY:BOOL=${BUILD_PROTOTYPE_GY}
      -DBUILD_PROTOTYPE_KKL:BOOL=${BUILD_PROTOTYPE_KKL}
      -DBUILD_PROTOTYPE_BE:BOOL=${BUILD_PROTOTYPE_BE}
      -DBUILD_PROTOTYPE_MJC:BOOL=${BUILD_PROTOTYPE_MJC}
      -DBUILD_TESTING:BOOL=${BUILD_TESTING}
      -DBUILD_MESHING:BOOL=${BUILD_MESHING}
      -DBUILD_NIFTYREG:BOOL=${BUILD_NIFTYREG}
      -DBUILD_NIFTYREC:BOOL=${BUILD_NIFTYREC}
      -DBUILD_NIFTYSIM:BOOL=${BUILD_NIFTYSIM}
      -DBUILD_NIFTYSEG:BOOL=${BUILD_NIFTYSEG}
      -DCUDA_SDK_ROOT_DIR:PATH=${CUDA_SDK_ROOT_DIR}
      -DCUDA_CUT_INCLUDE_DIR:PATH=${CUDA_CUT_INCLUDE_DIR}
      -DNIFTK_USE_FFTW:BOOL=${NIFTK_USE_FFTW}
      -DNIFTK_USE_CUDA:BOOL=${NIFTK_USE_CUDA}
      -DNIFTK_BUILD_ALL_APPS:BOOL=${NIFTK_BUILD_ALL_APPS}
      -DNIFTK_WITHIN_SUPERBUILD:BOOL=ON                    # Set this to ON, as some compilation flags rely on knowing if we are doing superbuild.
      -DNIFTK_VERSION_MAJOR:STRING=${NIFTK_VERSION_MAJOR}
      -DNIFTK_VERSION_MINOR:STRING=${NIFTK_VERSION_MINOR}
      -DNIFTK_PLATFORM:STRING=${NIFTK_PLATFORM}
      -DNIFTK_COPYRIGHT:STRING=${NIFTK_COPYRIGHT}
      -DNIFTK_ORIGIN_URL:STRING=${NIFTK_ORIGIN_URL}
      -DNIFTK_ORIGIN_SHORT_TEXT:STRING=${NIFTK_ORIGIN_SHORT_TEXT}
      -DNIFTK_ORIGIN_LONG_TEXT:STRING=${NIFTK_ORIGIN_LONG_TEXT}
      -DNIFTK_WIKI_URL:STRING=${NIFTK_WIKI_URL}
      -DNIFTK_WIKI_TEXT:STRING=${NIFTK_WIKI_TEXT}
      -DNIFTK_DASHBOARD_URL:STRING=${NIFTK_DASHBOARD_URL}
      -DNIFTK_DASHBOARD_TEXT:STRING=${NIFTK_DASHBOARD_TEXT}
      -DNIFTK_USER_CONTACT:STRING=${NIFTK_USER_CONTACT}
      -DNIFTK_DEVELOPER_CONTACT:STRING=${NIFTK_DEVELOPER_CONTACT}
      -DNIFTK_BASE_NAME:STRING=${NIFTK_BASE_NAME}
      -DNIFTK_VERSION_STRING:STRING=${NIFTK_VERSION_STRING}
      -DNIFTK_GENERATE_DOXYGEN_HELP:BOOL=${NIFTK_GENERATE_DOXYGEN_HELP}
      -DNIFTK_VERBOSE_COMPILER_WARNINGS:BOOL=${NIFTK_VERBOSE_COMPILER_WARNINGS}
      -DNIFTK_CHECK_COVERAGE:BOOL=${NIFTK_CHECK_COVERAGE}
      -DNIFTK_ADDITIONAL_C_FLAGS:STRING=${NIFTK_ADDITIONAL_C_FLAGS}
      -DNIFTK_ADDITIONAL_CXX_FLAGS:STRING=${NIFTK_ADDITIONAL_CXX_FLAGS}
      -DNIFTK_FFTWINSTALL:PATH=${NIFTK_LINK_PREFIX}/fftw     # We don't have CMake SuperBuild version of FFTW, so must rely on it already being there
      -DNIFTK_DATA_DIR:PATH=${NIFTK_DATA_DIR}
      -DVTK_DIR:PATH=${VTK_DIR}
      -DITK_DIR:PATH=${ITK_DIR}
      -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
      -DBOOST_ROOT:PATH=${BOOST_ROOT}
      -DBOOST_VERSION:STRING=${NIFTK_VERSION_BOOST}
      -DBOOST_INCLUDEDIR:PATH=${BOOST_INCLUDEDIR}
      -DBOOST_LIBRARYDIR:PATH=${BOOST_LIBRARYDIR}
      -DMITK_DIR:PATH=${MITK_DIR}
      -DCTK_DIR:PATH=${CTK_DIR}
      -DCTK_SOURCE_DIR:PATH=${CTK_SOURCE_DIR}
      -DNiftyLink_DIR:PATH=${NiftyLink_DIR}
      -DNiftyLink_SOURCE_DIR:PATH=${NiftyLink_SOURCE_DIR}
      -DNIFTYREG_DIR:PATH=${EP_BASE}/Install/NIFTYREG
      -DNIFTYREC_DIR:PATH=${EP_BASE}/Install/NIFTYREC
      -DNIFTYSIM_DIR:PATH=${EP_BASE}/Install/NIFTYSIM
      -DNIFTYSEG_DIR:PATH=${EP_BASE}/Install/NIFTYSEG
	  -DCURL_DIR:PATH=${EP_BASE}/Install/curl
      DEPENDS ${proj_DEPENDENCIES}
  )

ENDIF()
