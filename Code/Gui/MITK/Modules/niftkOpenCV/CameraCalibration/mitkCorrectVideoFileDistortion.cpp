/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkCorrectVideoFileDistortion.h"
#include "mitkCameraCalibrationFacade.h"
#include <ios>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cv.h>
#include <highgui.h>
#include "FileHelper.h"

namespace mitk {

//-----------------------------------------------------------------------------
CorrectVideoFileDistortion::CorrectVideoFileDistortion()
{

}


//-----------------------------------------------------------------------------
CorrectVideoFileDistortion::~CorrectVideoFileDistortion()
{

}


//-----------------------------------------------------------------------------
bool CorrectVideoFileDistortion::Correct(
    const std::string& inputImageFileName,
    const std::string& inputIntrinsicsFileNameLeft,
    const std::string& inputDistortionCoefficientsFileNameLeft,
    const std::string& inputIntrinsicsFileNameRight,
    const std::string& inputDistortionCoefficientsFileNameRight,
    const std::string& outputImageFileName,
    bool writeInterleaved
    )
{
  bool isSuccessful = false;

  try
  {
    CorrectDistortionInStereoVideoFile(
        inputImageFileName,
        inputIntrinsicsFileNameLeft,
        inputDistortionCoefficientsFileNameLeft,
        inputIntrinsicsFileNameRight,
        inputDistortionCoefficientsFileNameRight,
        writeInterleaved,
        outputImageFileName);

    // No exceptions ... so all OK.
    isSuccessful = true;
  }
  catch(std::logic_error e)
  {
    std::cerr << "CorrectVideoFileDistortion::Correct: exception thrown e=" << e.what() << std::endl;
  }

  return isSuccessful;
}

} // end namespace
