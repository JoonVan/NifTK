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

#include <mitkTwoTrackerAnalysis.h>
#include <niftkTwoTrackerAnalysisCLP.h>

int main(int argc, char** argv)
{
  PARSE_ARGS;
  int returnStatus = EXIT_FAILURE;

  if ( Directory1.length() == 0 || Directory2.length() == 0 )
  {
    std::cout << Directory1.length() << std::endl;
    commandLine.getOutput()->usage(commandLine);
    return returnStatus;
  }

  try
  {
    mitk::TwoTrackerAnalysis::Pointer trackerMatcherObject = mitk::TwoTrackerAnalysis::New();
    trackerMatcherObject->SetTimingTolerance(MaxTimingError * 1e6);
    if ( CullOnDistance ) 
    {
      MITK_INFO << "do something";
    }
    trackerMatcherObject->Initialise(Directory1, Directory2);
    //check it's a rigid body first
    if ( FlipDir1 ) 
    {
      trackerMatcherObject->FlipMats1();
    }
    if ( FlipDir2 )
    {
      trackerMatcherObject->FlipMats2();
    }
    if ( TCfileout.length() != 0 )
    {
      trackerMatcherObject->TemporalCalibration(temporalWindowLow, temporalWindowHigh, true, TCfileout);
    }
    if ( HEfileout.length() != 0 ) 
    {
      trackerMatcherObject->HandeyeCalibration( false, HEfileout, MatricesToUse, CullOnDistance);
      if ( CullOnDistance )
      {
        trackerMatcherObject->HandeyeCalibration( false, HEfileout, MatricesToUse, false);
      }
    }
 
    returnStatus = EXIT_SUCCESS;
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << "Caught mitk::Exception: " << e.GetDescription() << ", from:" << e.GetFile() << "::" << e.GetLine() << std::endl;
    returnStatus = EXIT_FAILURE + 100;
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Caught std::exception: " << e.what() << std::endl;
    returnStatus = EXIT_FAILURE + 101;
  }
  catch (...)
  {
    MITK_ERROR << "Caught unknown exception:" << std::endl;
    returnStatus = EXIT_FAILURE + 102;
  }
  return returnStatus;
}
