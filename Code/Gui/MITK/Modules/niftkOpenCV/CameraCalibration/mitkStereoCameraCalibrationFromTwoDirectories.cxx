/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkStereoCameraCalibrationFromTwoDirectories.h"
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
StereoCameraCalibrationFromTwoDirectories::StereoCameraCalibrationFromTwoDirectories()
{

}


//-----------------------------------------------------------------------------
StereoCameraCalibrationFromTwoDirectories::~StereoCameraCalibrationFromTwoDirectories()
{

}


//-----------------------------------------------------------------------------
double StereoCameraCalibrationFromTwoDirectories::Calibrate(const std::string& leftDirectoryName,
    const std::string& rightDirectoryName,
    const int& numberCornersX,
    const int& numberCornersY,
    const float& sizeSquareMillimeters,
    const std::string& outputFileName,
    const bool& writeImages
    )
{
  double reprojectionError = std::numeric_limits<double>::max();
  int width = 0;
  int height = 0;

  std::vector<IplImage*> imagesLeft;
  std::vector<std::string> fileNamesLeft;
  std::vector<IplImage*> successfullImagesLeft;
  std::vector<std::string> successfullFileNamesLeft;

  std::vector<IplImage*> imagesRight;
  std::vector<std::string> fileNamesRight;
  std::vector<IplImage*> successfullImagesRight;
  std::vector<std::string> successfullFileNamesRight;

  CvMat *imagePointsLeft = NULL;
  CvMat *objectPointsLeft = NULL;
  CvMat *pointCountsLeft = NULL;

  CvMat *imagePointsRight = NULL;
  CvMat *objectPointsRight = NULL;
  CvMat *pointCountsRight = NULL;

  LoadChessBoardsFromDirectory(leftDirectoryName, imagesLeft, fileNamesLeft);
  LoadChessBoardsFromDirectory(rightDirectoryName, imagesRight, fileNamesRight);

  std::vector<IplImage*> allImages;
  allImages.insert(allImages.begin(), imagesLeft.begin(), imagesLeft.end());
  allImages.insert(allImages.begin(), imagesRight.begin(), imagesRight.end());
  CheckConstImageSize(allImages, width, height);

  CvSize imageSize = cvGetSize(allImages[0]);

  ExtractChessBoardPoints(imagesLeft, fileNamesLeft, numberCornersX, numberCornersY, writeImages, sizeSquareMillimeters, successfullImagesLeft, successfullFileNamesLeft, imagePointsLeft, objectPointsLeft, pointCountsLeft);
  ExtractChessBoardPoints(imagesRight, fileNamesRight, numberCornersX, numberCornersY, writeImages, sizeSquareMillimeters, successfullImagesRight, successfullFileNamesRight, imagePointsRight, objectPointsRight, pointCountsRight);

  std::vector<std::string> allSuccessfulFileNames;
  allSuccessfulFileNames.insert(allSuccessfulFileNames.begin(), successfullFileNamesLeft.begin(), successfullFileNamesLeft.end());
  allSuccessfulFileNames.insert(allSuccessfulFileNames.begin(), successfullFileNamesRight.begin(), successfullFileNamesRight.end());

  // Sanity check
  if (successfullImagesLeft.size() != successfullImagesRight.size())
  {
    throw std::logic_error("The left and right channel had a different number of images with successfully matched corners.");
  }

  int numberOfSuccessfulViews = successfullImagesLeft.size();

  CvMat *intrinsicMatrixLeft = cvCreateMat(3,3,CV_32FC1);
  CvMat *distortionCoeffsLeft = cvCreateMat(5, 1, CV_32FC1);
  CvMat *rotationVectorsLeft = cvCreateMat(numberOfSuccessfulViews, 3,CV_32FC1);
  CvMat *translationVectorsLeft = cvCreateMat(numberOfSuccessfulViews, 3, CV_32FC1);

  CvMat *intrinsicMatrixRight = cvCreateMat(3,3,CV_32FC1);
  CvMat *distortionCoeffsRight = cvCreateMat(5, 1, CV_32FC1);
  CvMat *rotationVectorsRight = cvCreateMat(numberOfSuccessfulViews, 3,CV_32FC1);
  CvMat *translationVectorsRight = cvCreateMat(numberOfSuccessfulViews, 3, CV_32FC1);

  CvMat *rightToLeftRotationMatrix = cvCreateMat(3, 3,CV_32FC1);
  CvMat *rightToLeftTranslationVector = cvCreateMat(3, 1, CV_32FC1);
  CvMat *rightToLeftRotationVectors = cvCreateMat(numberOfSuccessfulViews, 3,CV_32FC1);
  CvMat *rightToLeftTranslationVectors = cvCreateMat(numberOfSuccessfulViews, 3, CV_32FC1);
  CvMat *r2LRot = cvCreateMat(1, 3, CV_32FC1);
  CvMat *r2LTrans = cvCreateMat(1, 3, CV_32FC1);
  CvMat *essentialMatrix = cvCreateMat(3, 3,CV_32FC1);
  CvMat *fundamentalMatrix = cvCreateMat(3, 3,CV_32FC1);

  reprojectionError = CalibrateStereoCameraParameters(
      numberOfSuccessfulViews,
      *objectPointsLeft,
      *imagePointsLeft,
      *pointCountsLeft,
      imageSize,
      *objectPointsRight,
      *imagePointsRight,
      *pointCountsRight,
      *intrinsicMatrixLeft,
      *distortionCoeffsLeft,
      *rotationVectorsLeft,
      *translationVectorsLeft,
      *intrinsicMatrixRight,
      *distortionCoeffsRight,
      *rotationVectorsRight,
      *translationVectorsRight,
      *rightToLeftRotationMatrix,
      *rightToLeftTranslationVector,
      *essentialMatrix,
      *fundamentalMatrix
      );

  std::ostream *os = NULL;
  std::ostringstream oss;
  std::ofstream fs;

  if (outputFileName.size() > 0)
  {
    fs.open(outputFileName.c_str(), std::ios::out);
    if (!fs.fail())
    {
      os = &fs;
      std::cout << "Writing to " << outputFileName << std::endl;
    }
    else
    {
      std::cerr << "ERROR: Writing calibration data to file " << outputFileName << " failed!" << std::endl;
    }
  }
  else
  {
    os = &oss;
  }

  *os << "Stereo calibration" << std::endl;

  *os << "Left camera" << std::endl;
  OutputCalibrationData(
      *os,
      *objectPointsLeft,
      *imagePointsLeft,
      *pointCountsLeft,
      *intrinsicMatrixLeft,
      *distortionCoeffsLeft,
      *rotationVectorsLeft,
      *translationVectorsLeft,
      reprojectionError,
      width,
      height,
      numberCornersX,
      numberCornersY,
      successfullFileNamesLeft
      );

  // Also output these as XML, as they are used in niftkCorrectVideoDistortion
  cvSave(std::string(outputFileName + ".left.intrinsic.xml").c_str(), intrinsicMatrixLeft);
  cvSave(std::string(outputFileName + ".left.distortion.xml").c_str(), distortionCoeffsLeft);

  *os << "Right camera" << std::endl;
  OutputCalibrationData(
      *os,
      *objectPointsRight,
      *imagePointsRight,
      *pointCountsRight,
      *intrinsicMatrixRight,
      *distortionCoeffsRight,
      *rotationVectorsRight,
      *translationVectorsRight,
      reprojectionError,
      width,
      height,
      numberCornersX,
      numberCornersY,
      successfullFileNamesRight
      );

  // Also output these as XML, as they are used in niftkCorrectVideoDistortion
  cvSave(std::string(outputFileName + ".right.intrinsic.xml").c_str(), intrinsicMatrixRight);
  cvSave(std::string(outputFileName + ".right.distortion.xml").c_str(), distortionCoeffsRight);

  // Output the right to left rotation and translation.
  // This is the MEDIAN of all the views.
  cvSave(std::string(outputFileName + ".r2l.rotation.xml").c_str(), rightToLeftRotationMatrix);
  cvSave(std::string(outputFileName + ".r2l.translation.xml").c_str(), rightToLeftTranslationVector);

  // Also calculate specific right to left transformations for each view.
  ComputeRightToLeftTransformations(
      *rotationVectorsLeft,
      *translationVectorsLeft,
      *rotationVectorsRight,
      *translationVectorsRight,
      *rightToLeftRotationVectors,
      *rightToLeftTranslationVectors
      );

  for (unsigned int i = 0; i < successfullFileNamesLeft.size(); i++)
  {
    for (int j = 0; j < 3; j++)
    {
      CV_MAT_ELEM(*r2LRot, float, 0, j) = CV_MAT_ELEM(*rightToLeftRotationVectors, float, i, j);
      CV_MAT_ELEM(*r2LTrans, float, 0, j) = CV_MAT_ELEM(*rightToLeftTranslationVectors, float, i, j);
    }
    cvSave(std::string(successfullFileNamesLeft[i] + ".r2l.rotation.xml").c_str(), r2LRot);
    cvSave(std::string(successfullFileNamesLeft[i] + ".r2l.translation.xml").c_str(), r2LTrans);
  }

  // Might as well
  cvSave(std::string(outputFileName + ".essential.xml").c_str(), essentialMatrix);
  cvSave(std::string(outputFileName + ".fundamental.xml").c_str(), fundamentalMatrix);

  // Tidy up.
  if(fs.is_open())
  {
    fs.close();
  }

  cvReleaseMat(&imagePointsLeft);
  cvReleaseMat(&objectPointsLeft);
  cvReleaseMat(&pointCountsLeft);
  cvReleaseMat(&imagePointsRight);
  cvReleaseMat(&objectPointsRight);
  cvReleaseMat(&pointCountsRight);

  cvReleaseMat(&intrinsicMatrixLeft);
  cvReleaseMat(&distortionCoeffsLeft);
  cvReleaseMat(&rotationVectorsLeft);
  cvReleaseMat(&translationVectorsLeft);

  cvReleaseMat(&intrinsicMatrixRight);
  cvReleaseMat(&distortionCoeffsRight);
  cvReleaseMat(&rotationVectorsRight);
  cvReleaseMat(&translationVectorsRight);

  cvReleaseMat(&rightToLeftRotationMatrix);
  cvReleaseMat(&rightToLeftTranslationVector);
  cvReleaseMat(&rightToLeftRotationVectors);
  cvReleaseMat(&rightToLeftTranslationVectors);
  cvReleaseMat(&r2LRot);
  cvReleaseMat(&r2LTrans);
  cvReleaseMat(&fundamentalMatrix);
  cvReleaseMat(&essentialMatrix);

  for (unsigned int i = 0; i < allImages.size(); i++)
  {
    cvReleaseImage(&allImages[i]);
  }

  return reprojectionError;
}

} // end namespace