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

#include <mitkVideoTrackerMatching.h>
#include <niftkGetMatchedTrackingMatrixCLP.h>

int main(int argc, char** argv)
{
  PARSE_ARGS;
  int returnStatus = EXIT_FAILURE;

  if ( trackingInputDirectory.length() == 0 )
  {
    std::cout << trackingInputDirectory.length() << std::endl;
    commandLine.getOutput()->usage(commandLine);
    return returnStatus;
  }

  try
  {
    mitk::VideoTrackerMatching::Pointer trackerMatcherObject = mitk::VideoTrackerMatching::New();
    trackerMatcherObject->SetFlipMatrices(FlipTracking);
    trackerMatcherObject->Initialise(trackingInputDirectory);
    if ( handeyes.length() !=0 ) 
    {
      trackerMatcherObject->SetCameraToTrackers(handeyes);
    }

    long long* timingError = new long long;
    std::cout << trackerMatcherObject->GetTrackerMatrix(frameNumber, timingError, trackerIndex);

    returnStatus = EXIT_SUCCESS;
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
