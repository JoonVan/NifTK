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


#-----------------------------------------------------------------------------
# PCL - Point Cloud Library..
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED PCL_DIR AND NOT EXISTS ${PCL_DIR})
  message(FATAL_ERROR "PCL_DIR variable is defined but corresponds to non-existing directory \"${PCL_DIR}\".")
endif()

if(BUILD_IGI AND BUILD_PCL)

  niftkMacroGetCommitHashOfCurrentFile(config_version)

  set(proj PCL)
  set(proj_VERSION ${NIFTK_VERSION_${proj}})
  set(proj_SOURCE ${EP_BASE}/${proj}-${proj_VERSION}-${config_version}-src)
  set(proj_CONFIG ${EP_BASE}/${proj}-${proj_VERSION}-${config_version}-cmake)
  set(proj_BUILD ${EP_BASE}/${proj}-${proj_VERSION}-${config_version}-build)
  set(proj_INSTALL ${EP_BASE}/${proj}-${proj_VERSION}-${config_version}-install)
  set(proj_DEPENDENCIES Boost Eigen FLANN VTK)
  set(PCL_DEPENDS ${proj})

  if(NOT DEFINED PCL_DIR)

    niftkMacroGetChecksum(NIFTK_CHECKSUM_PCL ${NIFTK_LOCATION_PCL})

    ExternalProject_Add(${proj}
      SOURCE_DIR ${proj_SOURCE}
      PREFIX ${proj_CONFIG}
      BINARY_DIR ${proj_BUILD}
      INSTALL_DIR ${proj_INSTALL}
      URL ${NIFTK_LOCATION_PCL}
      URL_MD5 ${NIFTK_CHECKSUM_PCL}
      UPDATE_COMMAND  ${GIT_EXECUTABLE} checkout ${proj_VERSION}
      CMAKE_GENERATOR ${GEN}
      CMAKE_ARGS
        ${EP_COMMON_ARGS}
        -DCMAKE_INSTALL_PREFIX:PATH=${proj_INSTALL}
        -DBOOST_ROOT:PATH=${BOOST_ROOT}
        -DBOOST_INCLUDEDIR:PATH=${BOOST_ROOT}/include
        -DBOOST_LIBRARYDIR:PATH=${BOOST_ROOT}/lib
        -DBoost_NO_SYSTEM_PATHS:BOOL=ON
        -DEIGEN_ROOT:PATH=${Eigen_DIR}
        -DFLANN_ROOT:PATH=${FLANN_DIR}
        -DVTK_DIR:PATH=${VTK_DIR}
        # explicitly define this (with the default value) because pcl will try to use static libs otherwise
        -DBoost_USE_STATIC_LIBS:BOOL=${Boost_USE_STATIC_LIBS}
        -DPCL_BUILD_WITH_BOOST_DYNAMIC_LINKING_WIN32:BOOL=NOT ${Boost_USE_STATIC_LIBS}
        -DBUILD_tools:BOOL=OFF
      DEPENDS ${proj_DEPENDENCIES}
    )
    if(WIN32)
      set(PCL_DIR ${proj_INSTALL}/cmake)
    else()
      set(PCL_DIR ${proj_INSTALL}/share/pcl-1.8)
    endif()
    message("SuperBuild loading PCL from ${PCL_DIR}")

  else(NOT DEFINED PCL_DIR)

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif(NOT DEFINED PCL_DIR)

endif()
