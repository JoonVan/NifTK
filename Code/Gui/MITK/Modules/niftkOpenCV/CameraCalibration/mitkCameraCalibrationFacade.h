/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkCameraCalibrationFacade_h
#define mitkCameraCalibrationFacade_h

#include "niftkOpenCVExports.h"
#include <cv.h>
#include <cstdlib>
#include <iostream>
#include <mitkVector.h>

/**
 * \file mitkCameraCalibrationFacade
 * \brief Interface to OpenCV camera calibration, and associated routines.
 */
namespace mitk {

/**
 * \brief Uses OpenCV to load images.
 * \throw Throws exception if files is empty, or no images found.
 * \param[In] files list of files
 * \param[Out] images which the caller must take responsibility for and de-allocate appropriately.
 * \param[Out] fileNames list of filenames
 */
extern "C++" NIFTKOPENCV_EXPORT void LoadImages(const std::vector<std::string>& files,
  std::vector<IplImage*>& images,
  std::vector<std::string>& fileNames
  );


/**
 * \brief Scans a directory for all filenames, and uses OpenCV to load images.
 * \param fullDirectoryName full directory name
 * \param images which the caller must take responsibility for and de-allocate appropriately.
 * \param fileNames list of filenames
 */
extern "C++" NIFTKOPENCV_EXPORT void LoadImagesFromDirectory(
  const std::string& fullDirectoryName,
  std::vector<IplImage*>& images,
  std::vector<std::string>& fileNames
  );


/**
 * \brief Utility method to check and load stereo pairs of chessboards.
 * \param leftFileName
 * \param rightFileName
 * \param numberCornersX
 * \param numberCornersY
 * \param sizeSquareMillimeters
 * \param pixelScaleFactor
 * \param successfulLeftFiles
 * \param successfulRightFiles
 * \return true if the pair was loaded.
 */
extern "C++" NIFTKOPENCV_EXPORT bool CheckAndAppendPairOfFileNames(const std::string& leftFileName, const std::string& rightFileName,
                                   const int& numberCornersX,
                                   const int& numberCornersY,
                                   const double& sizeSquareMillimeters,
                                   const mitk::Point2D& pixelScaleFactor,
                                   std::vector<std::string>& successfulLeftFiles, std::vector<std::string>& successfulRightFiles
                                   );


/**
 * \brief Iterates through the list of images, checking that the width and height are consistent.
 * \throw Throws exception if any images are of different size to the first, or the list is empty.
 * \param width output parameter containing the image width for all the images.
 * \param height output parameter containing the image height for all the images.
 */
extern "C++" NIFTKOPENCV_EXPORT void CheckConstImageSize(
  const std::vector<IplImage*>& images, int& width, int& height
  );


/**
 * \brief Extracts the chess board points, using OpenCV routines.
 * \param images vector of pointers to images, which must be all of the same size.
 * \param fileNames the corresponding file names, and this vector must be the same length as the vector of images.
 * \param numberCornersWidth the number of internal corners along the width axis (X).
 * \param numberCornersHeight the number of internal corners along the height axis (Y).
 * \param drawCorners if true will dump images in the same directory as the input images, to indicate which points were found (usefull for debugging).
 * \param squareSizeInMillimetres The size of the chessboard squares in millimetres, needed to make sure that the units of the output camera model are millimetres rather than multiples of the chessboard square size.
 * \param pixelScaleFactor The caller can specify a multiplier for the number of pixels in each direction to scale up/down the image.
 * \param outputImages list of successfully processed images, which are just pointers back to the same images as in the first parameter vector, i.e. they are not copied, so don't de-allocate the images twice.
 * \param outputFileNames corresponding list of successfully processed image filenames,
 * \param outputImagePoints output image points, array size = (number of successes (M) * numberOfCorners (N)) x 2, and caller must de-allocate.
 * \param outputObjectPoints output object points, array size = (number of successes (M) * numberOfCorners (N)) x 3, and caller must de-allocate.
 * \param outputPointCounts output point counts, array size = number of successes (M) x 1, and caller must de-allocate. In this case, a "successful" result is
 * one in which the extraction process retrieved all N points for that chessboard. So, by definition, this array, is M x 1, with each entry containing
 * the number N.
 */
extern "C++" NIFTKOPENCV_EXPORT void ExtractChessBoardPoints(
  const std::vector<IplImage*>& images,
  const std::vector<std::string>& fileNames,
  const int& numberCornersWidth,
  const int& numberCornersHeight,
  const bool& drawCorners,
  const double& squareSizeInMillimetres,
  const mitk::Point2D& pixelScaleFactor,
  std::vector<IplImage*>& outputImages,
  std::vector<std::string>& outputFileNames,
  CvMat*& outputImagePoints,
  CvMat*& outputObjectPoints,
  CvMat*& outputPointCounts
  );


/**
 * \brief Extracts the chess board points, using OpenCV routines.
 * \param image is a single image.
 * \param numberCornersWidth the number of internal corners along the width axis (X).
 * \param numberCornersHeight the number of internal corners along the height axis (Y).
 * \param squareSizeInMillimetres The size of the chessboard squares in millimetres, needed to make sure that the units of the output camera model are millimetres rather than multiples of the chessboard square size.
 * \param pixelScaleFactor The caller can specify a multiplier for the number of pixels in each direction to scale up/down the image.
 * \param outputImagePoints output image points, array size = (1 * numberOfCorners (N)) x 2, and caller must de-allocate.
 * \param outputObjectPoints output object points, array size = (1 * numberOfCorners (N)) x 3, and caller must de-allocate.
 */
extern "C++" NIFTKOPENCV_EXPORT bool ExtractChessBoardPoints(
  const cv::Mat& image,
  const int& numberCornersWidth,
  const int& numberCornersHeight,
  const bool& drawCorners,
  const double& squareSizeInMillimetres,
  const mitk::Point2D& pixelScaleFactor,
  std::vector<cv::Point2d>& outputImagePoints,
  std::vector<cv::Point3d>& outputObjectPoints
  );


/**
 * \brief Calibrate a single camera's intrinsic parameters, by directly calling cvCalibrationCamera2.
 * \param objectPoints [(MxN)x3] list of 3D points generated by ExtractChessBoardPoints, in the objects natural coordinate system.
 * \param imagePoints [(MxN)x2] list of 2D image points generated by ExtractChessBoardPoints.
 * \param pointCounts [(MxN)x1] array containing the number of points, where this array should be the same length as objectPoints and imagePoints, and each entry contain the number of points.
 * \param outputIntrinsicMatrix [3x3] matrix.
 * \param outputDistortionCoefficients [4x1] matrix of [k1, k2, p1, p2].
 * \param flags A bit-wise OR, of zero, CV_CALIB_USE_INTRINSIC_GUESS, CV_CALIB_FIX_PRINCIPAL_POINT, CV_CALIB_FIX_ASPECT_RATIO,
 * CV_CALIB_FIX_FOCAL_LENGTH, CV_CALIB_FIXK1, CV_CALIB_FIXK2, CV_CALIB_FIXK3 and CV_CALIB_ZERO_TANGENT_DIST.
 */
extern "C++" NIFTKOPENCV_EXPORT double CalibrateSingleCameraParameters(
  const CvMat& objectPoints,
  const CvMat& imagePoints,
  const CvMat& pointCounts,
  const CvSize& imageSize,
  CvMat& outputIntrinsicMatrix,
  CvMat& outputDistortionCoefficients,
  CvMat* outputRotationVectors,
  CvMat* outputTranslationVectors,
  const int& flags=0
  );


/**
 * \brief Calibrate a single camera's intrinsic parameters by using 3 passes, firstly with fixed principal point
 * and fixed aspect ratio, then with fixed principal point, then with nothing fixed.
 * \param objectPoints \see CalibrateSingleCameraIntrinsicParameters
 * \param imagePoints \see CalibrateSingleCameraIntrinsicParameters
 * \param pointCounts \see CalibrateSingleCameraIntrinsicParameters
 * \param outputIntrinsicMatrix \see CalibrateSingleCameraIntrinsicParameters
 * \param outputDistortionCoefficients \see CalibrateSingleCameraIntrinsicParameters
 */
extern "C++" NIFTKOPENCV_EXPORT double CalibrateSingleCameraUsingMultiplePasses(
  const CvMat& objectPoints,
  const CvMat& imagePoints,
  const CvMat& pointCounts,
  const CvSize& imageSize,
  CvMat& outputIntrinsicMatrix,
  CvMat& outputDistortionCoefficients,
  CvMat& outputRotationVectors,
  CvMat& outputTranslationVectors
  );


/**
 * \brief Calculates JUST the extrinsic parameters for a whole bunch of calibrations.
 */
extern "C++" NIFTKOPENCV_EXPORT void CalibrateSingleCameraExtrinsics(
  const CvMat& objectPoints,
  const CvMat& imagePoints,
  const CvMat& pointCounts,
  const CvMat& intrinsicMatrix,
  const CvMat& distortionCoefficients,
  const bool& useExtrinsicGuess,
  CvMat& outputRotationVectors,
  CvMat& outputTranslationVectors
  );


/**
 * \brief The above method CalibrateSingleCameraParameters outputs a whole load of rotation and translation vectors,
 * so this utility method reconstructs a single extrinsic parameter matrix, for a given viewNumber.
 * \param rotationVectors [Mx3] matrix of rotation vectors, where M is the number of views of the chess board.
 * \param translationVectors [Mx3] matrix of translation vectors, where M is the number of views of the chess board.
 * \param viewNumber which view to extract, where 0 <= viewNumber < M, and viewNumber is unvalidated (unchecked).
 * \param outputExtrinsicMatrix the output matrix, which should be a pre-allocated 4x4 matrix
 */
extern "C++" NIFTKOPENCV_EXPORT void ExtractExtrinsicMatrixFromRotationAndTranslationVectors(
  const CvMat& rotationVectors,
  const CvMat& translationVectors,
  const int& viewNumber,
  CvMat& outputExtrinsicMatrix
  );


/**
 * \brief Method to take a set of rotation and translation vectors for left and
 * right cameras, and compute transformations from right to left.
 *
 * This means that a given a point P_r in the coordinate frame of the right
 * hand camera, then when this point is multiplied by the rotationVectorsRightToLeft
 * and translationVectorsRightToLeft, the point will be converted to a point P_l
 * that is in the coordinate frame of the left hand camera.
 */
extern "C++" NIFTKOPENCV_EXPORT void ComputeRightToLeftTransformations(
  const CvMat& rotationVectorsLeft,
  const CvMat& translationVectorsLeft,
  const CvMat& rotationVectorsRight,
  const CvMat& translationVectorsRight,
  const CvMat& rotationVectorsRightToLeft,
  const CvMat& translationVectorsRightToLeft
  );


/**
 * \brief Bulk method to project all points for all calibrations back to 2D, useful for validating calibration.
 * \param numberSuccessfulViews the number of successful views from ExtractChessBoardPoints, in this documentation called 'M'.
 * \param pointCount the number of detected points in each view, in this documentation, called 'N'.
 * \param objectPoints [(MxN)x3] list of 3D object points generated as output from ExtractChessBoardPoints.
 * \param intrinsicMatrix [3x3] matrix of pre-initialised intrinsic parameters.
 * \param distortionCoefficients [4x1] matrix of [k1, k2, p1, p2], pre-initialised.
 * \param rotationVectors an [Mx3] matrix of rotation vectors, see also cvRodrigues2 to convert from a rotation vector to a rotation matrix.
 * \param translationVectors an [Mx3] matrix of translation vectors.
 * \param outputImagePoints [(MxN)x2] list of 2D image points generated by calling cvProjectPoints2.
 */
extern "C++" NIFTKOPENCV_EXPORT void ProjectAllPoints(
  const int& numberSuccessfulViews,
  const int& pointCount,
  const CvMat& objectPoints,
  const CvMat& intrinsicMatrix,
  const CvMat& distortionCoeffictions,
  const CvMat& rotationVectors,
  const CvMat& translationVectors,
  CvMat& outputImagePoints
  );

/**
 * \brief Calculates the RMS projection error.
 * \param projectedPoints [Nx2] list of 2D points, measured in pixels.
 * \param goldStandardPoints [Nx2] list of 2D points, measured in pixels.
 */
extern "C++" NIFTKOPENCV_EXPORT double CalculateRPE(
    const CvMat& projectedPoints,
    const CvMat& goldStandardPoints
    );

/**
 * \brief Performs a stereo calibration, including all intrinsic, extrinsic, distortion co-efficients,
 * and also outputs the rotation and translation vector between the two cameras.
 * plus option to fix the intrinsics, so only the extrinsics and r2l transform are calculated
 */
extern "C++" NIFTKOPENCV_EXPORT double CalibrateStereoCameraParameters(
  const CvMat& objectPointsLeft,
  const CvMat& imagePointsLeft,
  const CvMat& pointCountsLeft,
  const CvSize& imageSize,
  const CvMat& objectPointsRight,
  const CvMat& imagePointsRight,
  const CvMat& pointCountsRight,
  CvMat& outputIntrinsicMatrixLeft,
  CvMat& outputDistortionCoefficientsLeft,
  CvMat& outputRotationVectorsLeft,
  CvMat& outputTranslationVectorsLeft,
  CvMat& outputIntrinsicMatrixRight,
  CvMat& outputDistortionCoefficientsRight,
  CvMat& outputRotationVectorsRight,
  CvMat& outputTranslationVectorsRight,
  CvMat& outputRightToLeftRotation,
  CvMat& outputRightToLeftTranslation,
  CvMat& outputEssentialMatrix,
  CvMat& outputFundamentalMatrix,
  const bool& fixedIntrinsics = false,
  const bool& fixedRightToLeft = false
  );


/**
 * \brief Utility method to dump output to a stream.
 * \return RMS projection error, as projected to one camera, for each image in order.
 *
 * i.e. this method is called seperately for both left and right camera.
 */
extern "C++" NIFTKOPENCV_EXPORT std::vector<double> OutputCalibrationData(
  std::ostream& outputStream,
  const std::string& outputDirectoryName,
  const std::string& intrinsicFlatFileName,
  const CvMat& objectPoints,
  const CvMat& imagePoints,
  const CvMat& pointCounts,
  const CvMat& intrinsicMatrix,
  const CvMat& distortionCoefficients,
  const CvMat& rotationVectors,
  const CvMat& translationVectors,
  const double& projectionError,
  const int& sizeX,
  const int& sizeY,
  const int& cornersX,
  const int& cornersY,
  std::vector<std::string>& fileNames
  );


/**
 * \brief Loads an image and parameters and writes the distortion corrected image
 * to the output. Assumes that both the intrinsic camera params and distortion coefficients
 * are in OpenCV's xml format.
 */
extern "C++" NIFTKOPENCV_EXPORT void CorrectDistortionInImageFile(
  const std::string& inputImageFileName,
  const std::string& inputIntrinsicsFileName,
  const std::string& inputDistortionCoefficientsFileName,
  const std::string& outputImageFileName
  );


/**
 * \brief Method that reads a single image (eg. png, jpg or anything that OpenCV recognises)
 * and corrects it using the intrinsic params and distortion co-efficients, and writes
 * it to the output file.
 */
extern "C++" NIFTKOPENCV_EXPORT void CorrectDistortionInImageFile(
  const std::string& inputFileName,
  const CvMat& intrinsicParams,
  const CvMat& distortionCoefficients,
  const std::string& outputFileName
  );


/**
 * \brief Assuming image is pre-allocated, will take the intrinsic and distortion parameters
 * and calculate a pixel-wise undistortion map, and apply it to image.
 */
extern "C++" NIFTKOPENCV_EXPORT void CorrectDistortionInSingleImage(
  const CvMat& intrinsicParams,
  const CvMat& distortionCoefficients,
  IplImage &image
  );


/**
 * \brief Assumes all image buffers are pre-allocated and the same size,
 * and applies mapX and mapY to image.
 */
extern "C++" NIFTKOPENCV_EXPORT void UndistortImageUsingDistortionMap(
  const IplImage &mapX,
  const IplImage &mapY,
  IplImage &image
  );


/**
 * \brief Assumes all image buffers are pre-allocated and the same size,
 * and applies mapX and mapY to the inputImage, and writes to outputImage.
 */
extern "C++" NIFTKOPENCV_EXPORT void ApplyDistortionCorrectionMap(
  const IplImage &mapX,
  const IplImage &mapY,
  const IplImage &inputImage,
  IplImage &outputImage
  );


/**
 * \brief Used to project 3D points into 2D locations for a stereo pair.
 * Here, 3D model points means that the 3D coordinates are with respect to the model's natural
 * coordinate system. So, if for example, you were using a chessboard, the coordinates of
 * each corner are (0,0,0), (1,0,0), (2,0,0) etc. and are independent of the size of the grid in millimetres.
 *
 * \param modelPointsIn3D [Nx3] matrix containing points in model coordinates.
 * \param leftCameraIntrinsic [3x3] matrix of intrinsic parameters, as output from these and OpenCV routines.
 * \param leftCameraDistortion [4x1] matrix of distortion co-efficients, as output from these and OpenCV routines.
 * \param leftCameraRotationVector [1x3] vector representing rotations (see cvRodrigues2 in OpenCV).
 * \param leftCameraTranslationVector [1x3] translation vector, along with leftCameraRotationVector represents the so-called camera extrinsic parameters.
 * \param rightCameraIntrinsic as per left
 * \param rightCameraDistortion as per left
 * \param rightToLeftRotationMatrix [3x3] rotation matrix.
 * \param rightToLeftTranslationVector [1x3] translation vector, which along with rightToLeftRotationMatrix represents the transformation of the right camera to coordinate system of the left camera.
 * \param output2DPointsLeft [Nx3] matrix of the 2D pixel location in left camera
 * \param output2DPointsRight [Nx3] matrix of the 2D pixel location in right camera
 */
extern "C++" NIFTKOPENCV_EXPORT void Project3DModelPositionsToStereo2D(
  const CvMat& modelPointsIn3D,
  const CvMat& leftCameraIntrinsic,
  const CvMat& leftCameraDistortion,
  const CvMat& leftCameraRotationVector,
  const CvMat& leftCameraTranslationVector,
  const CvMat& rightCameraIntrinsic,
  const CvMat& rightCameraDistortion,
  const CvMat& rightToLeftRotationMatrix,
  const CvMat& rightToLeftTranslationVector,
  CvMat& output2DPointsLeft,
  CvMat& output2DPointsRight,
  const bool& cropPointsToScreen = false,
  const double& xLow = 0.0 , const double& xHigh = 0.0 , 
  const double& yLow = 0.0 , const double& yHigh = 0.0 , const double& cropValue = 0.0
  );


/**
 * \brief Takes 3D world points, and normals, and if the normal is pointing towards
 * camera, will project the point to both left and right 2D position using
 * ProjectLeftCamera3DPositionToStereo2D. Here, in this method, the 3D points
 * are assumed to be in true 3D world (mm) coordinates relative to the left camera.
 * So, the left camera extrinsic parameters are not needed.
 * \return point IDs of the valid points
 * \param leftCameraWorldPointsIn3D [Nx3] matrix of 3D world points
 * \param leftCameraWorldNormalsIn3D [Nx3] matrix of 3D world unit vector normals, corresponding to points in leftCameraWorldPointsIn3D
 * \param leftCameraPositionToFocalPointUnitVector [1x3] unit vector pointing from camera position to focal point  (i.e. forwards).
 * \param leftCameraIntrinsic [3x3] matrix of intrinsic parameters, as output from these and OpenCV routines.
 * \param leftCameraDistortion [4x1] matrix of distortion co-efficients, as output from these and OpenCV routines.
 * \param rightCameraIntrinsic as per left
 * \param rightCameraDistortion as per left
 * \param rightToLeftRotationMatrix [3x3] rotation matrix.
 * \param rightToLeftTranslationVector [1x3] translation vector, which along with rightToLeftRotationMatrix represents the transformation of the right camera to coordinate system of the left camera.
 * \param outputLeftCameraWorldPointsIn3D [nx3] newly created matrix, that the user must de-allocate, of 3D points that were actually visible.
 * \param outputLeftCameraWorldNormalsIn3D [nx3] newly created matrix, that the user must de-allocate, of 3D unit normals that were actually visible.
 * \param output2DPointsLeft [nx3] newly created matrix, that the user must de-allocate of the 2D pixel location in left camera
 * \param output2DPointsRight [nx3] newly created matrix, that the user must de-allocate of the 2D pixel location in right camera
 * \param cropPointsToScreen optionally you can crop the output points to only use points that 
 * fit with set limits. This is useful if it possible to pass very large valued input points
 * as in this situation the underlying cv::undistortPoints will return the image principal point.
 * With this parameter set the cropValue will be returned instead.
 */
extern "C++" NIFTKOPENCV_EXPORT std::vector<int> ProjectVisible3DWorldPointsToStereo2D(
  const CvMat& leftCameraWorldPointsIn3D,
  const CvMat& leftCameraWorldNormalsIn3D,
  const CvMat& leftCameraPositionToFocalPointUnitVector,
  const CvMat& leftCameraIntrinsic,
  const CvMat& leftCameraDistortion,
  const CvMat& rightCameraIntrinsic,
  const CvMat& rightCameraDistortion,
  const CvMat& rightToLeftRotationMatrix,
  const CvMat& rightToLeftTranslationVector,
  CvMat*& outputLeftCameraWorldPointsIn3D,
  CvMat*& outputLeftCameraWorldNormalsIn3D,
  CvMat*& output2DPointsLeft,
  CvMat*& output2DPointsRight,
  const bool& cropPointsToScreen = false,
  const double& xLow = 0.0 , const double& xHigh = 0.0 , 
  const double& yLow = 0.0 , const double& yHigh = 0.0 , const double& cropValue = 0.0
  );


/**
 * \brief Takes image points, and undistorts them to ideal locations.
 * \param inputObservedPointsNx2 [Nx2] matrix of (x,y) points, as observed in a distorted image.
 * \param cameraIntrinsics3x3 [3x3] matrix of camera intrisic parameters.
 * \param cameraDistortionParams5x1 [5x1] camera distortion params.
 * \param outputIdealPointsNx2 [Nx2] matrix of (x,y) points, as ideal locations in an undistorted image.
 * \param cropPointsToScreen optionally you can crop the output points to only use points that 
 * fit with set limits. This is useful if it possible to pass very large valued input points
 * as in this situation the underlying cv::undistortPoints will return the image principal point.
 * With this parameter set the cropValue will be returned instead.
 */
extern "C++" NIFTKOPENCV_EXPORT void UndistortPoints(const cv::Mat& inputObservedPointsNx2,
  const cv::Mat& cameraIntrinsics3x3,
  const cv::Mat& cameraDistortionParams5x1,
  cv::Mat& outputIdealPointsNx2,
  const bool& cropPointsToScreen = false,
  const double& xLow = 0.0 , const double& xHigh = 0.0 , 
  const double& yLow = 0.0 , const double& yHigh = 0.0 , const double& cropValue = 0.0
  );


/**
 * \brief Takes image points, and undistorts them to ideal locations, which is a C++ wrapper for the above method.
 * \param inputObservedPoints vector of (x,y) points, as observed in a distorted image
 * \param cameraIntrinsics3x3 [3x3] matrix of camera intrisic parameters.
 * \param cameraDistortionParams4x1 [4x1] camera distortion params.
 * \param outputIdealPoints vector of (x,y) points, as ideal locations in an undistorted image
 * \param cropPointsToScreen optionally you can crop the output points to only use points that 
 * fit with set limits. This is useful if it possible to pass very large valued input points
 * as in this situation the underlying cv::undistortPoints will return the image principal point.
 * With this parameter set the cropValue will be returned instead.
 */
extern "C++" NIFTKOPENCV_EXPORT void UndistortPoints(const std::vector<cv::Point2d>& inputObservedPoints,
  const cv::Mat& cameraIntrinsics3x3,
  const cv::Mat& cameraDistortionParams5x1,
  std::vector<cv::Point2d>& outputIdealPoints,
  const bool& cropPointsToScreen = false,
  const double& xLow = 0.0 , const double& xHigh = 0.0 , 
  const double& yLow = 0.0 , const double& yHigh = 0.0 , const double& cropValue = 0.0
  );


/**
 * \brief Takes an image point, and undistorts it to ideal locations, which is a C++ wrapper for the above method.
 * \param inputObservedPoints vector of (x,y) points, as observed in a distorted image
 * \param cameraIntrinsics3x3 [3x3] matrix of camera intrisic parameters.
 * \param cameraDistortionParams5x1 [5x1] camera distortion params.
 * \param outputIdealPoints vector of (x,y) points, as ideal locations in an undistorted image
 * \param cropPointsToScreen optionally you can crop the output points to only use points that 
 * fit with set limits. This is useful if it possible to pass very large valued input points
 * as in this situation the underlying cv::undistortPoints will return the image principal point.
 * With this parameter set the cropValue will be returned instead.
 */
extern "C++" NIFTKOPENCV_EXPORT void UndistortPoint(const cv::Point2d& inputObservedPoint,
  const cv::Mat& cameraIntrinsics3x3,
  const cv::Mat& cameraDistortionParams,
  cv::Point2d& outputIdealPoint,
  const bool& cropPointsToScreen = false,
  const double& xLow = 0.0 , const double& xHigh = 0.0 , 
  const double& yLow = 0.0 , const double& yHigh = 0.0 , const double& cropValue = 0.0
  );


/**
 * \brief Triangulates a vector of un-distorted (i.e. already correction for distortion) 2D point pairs back into 3D.
 *
 * Taken from: http://geomalgorithms.com/a07-_distance.html
 *
 * \param rightToLeftRotationMatrix [3x3] matrix representing the rotation between camera axes
 * \param rightToLeftTranslationVector [1x3] translation between camera origins
 * \param tolerance if the distance between the midpoint of the two intersected rays, and a ray is
 * greater than the tolerance, the point is rejected.
 */
extern "C++" NIFTKOPENCV_EXPORT std::vector< cv::Point3d > TriangulatePointPairsUsingGeometry(
  const std::vector< std::pair<cv::Point2d, cv::Point2d> >& inputUndistortedPoints,
  const cv::Mat& leftCameraIntrinsicParams,
  const cv::Mat& rightCameraIntrinsicParams,
  const cv::Mat& rightToLeftRotationMatrix,
  const cv::Mat& rightToLeftTranslationVector,
  const double& tolerance
  );


/**
 * \brief Triangulates a single point from two 2D points by calling TriangulatePointPairs().
 *
 * \param rightToLeftRotation<Matrix [3x3] vector representing the rotation between camera axes
 * \param rightToLeftTranslationVector [1x3] translation between camera origins
 */
extern "C++" NIFTKOPENCV_EXPORT cv::Point3d TriangulatePointPairUsingGeometry(
  const std::pair<cv::Point2d, cv::Point2d> & inputUndistortedPoints,
  const cv::Mat& leftCameraIntrinsicParams,
  const cv::Mat& rightCameraIntrinsicParams,
  const cv::Mat& rightToLeftRotationMatrix,
  const cv::Mat& rightToLeftTranslationVector
  );


/**
 * \brief C Wrapper for the other TriangulatePointPairs.
 */
extern "C++" NIFTKOPENCV_EXPORT void CStyleTriangulatePointPairsUsingSVD(
  const CvMat& leftCameraUndistortedImagePoints,
  const CvMat& rightCameraUndistortedImagePoints,
  const CvMat& leftCameraIntrinsicParams,
  const CvMat& leftCameraRotationVector,
  const CvMat& leftCameraTranslationVector,
  const CvMat& rightCameraIntrinsicParams,
  const CvMat& rightCameraRotationVector,
  const CvMat& rightCameraTranslationVector,
  CvMat& output3DPoints
  );

/** 
 * \brief Reprojects undistorted  screen points to normalised points (x/z, y/z, 1.0) in lens coordinates.
 */
extern "C++" NIFTKOPENCV_EXPORT cv::Point3d ReProjectPoint (
    const cv::Point2d& inputUndistortedPoint, const cv::Mat& CameraIntrinsicParams);

/**
 * \brief Triangulates a vector of un-distorted (i.e. already correction for distortion) 2D point pairs back into 3D.
 *
 * NOTE: This is only valid up to an indeterminant scale factor.
 *
 * From "Triangulation", Hartley, R.I. and Sturm, P., Computer vision and image understanding, 1997.
 * and
 * <a href="http://www.morethantechnical.com/2012/01/04/simple-triangulation-with-opencv-from-harley-zisserman-w-code/">here</a>.
 * and
 * Price 2012, Computer Vision: Models, Learning and Inference.
 *
 * \param leftCameraIntrinsicParams [3x3] matrix for left camera, as output by OpenCV and routines in this file.
 * \param leftCameraRotationVector [1x3] matrix for the extrinsic parameters rotation vector.
 * \param leftCameraTranslationVector [1x3] matrix for the extrinsic parameters translation vector.
 * \param rightCameraIntrinsicParams [3x3] matrix for right camera, as output by OpenCV and routines in this file.
 * \param rightCameraRotationVector [1x3] matrix for the extrinsic parameters rotation vector.
 * \param rightCameraTranslationVector [1x3] matrix for the extrinsic parameters translation vector.
 * \param outputPoints reconstructed 3D points, but only reconstructed up to a an indeterminant scale factor.
 */
extern "C++" NIFTKOPENCV_EXPORT std::vector< cv::Point3d > TriangulatePointPairsUsingSVD(
  const std::vector< std::pair<cv::Point2d, cv::Point2d> >& inputUndistortedPoints,
  const cv::Mat& leftCameraIntrinsicParams,
  const cv::Mat& leftCameraRotationVector,
  const cv::Mat& leftCameraTranslationVector,
  const cv::Mat& rightCameraIntrinsicParams,
  const cv::Mat& rightCameraRotationVector,
  const cv::Mat& rightCameraTranslationVector
  );


/**
 * \brief Don't call this: Triangulates a 3D point using SVD.
 *
 * \param P1 left camera matrix, meaning a full perspective projection, including extrinsic and intrinsic.
 * \param P2 right camera matrix, meaning a full perspective projection, including extrinsic and intrinsic.
 * \param u1 normalised left camera image coordinate in pixels.
 * \param u2 normalised right camera image coordinate in pixels.
 */
cv::Mat_<double> InternalTriangulatePointUsingSVD(
  const cv::Matx34d& P1,
  const cv::Matx34d& P2,
  const cv::Point3d& u1,
  const cv::Point3d& u2,
  const double& w1,
  const double& w2
  );


/**
 * \brief Don't call this: Triangulates a 3D point using SVD by calling TriangulatePointUsingSVD with different weighting factors.
 *
 * \param P1 left camera matrix, meaning a full perspective projection, including extrinsic and intrinsic.
 * \param P2 right camera matrix, meaning a full perspective projection, including extrinsic and intrinsic.
 * \param u1 normalised left camera image coordinate in pixels.
 * \param u2 normalised right camera image coordinate in pixels.
 */
cv::Point3d InternalIterativeTriangulatePointUsingSVD(
  const cv::Matx34d& P1,
  const cv::Matx34d& P2,
  const cv::Point3d& u1,
  const cv::Point3d& u2
  );

/**
 * \brief Read a set of matrices, stored as plain text, 4x4 matrices from a directory and 
 * put them in a vector of 4x4 cvMats
 */
extern "C++" NIFTKOPENCV_EXPORT std::vector<cv::Mat> LoadMatricesFromDirectory (const std::string& fullDirectoryName);


/**
 * \brief Read a set of matrices, stored in openCV xml matrix format from a directory and
 * put them in a vector of 4x4 cvMats
 */
extern "C++" NIFTKOPENCV_EXPORT std::vector<cv::Mat> LoadOpenCVMatricesFromDirectory (const std::string& fullDirectoryName);


 /**
  * \brief Load a set of matrices from a file describing the 
  * extrinsic parameters of a standard camera calibration
  */
extern "C++" NIFTKOPENCV_EXPORT std::vector<cv::Mat> LoadMatricesFromExtrinsicFile (const std::string& fullFileName);


/**
  * \brief Load stereo camera parameters from a directory
  */
extern "C++" NIFTKOPENCV_EXPORT void LoadStereoCameraParametersFromDirectory (const std::string& directory,
  cv::Mat* leftCameraIntrinsic, cv::Mat* leftCameraDistortion,
  cv::Mat* rightCameraIntrinsic, cv::Mat* rightCameraDisortion,
  cv::Mat* rightToLeftRotationMatrix, cv::Mat* rightToLeftTranslationVector,
  cv::Mat* leftCameraToTracker);

/**
 * \brief reads the handeye and r2l transforms and writes out a set of left, centre and 
 * right hand eye matrices, useful for generating geometry for the scope
 */
extern "C++" NIFTKOPENCV_EXPORT void GenerateFullHandeyeMatrices (const std::string& directory);

/**
 * \brief Load camera intrinsics from a plain text file and return results as
 * cv::Mat
 * \param cameraIntrinsic 3x3 matrix (double!)
 * \param cameraDistortion is optional, number of components needs to match the file! (double!)
 * \throws exception if parsing fails for any reason.
 */
extern "C++" NIFTKOPENCV_EXPORT void LoadCameraIntrinsicsFromPlainText ( const std::string& filename,
  cv::Mat* cameraIntrinsic, cv::Mat* cameraDistortion);


/**
 * \brief Load stereo camera parameters from a plain text file
 * cv::Mat
 * \throws exception if parsing fails for any reason.
 */
extern "C++" NIFTKOPENCV_EXPORT void LoadStereoTransformsFromPlainText ( const std::string& filename,
  cv::Mat* rightToLeftRotationMatrix, cv::Mat* rightToLeftTranslationVector);


/**
 * \brief Load the handeye matrix from a plain text file
 * cv::Mat
 */
extern "C++" NIFTKOPENCV_EXPORT void LoadHandeyeFromPlainText ( const std::string& filename,
    cv::Mat* leftCameraToTracker);

 /**
  * \brief loads a result file into a residual vector and matrix
  */
extern "C++" NIFTKOPENCV_EXPORT void LoadResult(const std::string& Filename, cv::Mat& result,
  std::vector<double>& residuals);


 /** 
  * \brief Transforms a point relative to the left camera lens to 
  * world coordinates using the handeye and tracking matrices
  */
extern "C++" NIFTKOPENCV_EXPORT cv::Point3f LeftLensToWorld ( cv::Point3f pointInLensCS,
  cv::Mat& handeye, cv::Mat& tracker );


/** 
  * \brief Transforms a point in world coordinates to a point 
  * relative to the left lens using
  * world coordinates using the handeye and tracking matrices
  */
extern "C++" NIFTKOPENCV_EXPORT cv::Point3f WorldToLeftLens ( cv::Point3f pointInWorldCS,
  cv::Mat& handeye, cv::Mat& tracker );

/**
 * \brief Iterates through a vector of points and checks whether they are within the bounds
 * passed. If out of bounds the corresponding value in the destination vector is set to 
 * the passed value
 */
extern "C++" NIFTKOPENCV_EXPORT void CropToScreen ( const std::vector <cv::Point2d>& srcPoints,
    std::vector <cv::Point2d>& dstPoints , 
    const double& xLow, const double& xHigh, const double& yLow, const double& yHigh,
    const double& cropValue );

} // end namespace

#endif
