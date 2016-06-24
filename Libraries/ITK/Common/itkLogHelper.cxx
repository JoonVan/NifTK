/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <iostream>

#include "itkLogHelper.h"
#include "itkUCLMacro.h"
#include <niftkConversionUtils.h>

namespace niftk
{
  itkLogHelper::itkLogHelper()
  {
  }

  void itkLogHelper::PrintSelf(std::ostream &os, itk::Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

  itkLogHelper::~itkLogHelper()
  {
  }

  std::string itkLogHelper::ToString()
  {
    // No member variables yet, nothing to report.
    return "itkLogHelper[]";
  }

  void itkLogHelper::InfoMessage(const std::string& infoMessage)
  {
	  niftkitkInfoMacro(<< infoMessage);
  }

  void itkLogHelper::DebugMessage(const std::string& debugMessage)
  {
	  niftkitkDebugMacro(<< debugMessage);
  }

  void itkLogHelper::WarningMessage(const std::string& warningMessage)
  {
	  niftkitkWarningMacro(<< warningMessage);
  }

  void itkLogHelper::ErrorMessage(const std::string& errorMessage)
  {
	  niftkitkErrorMacro(<< errorMessage);
  }

  void itkLogHelper::ExceptionMessage(const std::string& exceptionMessage)
  {
	  niftkitkExceptionMacro(<< exceptionMessage);
  }

  std::string itkLogHelper::WriteParameterArray(const itk::Array<double>& array)
  {
    std::string result = std::string("array ");

    if (array.GetSize() < 20)
    {
      result += " contains[";
      for (unsigned int i = 0; i < array.GetSize(); i++)
      {
        result += (niftk::ConvertToString(array[i]) + ", ");
      }
      result += "]";
    }
    else
    {
      result += " is of size " + niftk::ConvertToString(((int)array.GetSize()));
    }

    return result;
  }

  void itkLogHelper::PrintCommandLineHeader(std::ostream& stream)
  {
    stream << NIFTK_COPYRIGHT << std::endl;
    stream << NIFTK_PLATFORM << ", " << NIFTK_VERSION_STRING << std::endl;
  }

}
