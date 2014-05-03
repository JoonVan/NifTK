/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkCameraCalibrationFromDirectory.h"
#include "mitkCameraCalibrationFacade.h"
#include <ios>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cv.h>
#include <highgui.h>
#include <niftkFileHelper.h>

namespace mitk {

//-----------------------------------------------------------------------------
CameraCalibrationFromDirectory::CameraCalibrationFromDirectory()
{

}


//-----------------------------------------------------------------------------
CameraCalibrationFromDirectory::~CameraCalibrationFromDirectory()
{

}


//-----------------------------------------------------------------------------
double CameraCalibrationFromDirectory::Calibrate(const std::string& fullDirectoryName,
    const int& numberCornersX,
    const int& numberCornersY,
    const double& sizeSquareMillimeters,
    const mitk::Point2D& pixelScaleFactor,
    const std::string& outputDirectoryName,
    const bool& writeImages
    )
{
  std::string outputFile = niftk::ConcatenatePath(outputDirectoryName, "mono-calibration.log");
  std::string intrinsicTxtFile = niftk::ConcatenatePath(outputDirectoryName, "calib.intrinsic.txt");
  std::string intrinsicXmlFile = niftk::ConcatenatePath(outputDirectoryName, "calib.intrinsic.xml");
  std::string distortionXmlFile = niftk::ConcatenatePath(outputDirectoryName, "calib.distortion.xml");
  
  std::ofstream fs;
  fs.open(outputFile.c_str(), std::ios::out);
  if (!fs.fail())
  {
    std::cout << "Writing to " << outputFile << std::endl;
  }
  else
  {
    std::cerr << "ERROR: Writing calibration data to file " << outputFile << " failed!" << std::endl;
    return -1;
  }

  double reprojectionError = std::numeric_limits<double>::max();
  int width = 0;
  int height = 0;

  std::vector<IplImage*> images;
  std::vector<std::string> fileNames;

  std::vector<IplImage*> successfullImages;
  std::vector<std::string> successfullFileNames;

  CvMat *imagePoints = NULL;
  CvMat *objectPoints = NULL;
  CvMat *pointCounts = NULL;

  CvMat *intrinsicMatrix = cvCreateMat(3,3,CV_64FC1);
  CvMat *distortionCoeffs = cvCreateMat(1, 4, CV_64FC1);

  LoadImagesFromDirectory(fullDirectoryName, images, fileNames);

  CheckConstImageSize(images, width, height);
  CvSize imageSize = cvGetSize(images[0]);

  ExtractChessBoardPoints(images, fileNames, numberCornersX, numberCornersY, writeImages, sizeSquareMillimeters, pixelScaleFactor, successfullImages, successfullFileNames, imagePoints, objectPoints, pointCounts);

  int numberOfSuccessfulViews = successfullImages.size();
  CvMat *rotationVectors = cvCreateMat(numberOfSuccessfulViews, 3,CV_64FC1);
  CvMat *translationVectors = cvCreateMat(numberOfSuccessfulViews, 3, CV_64FC1);

  reprojectionError = CalibrateSingleCameraParameters(
      *objectPoints,
      *imagePoints,
      *pointCounts,
      imageSize,
      *intrinsicMatrix,
      *distortionCoeffs,
      *rotationVectors,
      *translationVectors
      );

  fs << "Mono calibration" << std::endl;
  OutputCalibrationData(
      fs,
      outputDirectoryName,
      intrinsicTxtFile,
      *objectPoints,
      *imagePoints,
      *pointCounts,
      *intrinsicMatrix,
      *distortionCoeffs,
      *rotationVectors,
      *translationVectors,
      reprojectionError,
      width,
      height,
      numberCornersX,
      numberCornersY,
      successfullFileNames
      );

  // Also output these as XML, as they are used in niftkCorrectVideoDistortion
  cvSave(intrinsicXmlFile.c_str(), intrinsicMatrix);
  cvSave(distortionXmlFile.c_str(), distortionCoeffs);

  // Tidy up.
  if(fs.is_open())
  {
    fs.close();
  }
  cvReleaseMat(&imagePoints);
  cvReleaseMat(&objectPoints);
  cvReleaseMat(&pointCounts);
  cvReleaseMat(&intrinsicMatrix);
  cvReleaseMat(&distortionCoeffs);
  cvReleaseMat(&rotationVectors);
  cvReleaseMat(&translationVectors);

  for (unsigned int i = 0; i < images.size(); i++)
  {
    cvReleaseImage(&images[i]);
  }

  return reprojectionError;
}

} // end namespace
