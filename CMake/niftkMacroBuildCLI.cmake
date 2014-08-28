#/*================================================================================
#
# All or portions of this licensed product (such portions are the "Software") have 
# been obtained under license from The Brigham and Women's Hospital, Inc. and are 
# subject to the following terms and conditions:
#
# Copyright (c) Kitware Inc.
#
# See http://www.slicer.org/copyright/copyright.txt for details.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# For NifTK: This file was taken from the Slicer Execution Model project on GitHub.
#
# See here:
# https://github.com/Slicer/SlicerExecutionModel
# https://github.com/Slicer/SlicerExecutionModel/blob/master/CMake/SEMMacroBuildCLI.cmake
# hashtag: 7365853e2b
#
# The function was modified as follows:
# 1. Name changed from SEMMacroBuildCLI to SEMMacroBuildNifTKCLI to avoid confusion.
# 2. Removed parameters
#    RUNTIME_OUTPUT_DIRECTORY
#    LIBRARY_OUTPUT_DIRECTORY
#    ARCHIVE_OUTPUT_DIRECTORY
#    INSTALL_RUNTIME_DESTINATION
#    INSTALL_LIBRARY_DESTINATION
#    INSTALL_ARCHIVE_DESTINATION
# 3. Called the MITK_INSTALL macro instead of INSTALL macro.
# 4. Added all the header comments.
#/*================================================================================
#
#/*================================================================================
#
#  NifTK: A platform for combined medical image analysis and image guided surgery.
#  
#  Copyright (c) University College London (UCL). All rights reserved.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  
#
#  See LICENSE.txt in the top level directory for details. 
#
#=================================================================================*/

#
# Depends on:
#  CMakeParseArguments.cmake from Cmake 2.8.4 or greater
#
if(CMAKE_VERSION VERSION_LESS 2.8.4)
  include(${SlicerExecutionModel_CMAKE_DIR}/Pre283CMakeParseArguments.cmake)
else()
  include(CMakeParseArguments)
endif()

macro(SEMMacroBuildNifTKCLI)
  set(options
    EXECUTABLE_ONLY
    NO_INSTALL VERBOSE
    )
  set(oneValueArgs
    NAME LOGO_HEADER
    CLI_XML_FILE
    CLI_LIBRARY_WRAPPER_CXX
    CLI_SHARED_LIBRARY_WRAPPER_CXX # Deprecated
    )
  set(multiValueArgs
    ADDITIONAL_SRCS
    TARGET_LIBRARIES
    LINK_DIRECTORIES
    INCLUDE_DIRECTORIES
    )
  CMAKE_PARSE_ARGUMENTS(LOCAL_SEM
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  message(STATUS "Configuring SEM CLI module: ${LOCAL_SEM_NAME}")
  # --------------------------------------------------------------------------
  # Print information helpful for debugging checks
  # --------------------------------------------------------------------------
  if(LOCAL_SEM_VERBOSE)
    list(APPEND ALL_OPTIONS ${options} ${oneValueArgs} ${multiValueArgs})
    foreach(curr_opt ${ALL_OPTIONS})
      message(STATUS "${curr_opt} = ${LOCAL_SEM_${curr_opt}}")
    endforeach()
  endif()
  if(LOCAL_SEM_INSTALL_UNPARSED_ARGUMENTS)
    message(AUTHOR_WARNING "Unparsed arguments given [${LOCAL_SEM_INSTALL_UNPARSED_ARGUMENTS}]")
  endif()
  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(NOT DEFINED LOCAL_SEM_NAME)
    message(FATAL_ERROR "NAME is mandatory: [${LOCAL_SEM_NAME}]")
  endif()

  if(DEFINED LOCAL_SEM_LOGO_HEADER AND NOT EXISTS ${LOCAL_SEM_LOGO_HEADER})
    message(AUTHOR_WARNING "Specified LOGO_HEADER [${LOCAL_SEM_LOGO_HEADER}] doesn't exist")
    set(LOCAL_SEM_LOGO_HEADER)
  endif()

  if(DEFINED LOCAL_SEM_CLI_SHARED_LIBRARY_WRAPPER_CXX)
    message(AUTHOR_WARNING "Parameter 'CLI_SHARED_LIBRARY_WRAPPER_CXX' is deprecated. Use 'CLI_LIBRARY_WRAPPER_CXX' instead.")
    set(LOCAL_SEM_CLI_LIBRARY_WRAPPER_CXX ${LOCAL_SEM_CLI_SHARED_LIBRARY_WRAPPER_CXX})
  endif()

  # Use default value if it applies
  if(NOT DEFINED LOCAL_SEM_CLI_LIBRARY_WRAPPER_CXX)
    set(LOCAL_SEM_CLI_LIBRARY_WRAPPER_CXX ${SlicerExecutionModel_DEFAULT_CLI_LIBRARY_WRAPPER_CXX})
  endif()

  foreach(v LOCAL_SEM_CLI_LIBRARY_WRAPPER_CXX)
    if(NOT EXISTS "${${v}}")
      message(FATAL_ERROR "Variable ${v} point to an non-existing file or directory !")
    endif()
  endforeach()

  if(DEFINED LOCAL_SEM_CLI_XML_FILE)
    set(cli_xml_file ${LOCAL_SEM_CLI_XML_FILE})
    if(NOT EXISTS ${cli_xml_file})
      message(FATAL_ERROR "Requested XML file [${cli_xml_file}] specified using CLI_XML_FILE doesn't exist !")
    endif()
  else()
    set(cli_xml_file ${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_SEM_NAME}.xml)
    if(NOT EXISTS ${cli_xml_file})
      set(cli_xml_file ${CMAKE_CURRENT_BINARY_DIR}/${LOCAL_SEM_NAME}.xml)
      if(NOT EXISTS ${cli_xml_file})
        message(FATAL_ERROR "Couldn't find XML file [${LOCAL_SEM_NAME}.xml] in either the current source directory [${CMAKE_CURRENT_SOURCE_DIR}] or the current build directory [${CMAKE_CURRENT_BINARY_DIR}] - Note that you could also specify a custom location using CLI_XML_FILE parameter !")
      endif()
    endif()
  endif()

  set(CLP ${LOCAL_SEM_NAME})

  # SlicerExecutionModel
  find_package(SlicerExecutionModel REQUIRED GenerateCLP)
  include(${GenerateCLP_USE_FILE})

  set(${CLP}_SOURCE ${CLP}.cxx ${LOCAL_SEM_ADDITIONAL_SRCS})
  generateclp(${CLP}_SOURCE ${cli_xml_file} ${LOCAL_SEM_LOGO_HEADER})

  if(DEFINED LOCAL_SEM_LINK_DIRECTORIES)
    link_directories(${LOCAL_SEM_LINK_DIRECTORIES})
  endif()

  if(DEFINED LOCAL_SEM_INCLUDE_DIRECTORIES)
    include_directories(${LOCAL_SEM_INCLUDE_DIRECTORIES})
  endif()
  
  if(DEFINED SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES)
    include_directories(${SlicerExecutionModel_EXTRA_INCLUDE_DIRECTORIES})
  endif()

  set(cli_targets)

  if(NOT LOCAL_SEM_EXECUTABLE_ONLY)

    add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
    set_target_properties(${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
    if(DEFINED LOCAL_SEM_TARGET_LIBRARIES)
      target_link_libraries(${CLP}Lib ${LOCAL_SEM_TARGET_LIBRARIES})
    endif()

    add_executable(${CLP} ${LOCAL_SEM_CLI_LIBRARY_WRAPPER_CXX})
    target_link_libraries(${CLP} ${CLP}Lib)

    set(cli_targets ${CLP} ${CLP}Lib)

  else()

    add_executable(${CLP} ${${CLP}_SOURCE})
    if(DEFINED LOCAL_SEM_TARGET_LIBRARIES)
      target_link_libraries(${CLP} ${LOCAL_SEM_TARGET_LIBRARIES})
    endif()

    set(cli_targets ${CLP})

  endif()

  # Set labels associated with the target.
  set_target_properties(${cli_targets} PROPERTIES LABELS ${CLP})
  
  if(NOT LOCAL_SEM_NO_INSTALL)
    MITK_INSTALL(TARGETS ${cli_targets})
  endif()

endmacro()

