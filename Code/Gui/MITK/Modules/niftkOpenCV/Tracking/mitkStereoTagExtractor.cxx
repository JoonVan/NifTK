/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkStereoTagExtractor.h"
#include "mitkTagTrackingFacade.h"
#include <mitkImageToOpenCVImageFilter.h>
#include <cv.h>

namespace mitk {

//-----------------------------------------------------------------------------
StereoTagExtractor::StereoTagExtractor()
{

}


//-----------------------------------------------------------------------------
StereoTagExtractor::~StereoTagExtractor()
{

}


//-----------------------------------------------------------------------------
void StereoTagExtractor::ExtractPoints(const mitk::Image::Pointer leftImage,
                                       const mitk::Image::Pointer rightImage,
                                       const float& minSize,
                                       const float& maxSize,
                                       const CvMat& leftCameraIntrinsics,
                                       const CvMat& rightCameraIntrinsics,
                                       const CvMat& rightToLeftRotationVector,
                                       const CvMat& rightToLeftTranslationVector,
                                       mitk::PointSet::Pointer pointSet
                                      )
{
  pointSet->Clear();

  mitk::ImageToOpenCVImageFilter::Pointer leftOpenCVFilter = mitk::ImageToOpenCVImageFilter::New();
  leftOpenCVFilter->SetImage(leftImage);

  IplImage *leftIm = leftOpenCVFilter->GetOpenCVImage();
  cv::Mat left(leftIm);

  mitk::ImageToOpenCVImageFilter::Pointer rightOpenCVFilter = mitk::ImageToOpenCVImageFilter::New();
  rightOpenCVFilter->SetImage(rightImage);

  IplImage *rightIm = rightOpenCVFilter->GetOpenCVImage();
  cv::Mat right(rightIm);

  cv::Mat leftInt(&leftCameraIntrinsics);
  cv::Mat rightInt(&rightCameraIntrinsics);
  cv::Mat r2lRot(&rightToLeftRotationVector);
  cv::Mat r2lTran(&rightToLeftTranslationVector);

  std::map<int, cv::Point3f> result = mitk::DetectMarkerPairs(
    left,
    right,
    leftInt,
    rightInt,
    r2lRot,
    r2lTran,
    minSize,
    maxSize
    );

  cv::Point3f extractedPoint;
  mitk::PointSet::PointType outputPoint;

  std::map<int, cv::Point3f>::iterator iter;
  for (iter = result.begin(); iter != result.end(); ++iter)
  {
    extractedPoint = (*iter).second;
    outputPoint[0] = extractedPoint.x;
    outputPoint[1] = extractedPoint.y;
    outputPoint[2] = extractedPoint.z;
    pointSet->InsertPoint((*iter).first, outputPoint);
  }

  cvReleaseImage(&leftIm);
  cvReleaseImage(&rightIm);
}


//-----------------------------------------------------------------------------
} // end namespace