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

MITK_CREATE_MODULE(
  INCLUDE_DIRS CUDAManager CUDAImage Image CameraCalibration
  DEPENDS MitkCore
  PACKAGE_DEPENDS Qt4|QtCore Qt4|QtOpenGL Qt5|Core Qt5|OpenGL
)

include_directories(
  ${CUDA_TOOLKIT_INCLUDE}
  ${CMAKE_SOURCE_DIR}/Libraries/CUDAKernels
)

target_link_libraries(niftkCUDA
  PRIVATE
    ${CUDA_CUDA_LIBRARY}
    ${CUDA_CUDART_LIBRARY}
    niftkCUDAKernels
)


if(MSVC AND NIFTK_DELAYLOAD_CUDA)
  # all of the nvidia-driver/cuda dependencies should be delay-loaded
  #  so this library would at least load on a machine that doesnt have nv hardware
  set_target_properties(niftkCUDA PROPERTIES LINK_FLAGS 
    "/DELAYLOAD:nvcuda.dll /DELAYLOAD:${CUDA_CUDART_DLL_NAME}")
endif()

if (NIFTK_USE_COTIRE AND COMMAND cotire)
  cotire(niftkCUDA)
endif()

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
