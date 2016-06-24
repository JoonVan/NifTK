/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkVideoTrackerMatching.h"
#include <mitkTestingMacros.h>
#include <mitkLogMacros.h>
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>

bool CheckTrackerMatrix (cv::Mat matrix)
{
  double Error = 0.0;

  Error += fabs ( matrix.at<double >(0,0) - ( 0.0536763780 ));
  Error += fabs ( matrix.at<double>(0,1) - ( -0.9976278543 ));
  Error += fabs ( matrix.at<double>(0,2) - ( 0.0430984311 ));
  Error += fabs ( matrix.at<double>(0,3) - ( 261.1099853516 ));

  Error += fabs ( matrix.at<double>(1,0) - ( 0.1872890294 ));
  Error += fabs ( matrix.at<double>(1,1) - ( -0.0323365629 ));
  Error += fabs ( matrix.at<double>(1,2) - ( -0.9817724824 ));
  Error += fabs ( matrix.at<double>(1,3) - (-106.7399978638 ));
  
  Error += fabs ( matrix.at<double>(2,0) - ( 0.9808372259 ));
  Error += fabs ( matrix.at<double>(2,1) - ( 0.0607698523 ));
  Error += fabs ( matrix.at<double>(2,2) - ( 0.1851090491 ));
  Error += fabs ( matrix.at<double>(2,3) - ( -1934.9799804688 ));
  
  Error += fabs ( matrix.at<double>(3,0) - ( 0.0 ));
  Error += fabs ( matrix.at<double>(3,1) - ( 0.0 ));
  Error += fabs ( matrix.at<double>(3,2) - ( 0.0 ));
  Error += fabs ( matrix.at<double>(3,3) - ( 1.0 ));
  if ( Error < 1e-6 ) 
  {
    return true;
  }
  else
  {
    return false;
  }
}


//-----------------------------------------------------------------------------
int mitkVideoTrackerMatchingTest(int argc, char * argv[])
{
  mitk::VideoTrackerMatching::Pointer matcher = mitk::VideoTrackerMatching::New();
  matcher->Initialise(argv[1]);

  //check it initialised, check it gets the right matrix with the right time error
  MITK_TEST_CONDITION_REQUIRED (matcher->IsReady() , "Testing that VideoTrackerMatcherInitialised OK");
  
  long long *timingError = new long long;
  cv::Mat trackingMatrix = matcher->GetTrackerMatrix(20, timingError, 1);

  //1374066239681720400-1374066239683720400 = -2000000
  //1374854436963960800-1374854436966961200 = -3000400
  MITK_TEST_CONDITION_REQUIRED(*timingError == -3000400, "Testing Timing error");
  MITK_TEST_CONDITION_REQUIRED(CheckTrackerMatrix(trackingMatrix), "Testing Tracker Matrix");
  matcher->SetVideoLagMilliseconds(50, true, 1);
  trackingMatrix = matcher->GetTrackerMatrix(20, timingError, 1);
  //1374854436963960800 + 50000000 - 1374854436998962800 = 14998000
  MITK_TEST_CONDITION_REQUIRED(*timingError == 14998000, "Testing Positive Timing error");
  MITK_TEST_CONDITION_REQUIRED(CheckTrackerMatrix(trackingMatrix), "Testing Tracker Matrix Again");
  return EXIT_SUCCESS;
}
