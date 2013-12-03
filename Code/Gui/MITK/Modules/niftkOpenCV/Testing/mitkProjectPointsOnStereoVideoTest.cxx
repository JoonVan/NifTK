/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkProjectPointsOnStereoVideo.h"
#include <mitkTestingMacros.h>
#include <mitkLogMacros.h>
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>

bool CheckTransformedPointVector (std::vector < std::vector <cv::Point3d> > points)
{
  double Error = 0.0;
  //here are some points calculated indepenently
  std::vector <cv::Point3d> frame0000points;
  frame0000points.push_back(cv::Point3d(35.8970  ,  36.3999 ,  124.6265));
  frame0000points.push_back(cv::Point3d(76.4005  ,  40.0814 ,  116.2794));
  frame0000points.push_back(cv::Point3d(73.0312 ,   69.5444 ,  110.9602));
  frame0000points.push_back(cv::Point3d(32.5277 ,   65.8629 ,  119.3073));
  std::vector <cv::Point3d> frame1155points;
  frame1155points.push_back(cv::Point3d(41.3955 ,   38.0281 ,  123.3948));
  frame1155points.push_back(cv::Point3d(82.2175 ,   36.2113 ,  116.0442));
  frame1155points.push_back(cv::Point3d(82.9025 ,   65.7807  , 110.3089 ));
  frame1155points.push_back(cv::Point3d(42.0805  ,  67.5975 ,  117.6595));
  
  for ( int i = 0 ; i < 4 ; i ++ ) 
  {
    Error += fabs ( points[0][i].x - frame0000points[i].x);
    Error += fabs ( points[0][i].y - frame0000points[i].y);
    Error += fabs ( points[0][i].z - frame0000points[i].z);
    //MITK_INFO << Error;

    Error += fabs ( points[1155][i].x - frame1155points[i].x);
    Error += fabs ( points[1155][i].y - frame1155points[i].y);
    Error += fabs ( points[1155][i].z - frame1155points[i].z);
    //MITK_INFO << Error;
  }


  if ( Error < 2e-3 ) 
  {
    return true;
  }
  else
  {
    return false;
  }
}


//-----------------------------------------------------------------------------
int mitkProjectPointsOnStereoVideoTest(int argc, char * argv[])
{
  mitk::ProjectPointsOnStereoVideo::Pointer Projector = mitk::ProjectPointsOnStereoVideo::New();
  Projector->SetVisualise(true);
  Projector->Initialise(argv[1], argv[2]);
  Projector->SetTrackerIndex(2);
  Projector->SetDrawAxes(true);
  mitk::VideoTrackerMatching::Pointer matcher = mitk::VideoTrackerMatching::New();
  matcher->Initialise(argv[1]);
  matcher->SetFlipMatrices(false);
  Projector->SetMatcherCameraToTracker(matcher);
  //check it initialised, check it gets the right matrix with the right time error
  MITK_TEST_CONDITION_REQUIRED (Projector->GetInitOK() , "Testing mitkProjectPointsOnStereoVideo Initialised OK"); 

  //here are the on screen points manually found in frames 0 and 1155
  std::vector < std::pair < cv::Point2d, cv::Point2d > > frame0000ScreenPoints;
  std::vector < std::pair < cv::Point2d, cv::Point2d > > frame1155ScreenPoints;
  std::vector < std::pair < cv::Point2d, cv::Point2d > > frame1400ScreenPoints;
  std::vector < unsigned int > frame0000framenumbers;
  std::vector < unsigned int > frame1155framenumbers;
  std::vector < unsigned int > frame1400framenumbers;
  frame0000ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(756,72), cv::Point(852,84 )) );
  frame0000ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(1426,78), cv::Point(1524,90 )) );
  frame0000ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(1406,328), cv::Point(1506,342 )) );
  frame0000ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(702,306), cv::Point(798,320 )) );
  for ( unsigned int i = 0 ; i < 4 ; i ++ ) 
  {
    frame0000framenumbers.push_back(2);
  }
 frame1155ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(668,34), cv::Point(762,52 )) );
  frame1155ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(1378,50), cv::Point(1474,62 )) );
  frame1155ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(1372,308), cv::Point(1468,324)) );
  frame1155ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(628,296), cv::Point( 714,308)) );
  for ( unsigned int i = 0 ; i < 4 ; i ++ ) 
  {
    frame1155framenumbers.push_back(1155);
  }
  frame1400ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(438,32), cv::Point(340,10 )) );
  frame1400ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(1016,162), cv::Point(930,142 )) );
  frame1400ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(798,386), cv::Point(714,368)) );
  frame1400ScreenPoints.push_back (std::pair<cv::Point2d,cv::Point2d>
      ( cv::Point2d(216,240), cv::Point( 122,220)) );
  for ( unsigned int i = 0 ; i < 4 ; i ++ ) 
  {
    frame1400framenumbers.push_back(1400);
  }

  Projector->SetWorldPointsByTriangulation(frame1400ScreenPoints,frame1400framenumbers, matcher);
//  Projector->SetWorldPointsByTriangulation(frame1155ScreenPoints,1155);
  Projector->SetWorldPointsByTriangulation(frame0000ScreenPoints,frame0000framenumbers, matcher);
 
  Projector->SetDrawLines(true);

  std::vector <cv::Point3d> WorldGridPoints;
  //these are the corners of the grid according to the handeye calibration of the certus
  WorldGridPoints.push_back ( cv::Point3d(-826.2,-207.2,-2010.6));
  WorldGridPoints.push_back ( cv::Point3d(-820.3,-205.0,-2036.9));
  WorldGridPoints.push_back ( cv::Point3d(-820.8,-166.1,-2033.7));
  WorldGridPoints.push_back ( cv::Point3d(-826.8,-168.4,-2007.0));
  Projector->SetWorldPoints(WorldGridPoints);
  Projector->Project(matcher);
  MITK_TEST_CONDITION_REQUIRED (Projector->GetProjectOK(), "Testing mitkProjectPointsOnStereoVideo projected OK"); 

  MITK_TEST_CONDITION(CheckTransformedPointVector(Projector->GetPointsInLeftLensCS()), "Testing projected points");
  return EXIT_SUCCESS;
}
