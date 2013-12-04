/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkProjectPointsOnStereoVideo_h
#define mitkProjectPointsOnStereoVideo_h

#include "niftkOpenCVExports.h"
#include <string>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <cv.h>
#include <highgui.h>
#include "mitkVideoTrackerMatching.h"

namespace mitk {

/**
 * \class Project points on stereo video
 * \brief Takes an input video file and tracking data. The 
 * video is split into right and left channels.
 * the user can specify a set of points in world coordinates that
 * are projected onto the screen for each frame of the video. 
 * Methods are provided to save:
 * -> the left and right video channels. 
 * -> the projected points as on screen coordinates for each frame
 * -> the points transformed to the coordinate system of the left hand lens.
 *
 */
class NIFTKOPENCV_EXPORT ProjectPointsOnStereoVideo : public itk::Object
{

public:

  mitkClassMacro(ProjectPointsOnStereoVideo, itk::Object);
  itkNewMacro(ProjectPointsOnStereoVideo);

  /** 
   * \brief
   * Set up the projector, finds the video file in the directory, and the tracking data, 
   * and sets up the videotracker matcher
   */
  void Initialise (std::string directory, std::string calibrationParameterDirectory);
  /**
   * \brief
   * performs the point projection
   */
  void  Project(mitk::VideoTrackerMatching::Pointer matcher, std::vector<double> * perturbation = NULL);
  
  /**
   * \brief
   * Sets the cameratotracker matrix for the passed matcher 
   * to match the matrix for the projector
   */
  void  SetMatcherCameraToTracker(mitk::VideoTrackerMatching::Pointer matcher);

  /**
   * \brief save the projected coordinates for each frame to a text file
   */
  void SaveProjectedCoordinates (std::string filename);

  /**
   * \brief save the points in left lens coordinates to a text file 
   */
  void SavePointsInLeftLensCoordinates (std::string filename);

  /**
   * \brief Set the world points directly
   */
 // void SetWorldPoints (std::vector<cv::Point3d> worldPoints);

  /**
   * \brief Set the world points by triangulating their position from the
   * on screen coordinates for the specified frame
   */
  void SetWorldPointsByTriangulation 
    (std::vector< std::pair<cv::Point2d,cv::Point2d> > onScreenPointPairs, 
     std::vector < unsigned int>  frameNumber , mitk::VideoTrackerMatching::Pointer matcher, 
     std::vector <double> * perturbation = NULL);

  void SetVisualise( bool) ;
  void SetSaveVideo( bool state, std::string prefix = "" );
  itkSetMacro ( TrackerIndex, int);
  itkSetMacro ( ReferenceIndex, int);
  itkSetMacro ( DrawLines, bool);
  itkSetMacro ( DrawAxes, bool);

  /**
   * \brief sets the world points and corresponding vectors
   */
  void SetWorldPoints ( std::vector<  std::pair < cv::Point3d, cv::Scalar >  > points );
  /** 
   * \brief set only the world points, the corresponding scalars get set to a default value
   */
  void SetWorldPoints ( std::vector< cv::Point3d > points );
  std::vector < std::vector <cv::Point3d> > GetPointsInLeftLensCS();
  std::vector < std::vector < std::pair<cv::Point2d, cv::Point2d> > >  GetProjectedPoints();
  itkGetMacro ( InitOK, bool);
  itkGetMacro ( ProjectOK, bool);
  itkGetMacro ( WorldToLeftCameraMatrices, std::vector < cv::Mat > );

protected:

  ProjectPointsOnStereoVideo();
  virtual ~ProjectPointsOnStereoVideo();

  ProjectPointsOnStereoVideo(const ProjectPointsOnStereoVideo&); // Purposefully not implemented.
  ProjectPointsOnStereoVideo& operator=(const ProjectPointsOnStereoVideo&); // Purposefully not implemented.

private:
  bool                          m_Visualise; //if true the project function attempts to open a couple of windows to show projection in real time
  bool                          m_SaveVideo; //if true the project function will buffer frames into a object to write out.
  std::string                   m_VideoIn; //the video in file
  std::string                   m_VideoOut; //video needs to be saved on the fly
  std::string                   m_Directory; //the directory containing the data
  std::vector< std::pair < cv::Point3d, cv::Scalar > >     
                                m_WorldPoints;  //the world points to project, and their accompanying scalar values 

  int                           m_TrackerIndex; //the tracker index to use for frame matching
  int                           m_ReferenceIndex; //the reference index to use for frame matching, not used by default
 
  bool                          m_DrawLines; //draw lines between the points
  bool                          m_InitOK;
  bool                          m_ProjectOK;
  bool                          m_DrawAxes;

  //the camera calibration parameters
  cv::Mat* m_LeftIntrinsicMatrix;
  cv::Mat* m_LeftDistortionVector;
  cv::Mat* m_RightIntrinsicMatrix;
  cv::Mat* m_RightDistortionVector;
  cv::Mat* m_RightToLeftRotationMatrix;
  cv::Mat* m_RightToLeftTranslationVector;
  cv::Mat* m_LeftCameraToTracker;

  /* m_ProjectPoints [framenumber][pointID](left.right);*/
  std::vector < std::vector < std::pair<cv::Point2d, cv::Point2d> > >
                                m_ProjectedPoints; // the projected points
  std::vector < std::vector < std::pair < cv::Point3d, cv::Scalar > > >    
                                m_PointsInLeftLensCS; // the points in left lens coordinates.
  std::vector < std::pair<cv::Point2d, cv::Point2d> > 
                                m_ScreenAxesPoints; // the projected axes points

  std::vector < cv::Mat >       m_WorldToLeftCameraMatrices;    // the saved camera positions

  CvCapture*                    m_Capture;
  CvVideoWriter*                m_LeftWriter;
  CvVideoWriter*                m_RightWriter;

  void ProjectAxes();
}; // end class

} // end namespace

#endif
