/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <mitkTestingMacros.h>
#include <mitkLogMacros.h>
#include <mitkHandeyeCalibrate.h>

/**
 * Runs ICP registration a known data set and checks the error
 */

int mitkHandeyeCalibrationTest ( int argc, char * argv[] )
{
  std::string inputExtrinsic = argv[1];
  std::string inputTracking = argv[2];

  mitk::HandeyeCalibrate::Pointer Calibrator = mitk::HandeyeCalibrate::New();
  
  std::vector<cv::Mat> ExtMatrices = Calibrator->LoadMatricesFromExtrinsicFile(inputExtrinsic);
  std::vector<cv::Mat> TrackMatrices = Calibrator->LoadMatricesFromDirectory(inputTracking);

  std::vector<cv::Mat> FlippedTrackMatrices = Calibrator->FlipMatrices(TrackMatrices);

/*  for ( unsigned int i = 0 ; i < TrackMatrices.size() ; i ++ )
  {
    std::cout << "Extrinsic Matrix Number " << i << std::endl;
    for ( int row = 0 ; row < 4 ; row ++ )
    {
      for ( int col = 0 ; col < 4 ; col ++ )
      {
        std::cout << ExtMatrices[i].at<double>(row,col) << " ";
      }
      std::cout << std::endl;
    }
    std::cout << "Tracking: "<< std::endl;
    for ( int row = 0 ; row < 4 ; row ++ )
    {
      for ( int col = 0 ; col < 4 ; col ++ )
      {
        std::cout << FlippedTrackMatrices[i].at<double>(row,col) << " ";
      }
      std::cout << std::endl;
    }

      std::cout << std::endl;
  }*/

  cv::Mat CamToMarker = cvCreateMat(4,4,CV_32FC1);
  Calibrator->Calibrate( FlippedTrackMatrices, ExtMatrices, CamToMarker);
  return EXIT_SUCCESS;
}
