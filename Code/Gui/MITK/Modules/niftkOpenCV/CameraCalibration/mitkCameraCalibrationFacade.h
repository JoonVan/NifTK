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

/**
 * \file mitkCameraCalibrationFacade
 * \brief Interface to OpenCV camera calibration, and associated routines.
 */
namespace mitk {

/**
 * \brief Uses OpenCV to load chessboard images from a directory.
 * \throw Throws logic_error if fullDirectoryName is not a valid directory,
 * the directory contains no files, or there are no files that are images that OpenCV recognises.
 * \param images output parameter containing pointers to the images, which the caller must take responsibility for and de-allocate appropriately.
 * \param fileNames output vector containing the corresponding filenames.
 */
 void LoadChessBoardsFromDirectory(const std::string& fullDirectoryName,
                                   std::vector<IplImage*>& images,
                                   std::vector<std::string>& fileNames);


/**
 * \brief Iterates through the list of images, checking that the width and height are consistent.
 * \throw Throws logic_error if any images are of different size to the first, or the list is empty.
 * \param width output parameter containing the image width for all the images.
 * \param height output parameter containing the image height for all the images.
 */
void CheckConstImageSize(const std::vector<IplImage*>& images, int& width, int& height);


/**
 * \brief Extracts the chess board points, using OpenCV routines.
 * \param images vector of pointers to images, which must be all of the same size.
 * \param fileNames the corresponding file names, and this vector must be the same length as the vector of images.
 * \param numberCornersWidth the number of internal corners along the width axis (X).
 * \param numberCornersHeight the number of internal corners along the height axis (Y).
 * \param drawCorners if true will dump images in the same directory as the input images, to indicate which points were found (usefull for debugging).
 * \param squareSizeInMillimetres The size of the chessboard squares in millimetres, needed to make sure that the units of the output camera model are millimetres rather than multiples of the chessboard square size.
 * \param outputImages list of successfully processed images, which are just pointers back to the same images as in the first parameter vector, i.e. they are not copied, so don't de-allocate the images twice.
 * \param outputFileNames corresponding list of successfully processed image filenames,
 * \param outputImagePoints output image points, array size = (number of successes (M) * numberOfCorners (N)) x 2, and caller must de-allocate.
 * \param outputObjectPoints output object points, array size = (number of successes (M) * numberOfCorners (N)) x 3, and caller must de-allocate.
 * \param outputPointCounts output point counts, array size = number of successes (M) x 1, and caller must de-allocate. In this case, a "successful" result is
 * one in which the extraction process retrieved all N points for that chessboard. So, by definition, this array, is M x 1, with each entry containing
 * the number N.
 */
void ExtractChessBoardPoints(const std::vector<IplImage*>& images,
                             const std::vector<std::string>& fileNames,
                             const int& numberCornersWidth,
                             const int& numberCornersHeight,
                             const bool& drawCorners,
                             const double& squareSizeInMillimetres,
                             std::vector<IplImage*>& outputImages,
                             std::vector<std::string>& outputFileNames,
                             CvMat*& outputImagePoints,
                             CvMat*& outputObjectPoints,
                             CvMat*& outputPointCounts
                             );
/**
 * \brief Extracts the chess board points, using OpenCV routines.
 * \param images is a single image.
 * \param numberCornersWidth the number of internal corners along the width axis (X).
 * \param numberCornersHeight the number of internal corners along the height axis (Y).
 * \param squareSizeInMillimetres The size of the chessboard squares in millimetres, needed to make sure that the units of the output camera model are millimetres rather than multiples of the chessboard square size.
 * \param outputImagePoints output image points, array size = (1 * numberOfCorners (N)) x 2, and caller must de-allocate.
 * \param outputObjectPoints output object points, array size = (1 * numberOfCorners (N)) x 3, and caller must de-allocate.
 * \param outputPointCounts output point counts, array size = 1 x 1, and caller must de-allocate. In this case, a "successful" result is
 * one in which the extraction process retrieved all N points for that chessboard. So, by definition, this array, is 1 x 1, with each entry containing
 * the number N.
 */
bool ExtractChessBoardPoints(const cv::Mat  image,
                             const int& numberCornersWidth,
                             const int& numberCornersHeight,
                             const bool& drawCorners,
                             const double& squareSizeInMillimetres,
                             std::vector<cv::Point2f>*& outputImagePoints,
                             std::vector<cv::Point3f>*& outputObjectPoints
                             );



/**
 * \brief Calibrate a single camera's intrinsic parameters, by directly calling cvCalibrationCamera2.
 * \param objectPoints [(MxN)x3] list of 3D points generated by ExtractChessBoardPoints, in the objects natural coordinate system.
 * \param imagePoints [(MxN)x2] list of 2D image points generated by ExtractChessBoardPoints.
 * \param pointCounts [(MxN)x1] array containing the number of points, where this array should be the same length as objectPoints and imagePoints, and each entry contain the number of points.
 * \param outputIntrinsicMatrix [3x3] matrix.
 * \param outputDistortionCoefficients [5x1] matrix of [k1, k2, p1, p2, k3].
 * \param flags A bit-wise OR, of zero, CV_CALIB_USE_INTRINSIC_GUESS, CV_CALIB_FIX_PRINCIPAL_POINT, CV_CALIB_FIX_ASPECT_RATIO,
 * CV_CALIB_FIX_FOCAL_LENGTH, CV_CALIB_FIXK1, CV_CALIB_FIXK2, CV_CALIB_FIXK3 and CV_CALIB_ZERO_TANGENT_DIST.
 */
double CalibrateSingleCameraIntrinsicParameters(
       const CvMat& objectPoints,
       const CvMat& imagePoints,
       const CvMat& pointCounts,
       const CvSize& imageSize,
       CvMat& outputIntrinsicMatrix,
       CvMat& outputDistortionCoefficients,
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
double CalibrateSingleCameraIntrinsicUsing3Passes(
       const CvMat& objectPoints,
       const CvMat& imagePoints,
       const CvMat& pointCounts,
       const CvSize& imageSize,
       CvMat& outputIntrinsicMatrix,
       CvMat& outputDistortionCoefficients
       );


/**
 * \brief Calibrate a single cameras extrinsic parameters.
 * \param objectPoints [Nx3] list of 3D points for 1 image view of a chess board.
 * \param imagePoints [Nx2] list of 2D image points for 1 image view of a chess board.
 * \param intrinsicMatrix [3x3] matrix of pre-initialised intrinsic parameters.
 * \param distortionCoefficients [5x1] matrix of [k1, k2, p1, p2, k3], pre-initialised.
 * \param outputRotationMatrix [3x3] rotation matrix - see OpenCV docs - for this function its a 3x3 not a 1x3.
 * \param outputTranslationVector [1x3] translation vector.
 */
void CalibrateSingleCameraExtrinsicParameters(
     const CvMat& objectPoints,
     const CvMat& imagePoints,
     const CvMat& intrinsicMatrix,
     const CvMat& distortionCoefficients,
     CvMat& outputRotationMatrix,
     CvMat& outputTranslationVector
     );


/**
 * \brief Calibrate a single camera for both intrinsic and extrinsic parameters, using number of views = M, and number of corners per view = N.
 * \param objectPoints [(MxN)x3] list of 3D points generated as output from ExtractChessBoardPoints, for all M views.
 * \param imagePoints [(MxN)x2] list of 2D image points generated as output from ExtractChessBoardPoints for all M views.
 * \param pointCounts [Mx1] array containing the number of points that matched, which should all be equal for all M views, and contain the number N.
 * \param outputIntrinsicMatrix [3x3] matrix.
 * \param outputDistortionCoefficients [5x1] matrix of [k1, k2, p1, p2, k3].
 * \param outputRotationMatrix an [Mx3] matrix of rotation vectors, see also cvRodrigues2 to convert from a rotation vector to a rotation matrix.
 * \param outputTranslationVector an [Mx3] matrix of translation vectors.
 */
double CalibrateSingleCameraParameters(
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
 * \brief The above method CalibrateSingleCameraParameters outputs a whole load of rotation and translation vectors,
 * so this utility method reconstructs a single extrinsic parameter matrix, for a given viewNumber.
 * \param rotationVectors [Mx3] matrix of rotation vectors, where M is the number of views of the chess board.
 * \param translationVectors [Mx3] matrix of translation vectors, where M is the number of views of the chess board.
 * \param viewNumber which view to extract, where 0 <= viewNumber < M, and viewNumber is unvalidated (unchecked).
 * \param outputExtrinsicMatrix the output matrix, which should be a pre-allocated 4x4 matrix
 */
void ExtractExtrinsicMatrixFromRotationAndTranslationVectors(
    const CvMat& rotationVectors,
    const CvMat& translationVectors,
    const int& viewNumber,
    CvMat& outputExtrinsicMatrix
    );


/**
 * \brief Method to take a set of rotation and translation vectors for left and
 * right cameras, and compute transformations from right to left.
 */
void ComputeRightToLeftTransformations(
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
 * \param distortionCoefficients [5x1] matrix of [k1, k2, p1, p2, k3], pre-initialised.
 * \param rotationVectors an [Mx3] matrix of rotation vectors, see also cvRodrigues2 to convert from a rotation vector to a rotation matrix.
 * \param translationVectors an [Mx3] matrix of translation vectors.
 * \param outputImagePoints [(MxN)x2] list of 2D image points generated by calling cvProjectPoints2.
 */
void ProjectAllPoints(
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
 * \brief Performs a stereo calibration, including all intrinsic, extrinsic, distortion co-efficients,
 * and also outputs the rotation and translation vector between the two cameras.
 */
double CalibrateStereoCameraParameters(
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
    CvMat& outputFundamentalMatrix
    );

/**
 * \brief Utility method to dump output to a stream.
 */
void OutputCalibrationData(
    std::ostream& outputStream,
    const std::string intrinsicFlatFileName,
    const CvMat& objectPoints,
    const CvMat& imagePoints,
    const CvMat& pointCounts,
    const CvMat& intrinsicMatrix,
    const CvMat& distortionCoefficients,
    const CvMat& rotationVectors,
    const CvMat& translationVectors,
    const float& projectionError,
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
void CorrectDistortionInImageFile(
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
void CorrectDistortionInImageFile(
    const std::string& inputFileName,
    const CvMat& intrinsicParams,
    const CvMat& distortionCoefficients,
    const std::string& outputFileName
    );


/**
 * \brief Assuming image is pre-allocated, will take the intrinsic and distortion parameters
 * and calculate a pixel-wise undistortion map, and apply it to image.
 */
void CorrectDistortionInSingleImage(
    const CvMat& intrinsicParams,
    const CvMat& distortionCoefficients,
    IplImage &image
    );


/**
 * \brief Assumes all image buffers are pre-allocated and the same size,
 * and applies mapX and mapY to image.
 */
void UndistortImageUsingDistortionMap(
    const IplImage &mapX,
    const IplImage &mapY,
    IplImage &image
    );


/**
 * \brief Assumes all image buffers are pre-allocated and the same size,
 * and applies mapX and mapY to the inputImage, and writes to outputImage.
 */
void ApplyDistortionCorrectionMap(
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
 * \param leftCameraDistortion [5x1] matrix of distortion co-efficients, as output from these and OpenCV routines.
 * \param leftCameraRotationVector [1x3] vector representing rotations (see cvRodrigues2 in OpenCV).
 * \param leftCameraTranslationVector [1x3] translation vector, along with leftCameraRotationVector represents the so-called camera extrinsic parameters.
 * \param rightCameraIntrinsic as per left
 * \param rightCameraDistortion as per left
 * \param rightToLeftRotationMatrix [3x3] rotation matrix.
 * \param rightToLeftTranslationVector [1x3] translation vector, which along with rightToLeftRotationMatrix represents the transformation of the right camera to coordinate system of the left camera.
 * \param output2DPointsLeft [Nx3] matrix of the 2D pixel location in left camera
 * \param output2DPointsRight [Nx3] matrix of the 2D pixel location in right camera
 */
void Project3DModelPositionsToStereo2D(
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
    CvMat& output2DPointsRight
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
 * \param leftCameraDistortion [5x1] matrix of distortion co-efficients, as output from these and OpenCV routines.
 * \param rightCameraIntrinsic as per left
 * \param rightCameraDistortion as per left
 * \param rightToLeftRotationMatrix [3x3] rotation matrix.
 * \param rightToLeftTranslationVector [1x3] translation vector, which along with rightToLeftRotationMatrix represents the transformation of the right camera to coordinate system of the left camera.
 * \param outputLeftCameraWorldPointsIn3D [nx3] newly created matrix, that the user must de-allocate, of 3D points that were actually visible.
 * \param outputLeftCameraWorldNormalsIn3D [nx3] newly created matrix, that the user must de-allocate, of 3D unit normals that were actually visible.
 * \param output2DPointsLeft [nx3] newly created matrix, that the user must de-allocate of the 2D pixel location in left camera
 * \param output2DPointsRight [nx3] newly created matrix, that the user must de-allocate of the 2D pixel location in right camera
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
    CvMat*& output2DPointsRight
    );


/**
 * \brief Takes image points, and undistorts them to ideal locations.
 * \param inputObservedPointsNx2 [Nx2] matrix of (x,y) points, as observed in a distorted image.
 * \param cameraIntrinsics3x3 [3x3] matrix of camera intrisic parameters.
 * \param cameraDistortionParams5x1 [5x1] camera distortion params.
 * \param outputIdealPointsNx2 [Nx2] matrix of (x,y) points, as ideal locations in an undistorted image.
 */
extern "C++" NIFTKOPENCV_EXPORT void UndistortPoints(const cv::Mat& inputObservedPointsNx2,
    const cv::Mat& cameraIntrinsics3x3,
    const cv::Mat& cameraDistortionParams5x1,
    cv::Mat& outputIdealPointsNx2
    );


/**
 * \brief Takes image points, and undistorts them to ideal locations, which is a C++ wrapper for the above method.
 * \param inputObservedPoints vector of (x,y) points, as observed in a distorted image
 * \param cameraIntrinsics3x3 [3x3] matrix of camera intrisic parameters.
 * \param cameraDistortionParams5x1 [5x1] camera distortion params.
 * \param outputIdealPoints vector of (x,y) points, as ideal locations in an undistorted image
 */
void UndistortPoints(const std::vector<cv::Point2f>& inputObservedPoints,
    const cv::Mat& cameraIntrinsics3x3,
    const cv::Mat& cameraDistortionParams5x1,
    std::vector<cv::Point2f>& outputIdealPoints
    );

/**
 * \brief Takes an image point, and undistorts it to ideal locations, which is a C++ wrapper for the above method.
 * \param inputObservedPoints vector of (x,y) points, as observed in a distorted image
 * \param cameraIntrinsics3x3 [3x3] matrix of camera intrisic parameters.
 * \param cameraDistortionParams5x1 [5x1] camera distortion params.
 * \param outputIdealPoints vector of (x,y) points, as ideal locations in an undistorted image
 */
extern "C++" NIFTKOPENCV_EXPORT void UndistortPoint(const cv::Point2f& inputObservedPoint,
    const cv::Mat& cameraIntrinsics3x3,
    const cv::Mat& cameraDistortionParams5x1,
    cv::Point2f& outputIdealPoint
    );



/**
 * \brief Triangulates a vector of un-distorted (i.e. already correction for distortion) 2D point pairs back into 3D.
 *
 * Taken from: http://geomalgorithms.com/a07-_distance.html
 *
 * \param rightToLeftRotationVector [1x3] vector representing the rotation between camera axes
 * \param rightToLeftTranslationVector [1x3] translation between camera origins
 */
std::vector< cv::Point3f > TriangulatePointPairs(
    const std::vector< std::pair<cv::Point2f, cv::Point2f> >& inputUndistortedPoints,
    const cv::Mat& leftCameraIntrinsicParams,
    const cv::Mat& rightCameraIntrinsicParams,
    const cv::Mat& rightToLeftRotationVector,
    const cv::Mat& rightToLeftTranslationVector
    );

/**
 * \brief Triangulates an undistorted (i.e. already correction for distortion) 2D point pair back into 3D.
 *
 * Taken from: http://geomalgorithms.com/a07-_distance.html
 *
 * \param rightToLeftRotation<Matrix [3x3] vector representing the rotation between camera axes
 * \param rightToLeftTranslationVector [1x3] translation between camera origins
 */
extern "C++" NIFTKOPENCV_EXPORT cv::Point3f TriangulatePointPair(
    const std::pair<cv::Point2f, cv::Point2f> & inputUndistortedPoints,
    const cv::Mat& leftCameraIntrinsicParams,
    const cv::Mat& rightCameraIntrinsicParams,
    const cv::Mat& rightToLeftRotationMatrix,
    const cv::Mat& rightToLeftTranslationVector
    );


/**
 * \brief C Wrapper for the other TriangulatePointPairs.
 */
void TriangulatePointPairs(
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
std::vector< cv::Point3f > TriangulatePointPairs(
    const std::vector< std::pair<cv::Point2f, cv::Point2f> >& inputUndistortedPoints,
    const cv::Mat& leftCameraIntrinsicParams,
    const cv::Mat& leftCameraRotationVector,
    const cv::Mat& leftCameraTranslationVector,
    const cv::Mat& rightCameraIntrinsicParams,
    const cv::Mat& rightCameraRotationVector,
    const cv::Mat& rightCameraTranslationVector
    );


/**
 * \brief Triangulates a 3D point.
 *
 * NOTE: This is only valid up to an indeterminant scale factor.
 *
 * From "Triangulation", Hartley, R.I. and Sturm, P., Computer vision and image understanding, 1997.
 * and
 * <a href="http://www.morethantechnical.com/2012/01/04/simple-triangulation-with-opencv-from-harley-zisserman-w-code/">here</a>.
 * and
 * <a href="http://www.amazon.co.uk/Computer-Vision-Models-Learning-Inference/dp/1107011795/ref=sr_1_1?ie=UTF8&qid=1362560709&sr=8-1">Price 2012<a>.
 *
 * This method is called repeatedly from IterativeTriangulatePoint, with different weights (w1 and w2).
 * Users should call IterativeTriangulatePoint.
 *
 * \param P1 left camera matrix, meaning a full perspective projection, including extrinsic and intrinsic.
 * \param P2 right camera matrix, meaning a full perspective projection, including extrinsic and intrinsic.
 * \param u1 normalised left camera image coordinate in pixels.
 * \param u2 normalised right camera image coordinate in pixels.
 */
cv::Mat_<double> TriangulatePoint(
    const cv::Matx34d& P1,
    const cv::Matx34d& P2,
    const cv::Point3d& u1,
    const cv::Point3d& u2,
    const double& w1,
    const double& w2
    );


/**
 * \brief Triangulates a 3D point.
 *
 * From "Triangulation", Hartley, R.I. and Sturm, P., Computer vision and image understanding, 1997.
 * and
 * <a href="http://www.morethantechnical.com/2012/01/04/simple-triangulation-with-opencv-from-harley-zisserman-w-code/">here</a>.
 * and
 * <a href="http://www.amazon.co.uk/Computer-Vision-Models-Learning-Inference/dp/1107011795/ref=sr_1_1?ie=UTF8&qid=1362560709&sr=8-1">Price 2012<a>.
 *
 * \param P1 left camera matrix, meaning a full perspective projection, including extrinsic and intrinsic.
 * \param P2 right camera matrix, meaning a full perspective projection, including extrinsic and intrinsic.
 * \param u1 normalised left camera image coordinate in pixels.
 * \param u2 normalised right camera image coordinate in pixels.
 */
cv::Point3d IterativeTriangulatePoint(
    const cv::Matx34d& P1,
    const cv::Matx34d& P2,
    const cv::Point3d& u1,
    const cv::Point3d& u2
    );

/**
 * \brief Returns the angular distance between two rotation matrices
 */
double AngleBetweenMatrices(cv::Mat Mat1 , cv::Mat Mat2);

/**
 * \brief Converts a 3x3 rotation matrix to a quaternion
 */
cv::Mat DirectionCosineToQuaternion(cv::Mat dc_Matrix);

/**
 * \brief Returns -1.0 if value < 0 or 1.0 if value >= 0
 */
double ModifiedSignum(double value);

/**
 * \brief Returns 0.0 of value < 0 or sqrt(value) if value >= 0
 */
double SafeSQRT(double value);

/**
 * \brief Read a set of matrices, stored as plain text, 4x4 matrices from a directory and 
 * put them in a vector of 4x4 cvMats
 */
std::vector<cv::Mat> LoadMatricesFromDirectory (const std::string& fullDirectoryName);

/**
 * \brief Read a set of matrices, stored in openCV xml matrix format from a directory and
 * put them in a vector of 4x4 cvMats
 */
std::vector<cv::Mat> LoadOpenCVMatricesFromDirectory (const std::string& fullDirectoryName);

 /**
  * \brief Load a set of matrices from a file describing the 
  * extrinsic parameters of a standard camera calibration
  */
std::vector<cv::Mat> LoadMatricesFromExtrinsicFile (const std::string& fullFileName);

/**
  * \brief Load stereo camera parameters from a directory
  */
extern "C++" NIFTKOPENCV_EXPORT void LoadStereoCameraParametersFromDirectory (const std::string& directory,
    cv::Mat* leftCameraIntrinsic, cv::Mat* leftCameraDistortion, 
    cv::Mat* rightCameraIntrinsic, cv::Mat* rightCameraDisortion, 
    cv::Mat* rightToLeftRotationMatrix, cv::Mat* rightToLeftTranslationVector,
    cv::Mat* leftCameraToTracker);

/**
 * \brief Load camera intrinsics from a plain text file and return results as
 * cv::Mat
 */
void LoadCameraIntrinsicsFromPlainText ( const std::string& filename, 
    cv::Mat* CameraIntrinsic, cv::Mat* CameraDistortion); 

/**
 * \brief Load stereo camera parameters from a plain text file
 * cv::Mat
 */
void LoadStereoTransformsFromPlainText ( const std::string& filename, 
    cv::Mat* rightToLeftRotationMatrix, cv::Mat* rightToLeftTranslationVector);

/**
 * \brief Load the handeye matrix from a plain text file
 * cv::Mat
 */
void LoadHandeyeFromPlainText ( const std::string& filename, 
    cv::Mat* leftCameraToTracker);

/**
 * \brief Flips the matrices in the vector from left handed coordinate 
 * system to right handed and vice versa
 */
std::vector<cv::Mat> FlipMatrices (const std::vector<cv::Mat> Matrices);

/**
 * \brief find the average of a vector of 4x4 matrices
 */
cv::Mat AverageMatrices(std::vector<cv::Mat> Matrices);

 /**
 * \brief Sorts the matrices based on the translations , and returns the order
 */
std::vector<int> SortMatricesByDistance (const std::vector<cv::Mat> Matrices);
 
/**
  * \brief Sorts the matrices based on the rotations, and returns the order
  */
std::vector<int> SortMatricesByAngle (const std::vector<cv::Mat> Matrices);

 /**
  * \brief loads a result file into a residual vector and matrix
  */
void LoadResult(const std::string& Filename, cv::Mat& Result,
      std::vector<double>& residuals);

 /** 
  * \brief Transforms a point relative to the left camera lens to 
  * world coordinates using the handeye and tracking matrices
  */
extern "C++" NIFTKOPENCV_EXPORT cv::Point3f LeftLensToWorld ( cv::Point3f PointInLensCS,
      cv::Mat& Handeye, cv::Mat& Tracker );
 
/** 
  * \brief Transforms a point in world coordinates to a point 
  * relative to the left lens using
  * world coordinates using the handeye and tracking matrices
  */
extern "C++" NIFTKOPENCV_EXPORT cv::Point3f WorldToLeftLens ( cv::Point3f PointInWorldCS,
      cv::Mat& Handeye, cv::Mat& Tracker );

} // end namespace

#endif // MITKCAMERACALIBRATIONFACADE_H
