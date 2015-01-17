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
# RTK
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED RTK_DIR AND NOT EXISTS ${RTK_DIR})
  message(FATAL_ERROR "RTK_DIR variable is defined but corresponds to non-existing directory \"${RTK_DIR}\".")
endif()

if(BUILD_RTK)

  set(proj RTK)
  set(proj_VERSION ${NIFTK_VERSION_${proj}})
  set(proj_SOURCE ${EP_BASE}/${proj}-${proj_VERSION}-src)
  set(proj_CONFIG ${EP_BASE}/${proj}-${proj_VERSION}-cmake)
  set(proj_BUILD ${EP_BASE}/${proj}-${proj_VERSION}-build)
  set(proj_INSTALL ${EP_BASE}/${proj}-${proj_VERSION}-install)
  set(proj_DEPENDENCIES GDCM ITK)
  set(RTK_DEPENDS ${proj})

  if(NOT DEFINED RTK_DIR)

    set(additional_cmake_args )
    niftkMacroGetChecksum(NIFTK_CHECKSUM_RTK ${NIFTK_LOCATION_RTK})

    ExternalProject_Add(${proj}
      SOURCE_DIR ${proj_SOURCE}
      PREFIX ${proj_CONFIG}
      BINARY_DIR ${proj_BUILD}
      INSTALL_DIR ${proj_INSTALL}
      URL ${NIFTK_LOCATION_RTK}
      URL_MD5 ${NIFTK_CHECKSUM_RTK}
      UPDATE_COMMAND  ${GIT_EXECUTABLE} checkout ${proj_VERSION}
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${GEN}
      CMAKE_ARGS
        ${EP_COMMON_ARGS}
        ${additional_cmake_args}
        -DITK_DIR:PATH=${ITK_DIR}
        -DGDCM_DIR:PATH=${GDCM_DIR}
        -DCMAKE_SHARED_LINKER_FLAGS:STRING=-L${GDCM_DIR}/bin
        -DCMAKE_EXE_LINKER_FLAGS:STRING=-L${GDCM_DIR}/bin
        -DBUILD_SHARED_LIBS:BOOL=OFF
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(RTK_DIR ${proj_BUILD})
    message("SuperBuild loading RTK from ${RTK_DIR}")

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif()
