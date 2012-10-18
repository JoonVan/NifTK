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
#  Last Changed      : $LastChangedDate: 2011-12-17 14:35:07 +0000 (Sat, 17 Dec 2011) $ 
#  Revision          : $Revision: 8065 $
#  Last modified by  : $Author: mjc $
#
#  Original author   : j.hipwell@ucl.ac.uk
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notices for more information.
#
#=================================================================================*/

#-----------------------------------------------------------------------------
# NIFTYSEG
#-----------------------------------------------------------------------------

# Sanity checks
IF(DEFINED NIFTYSEG_ROOT AND NOT EXISTS ${NIFTYSEG_ROOT})
  MESSAGE(FATAL_ERROR "NIFTYSEG_ROOT variable is defined but corresponds to non-existing disegtory \"${NIFTYSEG_ROOT}\".")
ENDIF()

IF(BUILD_NIFTYSEG)

  SET(proj NIFTYSEG)
  SET(proj_DEPENDENCIES )
  SET(proj_INSTALL ${EP_BASE}/Install/${proj} )
  SET(NIFTYSEG_DEPENDS ${proj})

  IF(NOT DEFINED NIFTYSEG_ROOT)

    niftkMacroGetChecksum(NIFTK_CHECKSUM_NIFTYSEG ${NIFTK_LOCATION_NIFTYSEG})

    ExternalProject_Add(${proj}
      URL ${NIFTK_LOCATION_NIFTYSEG}
      URL_MD5 ${NIFTK_CHECKSUM_NIFTYSEG}
      CMAKE_GENERATOR ${GEN}
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DUSE_CUDA:BOOL=${NIFTK_USE_CUDA}
        -DINSTALL_PRIORS:BOOL=ON
        -DINSTALL_PRIORS_DIRECTORY:PATH=${EP_BASE}/Install/${proj}/priors
        -DCMAKE_INSTALL_PREFIX:PATH=${proj_INSTALL}
      DEPENDS ${proj_DEPENDENCIES}
      )

    SET(NIFTYSEG_ROOT ${proj_INSTALL})
    SET(NIFTYSEG_INCLUDE_DIR "${NIFTYSEG_ROOT}/include")
    SET(NIFTYSEG_LIBRARY_DIR "${NIFTYSEG_ROOT}/lib")

    MESSAGE("SuperBuild loading NIFTYSEG from ${NIFTYSEG_ROOT}")

  ELSE(NOT DEFINED NIFTYSEG_ROOT)

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  ENDIF(NOT DEFINED NIFTYSEG_ROOT)

ENDIF(BUILD_NIFTYSEG)
