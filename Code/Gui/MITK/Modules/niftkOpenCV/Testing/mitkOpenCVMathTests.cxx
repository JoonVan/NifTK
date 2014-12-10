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

#include <niftkFileHelper.h>
#include <mitkTestingMacros.h>
#include <mitkLogMacros.h>
#include <mitkOpenCVMaths.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <cmath>

/**
 * \file Tests for some of the functions in openCVMaths.
 */

void ArithmaticTests()
{
  cv::Point2d point1 = cv::Point2d ( 1.0 , 1.0 );
  cv::Point2d point2 = cv::Point2d ( 0.5 , 0.3 );
  cv::Point2d point3 = cv::Point2d ( 0.7 , 1.4 );
  cv::Point2d point4 = cv::Point2d ( 1.5 , 2.3 );

  double tolerance = 1e-6;
  MITK_TEST_CONDITION ( point1 == point1 , "Testing point2d equality operator" );
  MITK_TEST_CONDITION (mitk::NearlyEqual (point1, point1, tolerance) , "Testing Nearly Equal " );
  MITK_TEST_CONDITION (mitk::NearlyEqual (( point1 + point2 ), cv::Point2d ( 1.5 , 1.3 ), tolerance), "Testing addition operator");
  MITK_TEST_CONDITION (mitk::NearlyEqual ((point4 - point3) ,cv::Point2d(0.8, 0.9), tolerance), "Testing subtraction operator");
}

void RMSTest()
{
  //make some measured points
  std::vector < mitk::ProjectedPointPairsWithTimingError > measured;
  std::vector < mitk::ProjectedPointPairsWithTimingError > actual;

  mitk::ProjectedPointPairsWithTimingError measured_0;
  measured_0.m_Points.push_back(
      mitk::ProjectedPointPair (cv::Point2d ( -0.1, 0.7 ),cv::Point2d (1.1 , 0.9)));
  measured.push_back(measured_0);

  mitk::ProjectedPointPairsWithTimingError actual_0;
  actual_0.m_Points.push_back(
      mitk::ProjectedPointPair (cv::Point2d ( -0.1, 0.7 ),cv::Point2d (1.1 , 0.9)));
  actual.push_back(actual_0);

  int index = -1;
  cv::Point2d outlierSD = cv::Point2d(2.0, 2.0);
  long long allowableTimingError = 30e6;
  bool duplicateLines = true;
  std::pair <double,double> rmsError = mitk::RMSError ( measured , actual ,
      index, outlierSD, allowableTimingError , duplicateLines );

  double tolerance = 1e-4;
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( cv::Point2d( rmsError.first, rmsError.second ), cv::Point2d ( 0.0, 0.0), tolerance) , "Testing RMSError returns 0.0 when no error" );

  mitk::ProjectedPointPairsWithTimingError actual_1;
  actual_1.m_Points.push_back(
      mitk::ProjectedPointPair (cv::Point2d ( -0.2, 0.5 ),cv::Point2d (1.2 , 0.3)));

  measured.push_back(measured_0);
  actual.push_back(actual_1);

  duplicateLines=false;
  rmsError = mitk::RMSError ( measured , actual ,
      index, outlierSD, allowableTimingError , duplicateLines );
  
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( cv::Point2d( rmsError.first, rmsError.second ) , cv::Point2d ( 0.15811, 0.43012), tolerance ) , "Testing RMSError returns right value for a real error" );
 
  duplicateLines=true;
  rmsError = mitk::RMSError ( measured , actual ,
      index, outlierSD, allowableTimingError , duplicateLines );
  
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( cv::Point2d (rmsError.first, rmsError.second) , cv::Point2d ( 0.0, 0.0), tolerance) , "Testing duplicate lines parameter has the desired effect" );

  mitk::ProjectedPointPairsWithTimingError measured_1;
  measured_1.m_Points.push_back(
      mitk::ProjectedPointPair (cv::Point2d ( -0.1, 0.7 ),cv::Point2d (1.1 , 0.9)));
  measured_1.m_TimingError = 30e7;
  measured.push_back(measured_1);
  actual.push_back(actual_0);

  duplicateLines=false;
  rmsError = mitk::RMSError ( measured , actual ,
      index, outlierSD, allowableTimingError , duplicateLines );
  
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( cv::Point2d (rmsError.first, rmsError.second ) ,cv::Point2d ( 0.15811, 0.43012), tolerance) , "Testing RMSError rejects high timing error points" );
 
  allowableTimingError = 31e7;

  outlierSD = cv::Point2d(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
  rmsError = mitk::RMSError ( measured , actual ,
      index, outlierSD, allowableTimingError , duplicateLines );
  
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( cv::Point2d(rmsError.first, rmsError.second) ,cv::Point2d ( 0.12910, 0.35119), tolerance) , "Testing RMSError accepts when allowable timing error increased" );
 
  outlierSD = cv::Point2d(2.0, 2.0);
  rmsError = mitk::RMSError ( measured , actual ,
      index, outlierSD, allowableTimingError , duplicateLines );
  
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( cv::Point2d(rmsError.first, rmsError.second) , cv::Point2d( 0.0, 0.0), tolerance) , "Testing RMSError culls outliers" );

}

void FindIntersectTest()
{
  cv::Vec4i line1;
  cv::Vec4i line2;
  cv::Point2d intersect;

  for ( unsigned int i = 0 ; i < 4 ; i ++ )
  {
    line1[i] = 0;
    line2[i] = 0;
  }
  intersect = mitk::FindIntersect (line1, line2);

  MITK_TEST_CONDITION ((boost::math::isnan(intersect.x) && boost::math::isnan(intersect.y)), "IntersectTest: all zeros : " << intersect );
  //parallel vertical lines
  line1[0] = 0 ; line1[1] = 0 ; line1[2] = 0 ; line1[3] = 1; 
  line2[0] = 1 ; line2[1] = 0 ; line2[2] = 1 ; line2[3] = 1;
  intersect = mitk::FindIntersect (line1, line2);
  MITK_TEST_CONDITION ((boost::math::isnan(intersect.x) && boost::math::isnan(intersect.y)), "IntersectTest: parallel vertical lines" << intersect );
  //parallel horizontal lines
  line1[0] = 0 ; line1[1] = 0 ; line1[2] = 1 ; line1[3] = 0; 
  line2[0] = 0 ; line2[1] = 1 ; line2[2] = 1 ; line2[3] = 1;
  intersect = mitk::FindIntersect (line1, line2);
  MITK_TEST_CONDITION ((boost::math::isnan(intersect.x) && boost::math::isnan(intersect.y)), "IntersectTest: parallel horizontal lines" << intersect );
  //parallel oblique lines
  line1[0] = 0 ; line1[1] = 0 ; line1[2] = 1 ; line1[3] = 1; 
  line2[0] = 1 ; line2[1] = 0 ; line2[2] = 2 ; line2[3] = 1;
  intersect = mitk::FindIntersect (line1, line2);
  MITK_TEST_CONDITION ((boost::math::isnan(intersect.x) && boost::math::isnan(intersect.y)), "IntersectTest: parallel oblique lines" << intersect );
  //simple intersection
  line1[0] = 0 ; line1[1] = 0 ; line1[2] = 1 ; line1[3] = 1; 
  line2[0] = 1 ; line2[1] = 0 ; line2[2] = 0 ; line2[3] = 1;
  intersect = mitk::FindIntersect (line1, line2);
  MITK_TEST_CONDITION (intersect == cv::Point2d(0.5,0.5), "IntersectTest: 45 degrees intersect at 0.5" << intersect );
  //line1 vertical
  line1[0] = 0 ; line1[1] = 0 ; line1[2] = 0 ; line1[3] = 1; 
  line2[0] = 1 ; line2[1] = 0 ; line2[2] = 0 ; line2[3] = 1;
  intersect = mitk::FindIntersect (line1, line2);
  MITK_TEST_CONDITION (intersect == cv::Point2d(0.0,1.0), "IntersectTest: line 1 vertical " << intersect );
  //line 2 vertical
  line1[0] = 0 ; line1[1] = 0 ; line1[2] = 1 ; line1[3] = 1; 
  line2[0] = 1 ; line2[1] = 0 ; line2[2] = 1 ; line2[3] = 1;
  intersect = mitk::FindIntersect (line1, line2);
  MITK_TEST_CONDITION (intersect == cv::Point2d(1.0,1.0), "IntersectTest: line 2 vertical " << intersect );
  
}

void FindIntersectsTest()
{
  cv::Vec4i line1;
  cv::Vec4i line2;
  cv::Vec4i line3;
  cv::Vec4i line4;

  //line1 is degenerate
  line1[0] = 0 ; line1[1] = 0 ; line1[2] = 0 ; line1[3] = 0;
  //line2 y = 2x + 10
  line2[0] = 0 ; line2[1] = 10 ; line2[2] = 10 ; line2[3] = 30;
  //line3 y = -x ;
  line3[0] = -5; line3[1] = 5 ; line3[2] = 5; line3[3] = -5;
  //line4 y = x;
  line4[0] = 0; line4[1] = 0 ; line4[2] = 15; line4[3] = 15;

  std::vector <cv::Vec4i> lines;
  lines.push_back(line1);
  lines.push_back(line2);
  lines.push_back(line3);
  lines.push_back(line4);

  bool rejectPointsNotOnBothLines = false;
  bool rejectNonPerpendicularLines = false;
  double perpendicularityTolerance = 20.0;
  
  std::vector <cv::Point2d> intersects;
  intersects = mitk::FindIntersects ( lines, rejectPointsNotOnBothLines, rejectNonPerpendicularLines, perpendicularityTolerance );
  // there should be 3 intersections, (-10/3, 10/3 ) , (-10,-10), and ( 0,0)
  MITK_TEST_CONDITION ( intersects.size() == 3 , "Testing size of intesects vector with no conditions " << intersects.size());
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( intersects[0], cv::Point2d (-3.333333, 3.333333), 1e-6) , "Testing first intersect with no conditions " << intersects[0]);
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( intersects[1], cv::Point2d (-10.0, -10.0),1e-6) , "Testing second intersect with no conditions " << intersects[1]);
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( intersects[2], cv::Point2d (0.0, 0.0), 1e-6) , "Testing third intersect with no conditions " << intersects[2]);
  // add a perpendicularity constraint should remove line 2 line 4
  rejectNonPerpendicularLines = true;
  perpendicularityTolerance = 20.0;
  intersects = mitk::FindIntersects ( lines, rejectPointsNotOnBothLines, rejectNonPerpendicularLines, perpendicularityTolerance );
  MITK_TEST_CONDITION ( intersects.size() == 2 , "Testing size of intesects vector with 20 degree perpendicularity" << intersects.size());
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( intersects[0], cv::Point2d (-3.333333, 3.333333), 1e-6) , "Testing first intersect with no conditions " << intersects[0]);
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( intersects[1], cv::Point2d (0.0, 0.0), 1e-6) , "Testing second intersect with no conditions " << intersects[1]);
  // add an on both lines constraint should leave only line3 line4
  rejectPointsNotOnBothLines = true;
  intersects = mitk::FindIntersects ( lines, rejectPointsNotOnBothLines, rejectNonPerpendicularLines, perpendicularityTolerance );
  MITK_TEST_CONDITION ( intersects.size() == 1 , "Testing size of intesects vector with 20 degree perpendicularity and on interval requirement" << intersects.size());
  MITK_TEST_CONDITION ( mitk::NearlyEqual ( intersects[0], cv::Point2d (0.0, 0.0), 1e-6) , "Testing first intersect with all conditions " << intersects[0]);


}
void AngleBetweenLinesTest()
{
  double tolerance = 1e-6;
  cv::Vec4i line1;
  cv::Vec4i line2;

  for ( unsigned int i = 0 ; i < 4 ; i++ )
  {
    line1[i] = 0;
    line2[i] = 0;
  }

  MITK_TEST_CONDITION ( boost::math::isnan(mitk::AngleBetweenLines ( line1, line2)) , "Testing angle between 2 zero vectors " << mitk::AngleBetweenLines ( line1, line2) );

  line1[1] = 1;
  line2[3] = 1;

  MITK_TEST_CONDITION ( ( fabs (mitk::AngleBetweenLines ( line1, line2) - 0.0) < tolerance  ) , "Testing angle between 2 parallel vectors " << mitk::AngleBetweenLines ( line1, line2) );

  line2[1] = 1;
  line2[0] = 1;
  
  MITK_TEST_CONDITION ( ( fabs (mitk::AngleBetweenLines ( line1, line2) - CV_PI/2.0 ) < tolerance ) , "Testing angle between 2 perpendicular vectors " << mitk::AngleBetweenLines ( line1, line2) );

  line2[1]=2;
  
  MITK_TEST_CONDITION ( ( fabs (mitk::AngleBetweenLines ( line1, line2) - CV_PI/4.0) < tolerance ) , "Testing angle between 2 vectors at 45 degrees " << mitk::AngleBetweenLines ( line1, line2) );

  for ( unsigned int i = 0 ; i < 4 ; i++ )
  {
    line1[i] = 0;
    line2[i] = 0;
  }

  line1[0] = 1;
  line2[2] = -1;

  MITK_TEST_CONDITION ( ( fabs (mitk::AngleBetweenLines ( line1, line2) - 0.0 ) < tolerance ) , "Testing angle between 2 opposite parallel vectors " << mitk::AngleBetweenLines ( line1, line2) );

}

void CheckIfLinesArePerpendicularTest ( )
{
 
  double angleTolerance = 10.0;
  cv::Vec4i line1;
  cv::Vec4i line2;

  for ( unsigned int i = 0 ; i < 4 ; i++ )
  {
    line1[i] = 0;
    line2[i] = 0;
  }

  MITK_TEST_CONDITION ( (! mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance ) ), "Checking that 2 zero vectors are not perpendicular" );

  line1[1] = 1;
  line2[3] = 1;

  MITK_TEST_CONDITION ( (! mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance) ), "Checking that 2 parallel vectors are not perpendicular" );

  angleTolerance = 90.0;
  
  MITK_TEST_CONDITION ( ( mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance) ), "Checking that 2 parallel vectors are perpendicular within 90 degrees tolerance" );
  
  line1[1] = 0;
  line2[3] = 0;
  
  MITK_TEST_CONDITION ( (! mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance ) ), "Checking that 2 zero vectors are not perpendicular even when tolerance = 90 degrees" );
  
  angleTolerance = 0.0;

  line1[0] = 1000;
  line2[1] = -1000;
  
  MITK_TEST_CONDITION ( ( mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance ) ), "Checking that 2 perpendicular vectors are exactly  perpendicular" );

  angleTolerance = 10.0;
  line1[1]=-167;
  
  
  MITK_TEST_CONDITION ( ( mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance ) ), "Checking that 2 lines at 80.5 degrees are perpendicular within 10 degrees" );

  angleTolerance = 9.0;
  
  MITK_TEST_CONDITION ( ( ! mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance ) ), "Checking that 2 lines at 80.5 degrees are not perpendicular within 9 degrees" );
  
  angleTolerance = 10.0;
  line1[1]=167;
  
  MITK_TEST_CONDITION ( ( mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance ) ), "Checking that 2 lines at 99.5 degrees are perpendicular within 10 degrees" );

  angleTolerance = 9.0;
  
  MITK_TEST_CONDITION ( ( ! mitk::CheckIfLinesArePerpendicular(line1,line2, angleTolerance ) ), "Checking that 2 lines at 99.5 degrees are not perpendicular within 9 degrees" );
  
}


void PointInIntervalTest ( )
{
 
  cv::Vec4i line;

  //when x2 and y2 are greater than x1 and x1

  line[0] = -10;
  line[1] = 5;
  line[2] = 5; 
  line[3] = 7;

  MITK_TEST_CONDITION ( ( ! mitk::PointInInterval(cv::Point2d(-10.1,6),line)), "Point just outside x bound" );
  MITK_TEST_CONDITION ( ( mitk::PointInInterval(cv::Point2d(-10.0,5),line)), "Point on x bound" );
  MITK_TEST_CONDITION ( ( mitk::PointInInterval(cv::Point2d(-9.9,5.5),line)), "Point just inside x bound" );
  MITK_TEST_CONDITION ( ( ! mitk::PointInInterval(cv::Point2d(-10.0,0),line)), "Point outside y bound" );
  MITK_TEST_CONDITION ( ( ! mitk::PointInInterval(cv::Point2d(20.0,0),line)), "Point outside both bound" );

  //when x1 and y1 are greater than x2 and x2

  line[2] = -10;
  line[3] = 5;
  line[0] = 5; 
  line[1] = 7;

  MITK_TEST_CONDITION ( ( ! mitk::PointInInterval(cv::Point2d(-10.1,6),line)), "Point just outside x bound" );
  MITK_TEST_CONDITION ( ( mitk::PointInInterval(cv::Point2d(-10.0,5),line)), "Point on x bound" );
  MITK_TEST_CONDITION ( ( mitk::PointInInterval(cv::Point2d(-9.9,5.5),line)), "Point just inside x bound" );
  MITK_TEST_CONDITION ( ( ! mitk::PointInInterval(cv::Point2d(-10.0,0),line)), "Point outside y bound" );
  MITK_TEST_CONDITION ( ( ! mitk::PointInInterval(cv::Point2d(20.0,0),line)), "Point outside both bound" );
  MITK_TEST_CONDITION ( ( ! mitk::PointInInterval(cv::Point2d(std::numeric_limits<double>::quiet_NaN(),0),line)), "Point is NaN" );

  line[2] = 0;
  line[3] = 0;
  line[0] = 0; 
  line[1] = 0;

  MITK_TEST_CONDITION ( ( ! mitk::PointInInterval(cv::Point2d(-10.1,6),line)), "Zero interval" );
  MITK_TEST_CONDITION ( ( mitk::PointInInterval(cv::Point2d(0.0,0.0),line)), "Zero interval and point" );
 
}

int mitkOpenCVMathTests(int argc, char * argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkOpenCVMathTests");

  //MITK_TEST_CONDITION ( ArithmaticTests() , "Testing basic arithmetic");
  //MITK_TEST_CONDITION ( RMSTest(), "Testing RMSError" );
  ArithmaticTests();
  RMSTest();
  FindIntersectTest();
  FindIntersectsTest();
  AngleBetweenLinesTest();
  CheckIfLinesArePerpendicularTest();
  PointInIntervalTest();
  MITK_TEST_END();
}



