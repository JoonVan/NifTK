/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkStereoCameraCalibration.h"
#include "mitkCameraCalibrationFacade.h"
#include <ios>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cv.h>
#include <highgui.h>
#include <niftkFileHelper.h>
#include <cstdlib>
#include <set>

namespace mitk {

//-----------------------------------------------------------------------------
StereoCameraCalibration::StereoCameraCalibration()
{

}


//-----------------------------------------------------------------------------
StereoCameraCalibration::~StereoCameraCalibration()
{

}


//-----------------------------------------------------------------------------
double StereoCameraCalibration::Calibrate(const std::string& leftDirectoryName,
    const std::string& rightDirectoryName,
    const int& numberOfFrames,
    const int& numberCornersX,
    const int& numberCornersY,
    const double& sizeSquareMillimeters,
    const mitk::Point2D& pixelScaleFactor,
    const std::string& outputDirectoryName,
    const bool& writeImages
    )
{
  std::string outputFileName = niftk::ConcatenatePath(outputDirectoryName, "stereo-calibration.log");
  std::string r2lFileName = niftk::ConcatenatePath(outputDirectoryName, "calib.r2l.txt");
  std::string r2lRotationFileName = niftk::ConcatenatePath(outputDirectoryName, "calib.r2l.rotation.xml");
  std::string r2lTranslationFileName = niftk::ConcatenatePath(outputDirectoryName, "calib.r2l.translation.xml");
  std::string leftIntrinsicTextFile = niftk::ConcatenatePath(outputDirectoryName, "calib.left.intrinsic.txt");
  std::string leftIntrinsicXmlFile = niftk::ConcatenatePath(outputDirectoryName, "calib.left.intrinsic.xml");
  std::string leftDistortionXmlFile = niftk::ConcatenatePath(outputDirectoryName, "calib.left.distortion.xml");
  std::string rightIntrinsicTextFile = niftk::ConcatenatePath(outputDirectoryName, "calib.right.intrinsic.txt");
  std::string rightIntrinsicXmlFile = niftk::ConcatenatePath(outputDirectoryName, "calib.right.intrinsic.xml");
  std::string rightDistortionXmlFile = niftk::ConcatenatePath(outputDirectoryName, "calib.right.distortion.xml");
  std::string essentialMatrixFile = niftk::ConcatenatePath(outputDirectoryName, "calib.essential.xml");
  std::string fundamentalMatrixFile = niftk::ConcatenatePath(outputDirectoryName, "calib.fundamental.xml");
  
  std::ofstream fs;
  fs.open(outputFileName.c_str(), std::ios::out);
  if (!fs.fail())
  {
    std::cout << "Writing main calibration output to " << outputFileName << std::endl;
  }
  else
  {
    std::cerr << "ERROR: Writing main calibration output to file " << outputFileName << " failed!" << std::endl;
    return -1;
  }

  std::ofstream fsr2l;
  fsr2l.open((r2lFileName).c_str(), std::ios::out);
  if (!fsr2l.fail())
  {
    std::cout << "Writing right-to-left transform to " << r2lFileName << std::endl;
  }
  else
  {
    std::cerr << "ERROR: Writing right-to-left data to file " << r2lFileName << " failed!" << std::endl;
    if(fs.is_open())
    {
      fs.close();
    }
    return -2;
  }

  std::vector<IplImage*> imagesLeft;
  std::vector<std::string> fileNamesLeft;
  std::vector<IplImage*> imagesRight;
  std::vector<std::string> fileNamesRight;
  std::vector<std::string> successfulLeftFiles;
  std::vector<std::string> successfulRightFiles;

  // Load Data. There are two options.
  if (numberOfFrames == 0)
  {
    // Option a. Scan the whole directory. Directory assumed to contain ONLY chessboard images.
    // We scan left and right directory working out successful PAIRS. Then we double back
    // and only load the successful pairs.

    std::vector<std::string> leftFiles = niftk::GetFilesInDirectory(leftDirectoryName);
    std::sort(leftFiles.begin(), leftFiles.end());

    std::vector<std::string> rightFiles = niftk::GetFilesInDirectory(rightDirectoryName);
    std::sort(rightFiles.begin(), rightFiles.end());

    if (leftFiles.size() != rightFiles.size())
    {
      std::cerr << "StereoCameraCalibration: Different number of files in:\n" << leftDirectoryName << "\nand\n" << rightDirectoryName << std::endl;
      return 0;
    }

    for (int i = 0; i < leftFiles.size(); i++)
    {
      mitk::CheckAndAppendPairOfFileNames(leftFiles[i], rightFiles[i],
                                          numberCornersX, numberCornersY, sizeSquareMillimeters, pixelScaleFactor,
                                          successfulLeftFiles, successfulRightFiles);
    }
  }
  else if (numberOfFrames != 0)
  {
    // Option b. Get the whole list of files, randomly select successive pairs,
    // test if successful, and aim for a certain number of successful items.

    std::cout << "StereoCameraCalibration:Scanning for " << numberOfFrames << std::endl;

    std::vector<std::string> files = niftk::GetFilesInDirectory(leftDirectoryName);
    if (files.size() == 0)
    {
      files = niftk::GetFilesInDirectory(rightDirectoryName);
    }
    if (files.size() == 0)
    {
      std::cerr << "StereoCameraCalibration: Failed to find files in either:\n" << leftDirectoryName << "\nor\n" << rightDirectoryName << std::endl;
      return 0;
    }

    std::sort(files.begin(), files.end());

    std::set<unsigned int> setOfIndexes;
    std::vector<std::string> successfulLeftFiles;
    std::vector<std::string> successfulRightFiles;

    while(setOfIndexes.size() < numberOfFrames)
    {
      // Pick a random file.
      unsigned int indexOfFile = std::rand() % files.size();
      unsigned int indexOfLeft, indexOfRight;
      if (indexOfFile == 0)
      {
        indexOfLeft = 0;
        indexOfRight = 1;
      }
      else if (indexOfFile%2 == 1)
      {
        indexOfLeft = indexOfFile - 1;
        indexOfRight = indexOfFile;
      }
      else if (indexOfFile%2 == 0)
      {
        indexOfLeft = indexOfFile;
        indexOfRight = indexOfFile + 1;
      }

      if (setOfIndexes.find(indexOfLeft) == setOfIndexes.end())
      {

        std::string leftFileName = files[indexOfLeft];
        std::string rightFileName = files[indexOfRight];

        bool added = CheckAndAppendPairOfFileNames(leftFileName, rightFileName,
                                                   numberCornersX, numberCornersY, sizeSquareMillimeters, pixelScaleFactor,
                                                   successfulLeftFiles, successfulRightFiles);
        if (added)
        {
          setOfIndexes.insert(indexOfLeft);
        }
      }
    } // end while
  }

  std::cout << "StereoCameraCalibration: Loading left" << std::endl;
  LoadImages(successfulLeftFiles, imagesLeft, fileNamesLeft);

  std::cout << "StereoCameraCalibration: Loading right" << std::endl;
  LoadImages(successfulRightFiles, imagesRight, fileNamesRight);

  double reprojectionError = std::numeric_limits<double>::max();
  std::vector<double> leftMonoReprojectionErrors;
  std::vector<double> rightMonoReprojectionErrors;
  int width = 0;
  int height = 0;

  std::vector<IplImage*> successfullImagesLeft;
  std::vector<std::string> successfullFileNamesLeft;

  std::vector<IplImage*> successfullImagesRight;
  std::vector<std::string> successfullFileNamesRight;

  CvMat *imagePointsLeft = NULL;
  CvMat *objectPointsLeft = NULL;
  CvMat *pointCountsLeft = NULL;

  CvMat *imagePointsRight = NULL;
  CvMat *objectPointsRight = NULL;
  CvMat *pointCountsRight = NULL;


  std::vector<IplImage*> allImages;
  allImages.insert(allImages.begin(), imagesLeft.begin(), imagesLeft.end());
  allImages.insert(allImages.begin(), imagesRight.begin(), imagesRight.end());
  CheckConstImageSize(allImages, width, height);

  CvSize imageSize = cvGetSize(allImages[0]);

  ExtractChessBoardPoints(imagesLeft, fileNamesLeft, numberCornersX, numberCornersY, writeImages, sizeSquareMillimeters, pixelScaleFactor, successfullImagesLeft, successfullFileNamesLeft, imagePointsLeft, objectPointsLeft, pointCountsLeft);
  ExtractChessBoardPoints(imagesRight, fileNamesRight, numberCornersX, numberCornersY, writeImages, sizeSquareMillimeters, pixelScaleFactor, successfullImagesRight, successfullFileNamesRight, imagePointsRight, objectPointsRight, pointCountsRight);

  std::vector<std::string> allSuccessfulFileNames;
  allSuccessfulFileNames.insert(allSuccessfulFileNames.begin(), successfullFileNamesLeft.begin(), successfullFileNamesLeft.end());
  allSuccessfulFileNames.insert(allSuccessfulFileNames.begin(), successfullFileNamesRight.begin(), successfullFileNamesRight.end());

  // Sanity check
  if (successfullImagesLeft.size() != successfullImagesRight.size())
  {
    throw std::logic_error("The left and right channel had a different number of images with successfully matched corners.");
  }

  int numberOfSuccessfulViews = successfullImagesLeft.size();

  CvMat *intrinsicMatrixLeft = cvCreateMat(3,3,CV_64FC1);
  CvMat *distortionCoeffsLeft = cvCreateMat(4, 1, CV_64FC1);
  CvMat *rotationVectorsLeft = cvCreateMat(numberOfSuccessfulViews, 3,CV_64FC1);
  CvMat *translationVectorsLeft = cvCreateMat(numberOfSuccessfulViews, 3, CV_64FC1);

  CvMat *intrinsicMatrixRight = cvCreateMat(3,3,CV_64FC1);
  CvMat *distortionCoeffsRight = cvCreateMat(4, 1, CV_64FC1);
  CvMat *rotationVectorsRight = cvCreateMat(numberOfSuccessfulViews, 3,CV_64FC1);
  CvMat *translationVectorsRight = cvCreateMat(numberOfSuccessfulViews, 3, CV_64FC1);

  CvMat *rightToLeftRotationMatrix = cvCreateMat(3, 3,CV_64FC1);
  CvMat *rightToLeftTranslationVector = cvCreateMat(3, 1, CV_64FC1);
  CvMat *rightToLeftRotationVectors = cvCreateMat(numberOfSuccessfulViews, 3,CV_64FC1);
  CvMat *rightToLeftTranslationVectors = cvCreateMat(numberOfSuccessfulViews, 3, CV_64FC1);
  CvMat *r2LRot = cvCreateMat(1, 3, CV_64FC1);
  CvMat *r2LTrans = cvCreateMat(1, 3, CV_64FC1);
  CvMat *essentialMatrix = cvCreateMat(3, 3,CV_64FC1);
  CvMat *fundamentalMatrix = cvCreateMat(3, 3,CV_64FC1);

  reprojectionError = CalibrateStereoCameraParameters(
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

  fs << "Stereo calibration" << std::endl;

  fs << "Left camera" << std::endl;
  leftMonoReprojectionErrors = OutputCalibrationData(
      fs,
      outputDirectoryName,
      leftIntrinsicTextFile,
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
  cvSave(leftIntrinsicXmlFile.c_str(), intrinsicMatrixLeft);
  cvSave(leftDistortionXmlFile.c_str(), distortionCoeffsLeft);

  fs << "Right camera" << std::endl;
  rightMonoReprojectionErrors = OutputCalibrationData(
      fs,
      outputDirectoryName,
      rightIntrinsicTextFile,
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
  cvSave(rightIntrinsicXmlFile.c_str(), intrinsicMatrixRight);
  cvSave(rightDistortionXmlFile.c_str(), distortionCoeffsRight);

  // Output the right to left rotation and translation.
  // This is the MEDIAN of all the views.
  cvSave(r2lRotationFileName.c_str(), rightToLeftRotationMatrix);
  cvSave(r2lTranslationFileName.c_str(), rightToLeftTranslationVector);

  // Output right to left MEDIAN transformation as a rotation [3x3] then a translation [1x3]
  for (int i = 0; i < 3; i++)
  {
    fsr2l << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, i, 0) << " " << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, i, 1) << " " << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, i, 2) << std::endl;
  }
  fsr2l << CV_MAT_ELEM(*rightToLeftTranslationVector, double, 0, 0) << " " << CV_MAT_ELEM(*rightToLeftTranslationVector, double, 1, 0) << " " << CV_MAT_ELEM(*rightToLeftTranslationVector, double, 2, 0) << std::endl;

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
      CV_MAT_ELEM(*r2LRot, double, 0, j) = CV_MAT_ELEM(*rightToLeftRotationVectors, double, i, j);
      CV_MAT_ELEM(*r2LTrans, double, 0, j) = CV_MAT_ELEM(*rightToLeftTranslationVectors, double, i, j);
    }
    
    cvSave((niftk::ConcatenatePath(outputDirectoryName, niftk::Basename(successfullFileNamesLeft[i]) + std::string(".r2l.rotation.xml"))).c_str(), r2LRot);
    cvSave((niftk::ConcatenatePath(outputDirectoryName, niftk::Basename(successfullFileNamesLeft[i]) + std::string(".r2l.translation.xml"))).c_str(), r2LTrans);
    
    // Also output in plain text format, which is a [3x3] rotation, AND THEN a [1x3] translation.
    std::ofstream tmpR2L;
    std::string tmpR2LFileName = niftk::ConcatenatePath(outputDirectoryName, niftk::Basename(successfullFileNamesLeft[i]) + std::string(".r2l.txt"));
    tmpR2L.open((tmpR2LFileName).c_str(), std::ios::out);
    if (!tmpR2L.fail())
    {
      cvRodrigues2(r2LRot, rightToLeftRotationMatrix);
      tmpR2L << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 0, 0) << " " << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 0, 1) << " " << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 0, 2) << std::endl;
      tmpR2L << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 1, 0) << " " << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 1, 1) << " " << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 1, 2) << std::endl;
      tmpR2L << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 2, 0) << " " << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 2, 1) << " " << CV_MAT_ELEM(*rightToLeftRotationMatrix, double, 2, 2) << std::endl;
      tmpR2L << CV_MAT_ELEM(*r2LTrans, double, 0, 0) << " " << CV_MAT_ELEM(*r2LTrans, double, 0, 1) << " " << CV_MAT_ELEM(*r2LTrans, double, 0, 2) << std::endl;
    }
    else
    {
      std::cerr << "ERROR: Writing right-to-left data to file " << tmpR2LFileName << " failed!" << std::endl;
    }
    if(tmpR2L.is_open())
    {
      tmpR2L.close();
    }
    fs << "Projecting error to individual camera[" << successfullFileNamesLeft[i] << "]: left=" << leftMonoReprojectionErrors[i] << ", right=" << rightMonoReprojectionErrors[i] << ", mean=" << (leftMonoReprojectionErrors[i]+rightMonoReprojectionErrors[i])/2.0 << std::endl;
  }  // end for each file


  // Might as well
  cvSave(essentialMatrixFile.c_str(), essentialMatrix);
  cvSave(fundamentalMatrixFile.c_str(), fundamentalMatrix);

  // Tidy up.
  if(fs.is_open())
  {
    fs.close();
  }
  if(fsr2l.is_open())
  {
    fsr2l.close();
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
