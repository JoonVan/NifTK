/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkVideoTrackerMatching_h
#define mitkVideoTrackerMatching_h

#include "niftkOpenCVExports.h"
#include <cv.h>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>

namespace mitk
{


class NIFTKOPENCV_EXPORT TrackingMatrices
{
public:
  std::vector<cv::Mat>   m_TrackingMatrices;
  std::vector<long long> m_TimingErrors;
};
class NIFTKOPENCV_EXPORT TrackingMatrixTimeStamps
{
public:
  std::vector<unsigned long long> m_TimeStamps;
  unsigned long long GetNearestTimeStamp (unsigned long long timestamp , long long * delta = NULL );
};

/**
 * \brief A class to match video frames to tracking frames, when reading 
 * recorded tracking data. 
 */
class NIFTKOPENCV_EXPORT VideoTrackerMatching : public itk::Object
{
public: 
  mitkClassMacro ( VideoTrackerMatching, itk::Object);
  itkNewMacro (VideoTrackerMatching);

  /**
   * \brief Initialise the class by passing it a directory name
   */

  void Initialise (std::string directory);

  /**
   * \brief Return the tracking matrix for a given video frame number
   */
  cv::Mat GetTrackerMatrix ( unsigned int FrameNumber, long long * TimingError = NULL, unsigned int TrackerIndex = 0 );

  /**
   * \brief Return the tracking matrix multiplied by the camera to tracker matrix for a given video frame number
   */
  cv::Mat GetCameraTrackingMatrix ( unsigned int FrameNumber, long long * TimingError = NULL, unsigned int TrackerIndex = 0 );

  /**
   * \brief returns state of m_Ready
   */
  bool IsReady () 
  { 
    return m_Ready;
  } 
  itkSetMacro (FlipMatrices, bool);

  /**
   * \brief get the frame count
   */
  int GetNumberOfFrames () 
  {
    return m_FrameNumbers.size();
  }

  /**
   * \brief if the tracking data is ahead of the video data you can set a video lag in 
   * milliseconds to account for this. If the video is ahead of the tracking set 
   * argument 2 to true
   */
  void SetVideoLagMilliseconds(unsigned long long VideoLag, bool VideoLeadsTracking =false, int trackerIndex = -1);

  /**
   * \brief set the camera to tracker matrix. if tracker index = -1 all camera to tracker 
   * matrices will be set with the same value
   */
  void SetCameraToTracker( cv::Mat, int trackerIndex = -1 );

  /*
   * \brief Convienient way to set the camera to tracker matrices where multiple tracking 
   * directories are present. File contains camera to tracker matrices in same order as 
   * tracker indices
   */
  void SetCameraToTrackers ( std::string filename );
  /**
   * \brief Pass a file name that defines the position of a point fixed in world
   * coordinates relative to the camera lens, along with the on screen coordinates
   * of the point. The VideoLag is adjusted so as to 
   * minimalise the standard deviation of the reconstructed world point
   */
  void TemporalCalibration (std::string filename, int windowLow = -100, int windowHigh = 100, bool visualise = false , std::string fileout = "" );

  /**
   * \brief Pass a file name that defines the position of a point fixed in world
   * coordinates relative to the camera lens, along with the on screen coordinates 
   * of the point. The world position of the point and 
   * the handeye calibration are optimised to minimise the residual error of the 
   * reconstructed point
   */
  void OptimiseHandeyeCalibration (std::string filename, bool visualise = false , std::string fileout = "" );

  /**
   * \brief Pass a file name that defines the position of a point fixed in world
   * coordinates relative to the camera lens, along with the on screen coordinates of 
   * the point. The world position of the point is 
   * is determined using a range of perturbed values for the handeye matrix. The
   * variance in the residual reconstruction error is used to determine the 
   * sensitivity of the system to errors in the hand eye calibration
   */
  void HandeyeSensitivityTest (std::string filename, bool visualise = false , std::string fileout = "" );

protected:
  VideoTrackerMatching();
  virtual ~VideoTrackerMatching();

  VideoTrackerMatching(const VideoTrackerMatching&); // Purposefully not implemented.
  VideoTrackerMatching& operator=(const VideoTrackerMatching&); // Purposefully not implemented.

private:
  std::vector<unsigned int>             m_FrameNumbers;
  std::vector<TrackingMatrices>         m_TrackingMatrices; 
  std::vector<TrackingMatrixTimeStamps> m_TrackingMatrixTimeStamps; 
  std::vector<std::string>              m_TrackingMatrixDirectories;
  std::string                           m_Directory;
  bool                                  m_Ready;
  bool                                  m_FlipMatrices;
  std::string                           m_FrameMap;

  std::vector<std::string> FindFrameMaps();
  void                     FindTrackingMatrixDirectories();
  TrackingMatrixTimeStamps FindTrackingTimeStamps(std::string directory);
  bool                     CheckIfDirectoryContainsTrackingMatrices(std::string directory);
  void                     ProcessFrameMapFile();
  cv::Mat                  ReadTrackerMatrix(std::string filename);
  bool                     CheckTimingErrorStats();
  /**
   * \brief Reads a file that defines the position of a point fixed in world
   * coordinates relative to the camera lens.
   */
  std::vector<cv::Point3d> ReadPointsInLensCSFile (std::string filename, 
      std::vector <cv::Point2d>* leftScreenPoints = NULL, 
      std::vector <cv::Point2d>* rightScreenPoints = NULL );
  std::vector<cv::Mat>     m_CameraToTracker;

  std::vector <unsigned long long>
                           m_VideoLag; //the delay between the tracking and video data
  std::vector <bool>       m_VideoLeadsTracking; //if the video lag is negative, set this to true

  
};


} // namespace


#endif // niftkVideoTrackerMatching_h
