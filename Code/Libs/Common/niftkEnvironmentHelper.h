/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkEnvironmentHelper_h
#define niftkEnvironmentHelper_h

#include <NifTKConfigure.h>
#include "niftkCommonWin32ExportHeader.h"

#include <string>

namespace niftk
{

#define NIFTK_DIR "NIFTK_DIR"

#define USERS_HOME "HOME"

#define WORKING_DIR "PWD"

NIFTKCOMMON_WINEXPORT std::string GetHomeDirectory();

NIFTKCOMMON_WINEXPORT std::string GetWorkingDirectory();

NIFTKCOMMON_WINEXPORT std::string GetNIFTKHome();

NIFTKCOMMON_WINEXPORT std::string GetEnvironmentVariable(const std::string& variableName);

NIFTKCOMMON_WINEXPORT bool BooleanEnvironmentVariableIsOn(const std::string& variableName);

NIFTKCOMMON_WINEXPORT bool BooleanEnvironmentVariableIsOff(const std::string& variableName);

} // end namespace

#endif
