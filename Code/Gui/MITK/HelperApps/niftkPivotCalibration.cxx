/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <cstdlib>
#include <limits>
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <mitkVector.h>

#include <niftkVTKFunctions.h>
#include <mitkPivotCalibration.h>
#include <niftkPivotCalibrationCLP.h>

int main(int argc, char** argv)
{
  PARSE_ARGS;
  int returnStatus = EXIT_FAILURE;
  double residualError = std::numeric_limits<double>::max();

  if ( matrixDirectory.length() == 0)
  {
    commandLine.getOutput()->usage(commandLine);
    return returnStatus;
  }

  try
  {
    std::cout << "niftkPivotCalibration: matrices         = " << matrixDirectory << std::endl;

    vtkSmartPointer<vtkMatrix4x4> transformationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

    // Do calibration
    mitk::PivotCalibration::Pointer calibration = mitk::PivotCalibration::New();
    calibration->CalibrateUsingFilesInDirectories(
      matrixDirectory,
      residualError,
      *transformationMatrix,
      percentage,
      numberOfReruns
      );

    if (niftk::SaveMatrix4x4ToFile(outputMatrixFile, *transformationMatrix))
    {
      returnStatus = EXIT_SUCCESS;
    }
    else
    {
      MITK_ERROR << "Failed to save transformation to file:" << outputMatrixFile << std::endl;
      returnStatus = EXIT_FAILURE;
    }
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Caught std::exception:" << e.what();
    returnStatus = -1;
  }
  catch (...)
  {
    MITK_ERROR << "Caught unknown exception:";
    returnStatus = -2;
  }

  return returnStatus;
}
