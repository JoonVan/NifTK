/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <mitkVector.h>
#include <mitkExceptionMacro.h>

#include <niftkUltrasoundProcessing.h>
#include <niftkUltrasoundReconstructionCLP.h>

int main(int argc, char** argv)
{
  PARSE_ARGS;
  int returnStatus = EXIT_FAILURE;

  if (    matrixDirectory.length() == 0
       || imageDirectory.length() == 0
       || rigidMatrixFile.length() == 0
       || scalingMatrixFile.length() == 0
       )
  {
    commandLine.getOutput()->usage(commandLine);
    return returnStatus;
  }

  try
  {
    returnStatus = EXIT_SUCCESS;
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << "Caught mitk::Exception: " << e.GetDescription() << ", from:" << e.GetFile() << "::" << e.GetLine() << std::endl;
    returnStatus = EXIT_FAILURE + 1;
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Caught std::exception: " << e.what() << std::endl;
    returnStatus = EXIT_FAILURE + 2;
  }
  catch (...)
  {
    MITK_ERROR << "Caught unknown exception:" << std::endl;
    returnStatus = EXIT_FAILURE + 3;
  }

  return returnStatus;
}
