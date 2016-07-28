/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkProjectPointsOnStereoVideo.h"
#include <mitkCameraCalibrationFacade.h>
#include <mitkOpenCVMaths.h>
#include <mitkOpenCVPointTypes.h>
#include <mitkOpenCVFileIOUtils.h>
#include <mitkPointSetWriter.h>
#include <cv.h>
#include <highgui.h>
#include <niftkFileHelper.h>

#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/lexical_cast.hpp>

namespace mitk {

//-----------------------------------------------------------------------------
ProjectPointsOnStereoVideo::ProjectPointsOnStereoVideo()
: m_Visualise(false)
, m_SaveVideo(false)
, m_CorrectVideoAspectRatioByHalvingWidth (true)
, m_VideoIn("")
, m_VideoOut("")
, m_Directory("")
, m_TriangulatedPointsOutName("")
, m_TrackerIndex(0)
, m_ReferenceIndex(-1)
, m_InitOK(false)
, m_ProjectOK(false)
, m_GoldStandardPointsClassifiedOK(false)
, m_TriangulateOK(false)
, m_DrawAxes(false)
, m_HaltOnVideoReadFail(true)
, m_DontProject(false)
, m_VisualiseTrackingStatus(false)
, m_AnnotateWithGoldStandards(false)
, m_WriteAnnotatedGoldStandards(false)
, m_WriteTrackingPositionData(false)
, m_WriteTrackingMatrixFilesPerFrame(false)
, m_CorrectTrackingMatrixFileNamesForSequentialChannelSplitVideo (true)
, m_LeftGSFramesAreEven(true)
, m_RightGSFramesAreEven(true)
, m_MaxGoldStandardPointIndex(-1)
, m_MaxGoldStandardLineIndex(-1)
, m_RightGSFrameOffset(0)
, m_LeftIntrinsicMatrix (new cv::Mat(3,3,CV_64FC1))
, m_LeftDistortionVector (new cv::Mat(1,4,CV_64FC1))
, m_RightIntrinsicMatrix (new cv::Mat(3,3,CV_64FC1))
, m_RightDistortionVector (new cv::Mat(1,4,CV_64FC1))
, m_RightToLeftRotationMatrix (new cv::Mat(3,3,CV_64FC1))
, m_RightToLeftTranslationVector (new cv::Mat(3,1,CV_64FC1))
, m_LeftCameraToTracker (new cv::Mat(4,4,CV_64FC1))
, m_ModelToWorldTransform (NULL)
, m_VideoWidth(1920)
, m_VideoHeight(540)
, m_Capture(NULL)
, m_WorldPoints(NULL)
, m_ModelPoints(NULL)
, m_ClassifierWorldPoints(NULL)
, m_LeftWriter(NULL)
, m_RightWriter(NULL)
, m_AllowablePointMatchingRatio (1.0)
, m_AllowableTimingError (20e6) // 20 milliseconds
, m_StartFrame(0)
, m_EndFrame(0)
, m_ProjectorScreenBuffer(0.0)
, m_ClassifierScreenBuffer(100.0)
, m_ReprojectionErrorZLimit (0.7)
{
}

//-----------------------------------------------------------------------------
ProjectPointsOnStereoVideo::~ProjectPointsOnStereoVideo()
{

}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::SetMatcherCameraToTracker(mitk::VideoTrackerMatching::Pointer trackerMatcher)
{
  if ( ! m_InitOK )
  {
    MITK_ERROR << "Can't set trackerMatcher handeye before projector initialiastion";
    return;
  }
  trackerMatcher->SetCameraToTracker(*m_LeftCameraToTracker, m_TrackerIndex);
  return;
}
//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::Initialise(std::string directory,
    std::string calibrationParameterDirectory)
{
  m_InitOK = false;

  Initialise ( directory );
  try
  {
    mitk::LoadStereoCameraParametersFromDirectory
      ( calibrationParameterDirectory,
      m_LeftIntrinsicMatrix,m_LeftDistortionVector,m_RightIntrinsicMatrix,
      m_RightDistortionVector,m_RightToLeftRotationMatrix,
      m_RightToLeftTranslationVector,m_LeftCameraToTracker);
  }
  catch ( int e )
  {
    MITK_ERROR << "Failed to load camera parameters";
    m_InitOK = false;
    return;
  }
  ProjectAxes();

  return;
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::Initialise(std::string directory)
{
  m_InitOK = false;
  m_Directory = directory;

  m_OutDirectory = m_Directory + niftk::GetFileSeparator() +  "ProjectionResults";

  m_InitOK = true;
  return;
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::FindVideoData(mitk::VideoTrackerMatching::Pointer trackerMatcher)
{
  if ( m_Visualise || m_SaveVideo || m_AnnotateWithGoldStandards )
  {
    if ( m_Capture == NULL )
    {
      m_VideoIn = niftk::FindVideoFile ( m_Directory , niftk::Basename (niftk::Basename ( trackerMatcher->GetFrameMap() )));
      if ( m_VideoIn == "" )
      {
        m_InitOK = false;
        return;
      }
      try
      {
        m_Capture = mitk::InitialiseVideoCapture(m_VideoIn, ( ! m_HaltOnVideoReadFail ));
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Caught exception " << e.what();
        exit(1);
      }
    }

    if ( m_SaveVideo )
    {
      try
      {
        niftk::CreateDirAndParents ( m_OutDirectory );
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Caught exception " << e.what();
        exit(1);
      }
    }

    if ( m_SaveVideo )
    {
      cv::Size S;
      if (  m_CorrectVideoAspectRatioByHalvingWidth )
      {
        S = cv::Size((int) m_VideoWidth/2.0, (int) m_VideoHeight );
      }
      else
      {
        S = cv::Size((int) m_VideoWidth, (int) m_VideoHeight );
      }
      double fps = static_cast<double>(m_Capture->get(CV_CAP_PROP_FPS));
      double halfFPS = fps/2.0;
      m_LeftWriter =cvCreateVideoWriter(std::string( m_OutDirectory + niftk::GetFileSeparator() + niftk::Basename(m_VideoIn) +  "_leftchannel.avi").c_str(), CV_FOURCC('D','I','V','X'),halfFPS,S, true);
      m_RightWriter =cvCreateVideoWriter(std::string(m_OutDirectory + niftk::GetFileSeparator() + niftk::Basename(m_VideoIn) + "_rightchannel.avi").c_str(), CV_FOURCC('D','I','V','X'),halfFPS,S, true);
    }
  }
  return;
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::SetVisualise ( bool visualise )
{
  if ( m_InitOK )
  {
    MITK_WARN << "Changing visualisation state after initialisation, will need to re-initialise";
  }
  m_Visualise = visualise;
  m_InitOK = false;
  return;
}
//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::SetSaveVideo ( bool savevideo )
{
  if ( m_InitOK )
  {
    MITK_WARN << "Changing save video  state after initialisation, will need to re-initialise";
  }
  m_SaveVideo = savevideo;

  m_InitOK = false;
  return;
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::Project(mitk::VideoTrackerMatching::Pointer trackerMatcher,
    std::vector<double>* perturbation)
{
  if ( ! m_InitOK )
  {
    MITK_WARN << "Called project before initialise.";
    return;
  }

  this->FindVideoData(trackerMatcher);

  m_ProjectOK = false;
  m_ProjectedPointLists.clear();
  m_PointsInLeftLensCS.clear();
  m_ClassifierProjectedPointLists.clear();

  if ( ! m_ModelPoints.IsNull() )
  {
    if ( m_ModelToWorldTransform == NULL )
    {
      MITK_WARN << "ProjectPointsOnStereoVideo::Project, ModelPoints Set but no ModelToWorldTransform, assuming Identity transform";
      m_ModelToWorldTransform = (new cv::Mat(4,4,CV_64FC1));
      for ( unsigned int i = 0 ; i < 4 ; ++i )
      {
        for ( unsigned int j = 0 ; j < 4 ; ++j )
        {
          if ( i == j )
          {
            m_ModelToWorldTransform->at<double>(i,j) = 1.0;
          }
          else
          {
            m_ModelToWorldTransform->at<double>(i,j) = 0.0;
          }
        }
      }
    }

    if ( m_WorldPoints.IsNull() )
    {
      m_WorldPoints = m_ModelPoints->TransformPointList ( m_ModelToWorldTransform );
      m_WorldPoints->SetChannel("world");
      //we probably need to set the channel for each picked point, but lets see what happens
    }
    else
    {
      //let's throw an error for now, I really don't know what we should do in this situation
      mitkThrow() <<"ProjectPointsOnStereoVideo::Project both m_WorldPoints and m_ModelPoints  set, I don't know what to do.";
    }
  }

  if ( m_WorldPoints.IsNull() )
  {
    m_WorldPoints = mitk::PickedPointList::New();
    m_WorldPoints->SetChannel("world");
  }

  //check there is a world point for every gold standar
  for ( std::vector < mitk::PickedObject >::iterator it = m_GoldStandardPoints.begin() ; it < m_GoldStandardPoints.end() ; ++ it )
  {
    m_WorldPoints->AddDummyPointIfNotPresent ( *it );
  }

  if ( ! ( m_DontProject ) && ( m_WorldPoints->GetListSize() == 0) )
  {
    MITK_WARN << "Called project with nothing to project";
    return;
  }

  if ( m_Visualise )
  {
    cvNamedWindow ("Left Channel", CV_WINDOW_AUTOSIZE);
    cvNamedWindow ("Right Channel", CV_WINDOW_AUTOSIZE);
  }
  int framenumber = 0 ;
  int key = 0;
  bool drawProjection = true;
  IplImage *smallimage = cvCreateImage (cvSize((int)m_VideoWidth/2.0, (int) m_VideoHeight/2.0), 8,3);
  IplImage *videoOutImage;

  if ( m_CorrectVideoAspectRatioByHalvingWidth )
  {
    videoOutImage = cvCreateImage (cvSize((int)m_VideoWidth/2.0, (int)m_VideoHeight), 8,3);
  }
  else
  {
    videoOutImage = cvCreateImage (cvSize((int)m_VideoWidth, (int)m_VideoHeight), 8,3);
  }

  std::vector < cv::Point3d > lastFrameTrackerOrigin;
  std::vector < cv::Point3d > lastFrameCameraOrigin;
  std::vector < unsigned long long > lastFrameTimeStamp;

  std::ofstream tracks_out;
  if ( m_WriteTrackingPositionData )
  {
    tracks_out.open(std::string (m_OutDirectory + "Tracking_Statistics.txt").c_str());
    unsigned int howMany = trackerMatcher->GetTrackingMatricesSize();
    tracks_out <<  "# framenumber timestamp ";
    for ( unsigned int i = 0 ; i < howMany ; ++ i )
    {
      lastFrameTrackerOrigin.push_back(cv::Point3d(0.0,0.0,0.0));
      lastFrameCameraOrigin.push_back(cv::Point3d(0.0,0.0,0.0));
      lastFrameTimeStamp.push_back(0);

      tracks_out << i << "_TimingError " << i << "_tracker_X " << i << "_tracker_Y " << i << "_tracker_Z " << i << "_tracker_Speed "  ;
      tracks_out << i << "_camera_X " << i << "_camera_Y " << i << "_camera_Z " << i << "_camera_Speed "  ;
    }
    tracks_out << std::endl;
  }
  while ( framenumber < trackerMatcher->GetNumberOfFrames() && key != 'q')
  {
    if ( ( m_StartFrame < m_EndFrame ) && ( framenumber < m_StartFrame || framenumber > m_EndFrame ) )
    {
      if ( m_Visualise || m_SaveVideo || m_AnnotateWithGoldStandards )
      {
        cv::Mat videoImage;
        m_Capture->read(videoImage);
        MITK_INFO << "Skipping frame " << framenumber;
      }
      framenumber ++;
    }
    else
    {
      if ( m_WriteTrackingPositionData || m_WriteTrackingMatrixFilesPerFrame )
      {
        unsigned int howMany = trackerMatcher->GetTrackingMatricesSize();
        unsigned long long timeStamp = trackerMatcher->GetVideoFrameTimeStamp(framenumber);
        if ( m_WriteTrackingMatrixFilesPerFrame || (timeStamp != lastFrameTimeStamp[0]) )
        {
          if ( m_WriteTrackingPositionData && (timeStamp != lastFrameTimeStamp[0]) )
          {
            tracks_out <<  framenumber << " " <<  timeStamp << " " ;
          }
          for ( unsigned int i = 0 ; i < howMany ; ++ i )
          {
            long long timingError;
            cv::Mat trackerToWorld = trackerMatcher->GetTrackerMatrix(framenumber, &timingError, i, m_ReferenceIndex);
            if ( m_WriteTrackingPositionData && (timeStamp != lastFrameTimeStamp[0]) )
            {
              cv::Mat cameraToWorld = trackerMatcher->GetCameraTrackingMatrix(framenumber, &timingError, i, perturbation, m_ReferenceIndex);
              cv::Point3d origin (0.0,0.0,0.0);
              cv::Point3d trackerOrigin = trackerToWorld * origin;
              cv::Point3d cameraOrigin = cameraToWorld * origin;
              double trackerSpeed = ( mitk::Norm  (trackerOrigin - lastFrameTrackerOrigin[i]) ) / ( timeStamp - lastFrameTimeStamp[i] ) * 1e9; //mm per second
              double cameraSpeed = ( mitk::Norm  (cameraOrigin - lastFrameCameraOrigin[i]) ) / ( timeStamp - lastFrameTimeStamp[i] ) * 1e9; //mm per second
              tracks_out << timingError << " " << trackerOrigin.x << " " << trackerOrigin.y << " " << trackerOrigin.z << " " << trackerSpeed  ;
              tracks_out << " " << cameraOrigin.x << " " << cameraOrigin.y << " " << cameraOrigin.z << " " << cameraSpeed  ;
              lastFrameTrackerOrigin[i] = trackerOrigin;
              lastFrameCameraOrigin[i] = cameraOrigin;
              lastFrameTimeStamp[i] = timeStamp;
            }
            if ( m_WriteTrackingMatrixFilesPerFrame )
            {
              if ( m_CorrectTrackingMatrixFileNamesForSequentialChannelSplitVideo )
              {
                if ( framenumber % 2 == 0 )
                {
                  std::ofstream trackingMatrixOut;
                  trackingMatrixOut.open(std::string (m_OutDirectory + niftk::GetFileSeparator() + "Tracker_" + \
                      boost::lexical_cast<std::string>(i) + "_Frame_" + boost::lexical_cast<std::string>(framenumber/2) \
                      + "_Tracking_Matrix.4x4").c_str());
                  trackingMatrixOut << trackerToWorld;
                  trackingMatrixOut << std::endl << "#Timing Error = " << timingError;
                  trackingMatrixOut.close();
                }
              }
              else
              {
                std::ofstream trackingMatrixOut;
                trackingMatrixOut.open(std::string (m_OutDirectory + niftk::GetFileSeparator() + "Tracker_" + \
                  boost::lexical_cast<std::string>(i) + "_Frame_" + boost::lexical_cast<std::string>(framenumber) \
                  + "_Tracking_Matrix.4x4").c_str());
                trackingMatrixOut << trackerToWorld;
                trackingMatrixOut << std::endl << "#Timing Error = " << timingError;
                trackingMatrixOut.close();
              }
            }

          }
          if ( m_WriteTrackingPositionData && (timeStamp != lastFrameTimeStamp[0]) )
          {
            tracks_out << std::endl;
          }
        }
      }

      //put the world points into the coordinates of the left hand camera.
      //worldtotracker * trackertocamera
      //in general the tracker matrices are trackertoworld
      long long timingError;
      cv::Mat WorldToLeftCamera = trackerMatcher->GetCameraTrackingMatrix(framenumber, &timingError, m_TrackerIndex, perturbation, m_ReferenceIndex).inv();
      unsigned long long matrixTimeStamp;
      unsigned long long absTimingError = static_cast<unsigned long long> ( std::abs(timingError));
      if ( timingError < 0 )
      {
        matrixTimeStamp = trackerMatcher->GetVideoFrameTimeStamp(framenumber) + absTimingError;
      }
      else
      {
        matrixTimeStamp = trackerMatcher->GetVideoFrameTimeStamp(framenumber) - absTimingError;
      }

      if ( ! m_DontProject )
      {
        m_WorldToLeftCameraMatrices.push_back(WorldToLeftCamera);

        if ( ! m_WorldPoints.IsNull () )
        {
          m_PointsInLeftLensCS.push_back (TransformPickedPointListToLeftLens ( m_WorldPoints, WorldToLeftCamera, matrixTimeStamp, framenumber ));
          m_ProjectedPointLists.push_back( ProjectPickedPointList ( m_PointsInLeftLensCS.back(), m_ProjectorScreenBuffer )) ;
        }

        if ( ! m_ClassifierWorldPoints.IsNull() )
        {
          mitk::PickedPointList::Pointer classifierPointsInLeftLensCS =
            TransformPickedPointListToLeftLens ( m_ClassifierWorldPoints, WorldToLeftCamera, matrixTimeStamp, framenumber );
          m_ClassifierProjectedPointLists.push_back (ProjectPickedPointList ( classifierPointsInLeftLensCS , m_ClassifierScreenBuffer ));
        }
      }
      else
      {
        drawProjection = false;
      }

      if ( m_Visualise || m_SaveVideo || m_AnnotateWithGoldStandards )
      {
        cv::Mat videoImage;
        m_Capture->read(videoImage);
        if ( drawProjection )
        {
          m_ProjectedPointLists.back()->AnnotateImage(videoImage);
        }
        if ( m_DrawAxes )
        {
          if ( framenumber % 2 == 0 )
          {
            cv::line(videoImage,m_ScreenAxesPoints.m_Points[0].m_Left,m_ScreenAxesPoints.m_Points[1].m_Left,cvScalar(255,0,0));
            cv::line(videoImage,m_ScreenAxesPoints.m_Points[0].m_Left,m_ScreenAxesPoints.m_Points[2].m_Left,cvScalar(0,255,0));
            cv::line(videoImage,m_ScreenAxesPoints.m_Points[0].m_Left,m_ScreenAxesPoints.m_Points[3].m_Left,cvScalar(0,0,255));
          }
          else
          {
            cv::line(videoImage,m_ScreenAxesPoints.m_Points[0].m_Right,m_ScreenAxesPoints.m_Points[1].m_Right,cvScalar(255,0,0));
            cv::line(videoImage,m_ScreenAxesPoints.m_Points[0].m_Right,m_ScreenAxesPoints.m_Points[2].m_Right,cvScalar(0,255,0));
            cv::line(videoImage,m_ScreenAxesPoints.m_Points[0].m_Right,m_ScreenAxesPoints.m_Points[3].m_Right,cvScalar(0,0,255));
          }
        }
        if ( m_VisualiseTrackingStatus )
        {
          unsigned int howMany = trackerMatcher->GetTrackingMatricesSize();

          for ( unsigned int i = 0 ; i < howMany ; i ++ )
          {

            long long timingError;
            trackerMatcher->GetCameraTrackingMatrix(framenumber , &timingError , i);
            cv::Point2d textLocation = cv::Point2d ( m_VideoWidth - ( m_VideoWidth * 0.03 ) , (i+1) *  m_VideoHeight * 0.07  );
            cv::Point2d location = cv::Point2d ( m_VideoWidth - ( m_VideoWidth * 0.035 ) , (i) *  m_VideoHeight * 0.07 + m_VideoHeight * 0.02  );
            cv::Point2d location1 = cv::Point2d ( m_VideoWidth - ( m_VideoWidth * 0.035 ) + ( m_VideoWidth * 0.025 ) ,
              (i) *  m_VideoHeight * 0.07 + (m_VideoHeight * 0.06) + m_VideoHeight * 0.02);
            if ( timingError < m_AllowableTimingError )
            {
              cv::rectangle ( videoImage, location, location1  , cvScalar (0,255,0), CV_FILLED);
              cv::putText(videoImage , "T" + boost::lexical_cast<std::string>(i), textLocation ,0,1.0, cvScalar ( 255,255,255), 4.0);
            }
            else
            {
              cv::rectangle ( videoImage, location, location1  , cvScalar (0,0,255), CV_FILLED);
              cv::putText(videoImage , "T" + boost::lexical_cast<std::string>(i), textLocation ,0,1.0, cvScalar ( 255,255,255), 4.0);
            }
          }
        }
        if ( m_AnnotateWithGoldStandards )
        {
          std::vector < mitk::PickedObject > goldStandardObjects;
          for ( std::vector<mitk::PickedObject>::iterator it = m_GoldStandardPoints.begin()  ; it < m_GoldStandardPoints.end() ; ++it )
          {
            if ( it->m_FrameNumber == framenumber )
            {
              if ( framenumber%2 == 0 )
              {
                if ( it->m_Channel == "left" )
                {
                  goldStandardObjects.push_back(*it);
                  goldStandardObjects.back().m_Scalar = cv::Scalar ( 0,255,255);
                }
              }
              if ( framenumber%2 !=0 )
              {
                if ( it->m_Channel == "right" )
                {
                  goldStandardObjects.push_back(*it);
                  goldStandardObjects.back().m_Scalar = cv::Scalar ( 0,255,255);
                }
              }
            }
          }
          if ( goldStandardObjects.size() != 0 )
          {
            mitk::PickedPointList::Pointer goldStandardPointList = mitk::PickedPointList::New();
            goldStandardPointList->SetPickedObjects(goldStandardObjects);
            goldStandardPointList->AnnotateImage(videoImage);
            if ( m_WriteAnnotatedGoldStandards )
            {
              std::string outname = boost::lexical_cast<std::string>(framenumber) + ".png";
              cv::imwrite(outname,videoImage);
            }

          }
        }

        if ( m_SaveVideo )
        {
          if ( m_LeftWriter != NULL )
          {
            if ( framenumber%2 == 0 )
            {
              IplImage image(videoImage);
              if ( m_CorrectVideoAspectRatioByHalvingWidth )
              {
                cvResize (&image, videoOutImage,CV_INTER_LINEAR);
                cvWriteFrame(m_LeftWriter,videoOutImage);
              }
              else
              {
                cvWriteFrame(m_LeftWriter,&image);
              }
            }
          }
          if ( m_RightWriter != NULL )
          {
            if ( framenumber%2 != 0 )
            {
              IplImage image(videoImage);
              if ( m_CorrectVideoAspectRatioByHalvingWidth )
              {
                cvResize (&image, videoOutImage,CV_INTER_LINEAR);
                cvWriteFrame(m_RightWriter,videoOutImage);
              }
              else
              {
                cvWriteFrame(m_RightWriter,&image);
              }
            }
          }
        }
        if ( m_Visualise )
        {
          IplImage image(videoImage);
          cvResize (&image, smallimage,CV_INTER_LINEAR);
          if ( framenumber %2 == 0 )
          {
            cvShowImage("Left Channel" , smallimage);
          }
          else
          {
            cvShowImage("Right Channel" , smallimage);
          }
          key = cvWaitKey (20);
          if ( key == 's' )
          {
            m_Visualise = false;
          }
          if ( key == 't' )
          {
            drawProjection = ! drawProjection;
          }
        }
      }
      framenumber ++;
    }
  }
  if ( m_LeftWriter != NULL )
  {
    cvReleaseVideoWriter(&m_LeftWriter);
  }
  if ( m_RightWriter != NULL )
  {
    cvReleaseVideoWriter(&m_RightWriter);
  }

  if ( tracks_out.is_open() )
  {
    tracks_out.close();
  }
  m_ProjectOK = true;

}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::SetLeftGoldStandardPoints (
    std::vector < mitk::GoldStandardPoint > points,
    mitk::VideoTrackerMatching::Pointer matcher )
{
  int maxLeftGSIndex = -1;
  for ( unsigned int i = 0 ; i < points.size() ; i ++ )
  {
    m_GoldStandardPoints.push_back(mitk::PickedObject(points[i], matcher->GetVideoFrameTimeStamp ( points[i].m_FrameNumber)));
    m_GoldStandardPoints.back().m_Channel = "left";

    if ( m_GoldStandardPoints.back().m_Id > maxLeftGSIndex )
    {
      maxLeftGSIndex =  m_GoldStandardPoints.back().m_Id;
    }
    if ( m_GoldStandardPoints.back().m_FrameNumber % 2 == 0 )
    {
      if ( ! m_LeftGSFramesAreEven )
      {
        MITK_ERROR << "Detected inconsistent frame numbering in the left gold standard points, left GS should be odd";
        exit(1);
      }
    }
    else
    {
      if ( ( i > 0 ) && ( m_LeftGSFramesAreEven ) )
      {
        MITK_ERROR << "Detected inconsistent frame numbering in the left gold standard points, left GS should be even";
        exit(1);
      }
      m_LeftGSFramesAreEven = false;
    }
  }
  if ( m_LeftGSFramesAreEven == m_RightGSFramesAreEven )
  {
    m_RightGSFrameOffset = 0 ;
  }
  else
  {
    m_RightGSFrameOffset = 1 ;
  }
  if ( maxLeftGSIndex > m_MaxGoldStandardPointIndex )
  {
    m_MaxGoldStandardPointIndex = maxLeftGSIndex;
  }
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::SetGoldStandardObjects( std::vector < mitk::PickedObject > pickedObjects )
{
  //this index checking doesn't account for lines / points
  int maxLeftGSIndex = -1;
  int maxRightGSIndex = -1;
  int maxLeftLinesGSIndex = -1;
  int maxRightLinesGSIndex = -1;
  int leftPoints = 0;
  int rightPoints = 0;
  if ( m_GoldStandardPoints.size() != 0 )
  {
    MITK_WARN << "Setting gold standard points with non empty vector, check this is what you intended.";
  }
  for ( unsigned int i = 0 ; i < pickedObjects.size() ; i ++ )
  {
    if ( pickedObjects[i].m_Channel == "left" )
    {
      if ( pickedObjects[i].m_IsLine && ( pickedObjects[i].m_Id > maxLeftLinesGSIndex ) )
      {
        maxLeftLinesGSIndex = pickedObjects[i].m_Id;
      }
      if (  ( ! pickedObjects[i].m_IsLine ) && ( pickedObjects[i].m_Id > maxLeftGSIndex ) )
      {
        maxLeftGSIndex = pickedObjects[i].m_Id;
      }
      if ( pickedObjects[i].m_FrameNumber % 2 == 0 )
      {
        if ( ! m_LeftGSFramesAreEven )
        {
          MITK_ERROR << "Detected inconsistent frame numbering in the left gold standard points, left GS should be odd";
          exit(1);
        }
      }
      else
      {
        if ( ( leftPoints > 0 ) && ( m_LeftGSFramesAreEven ) )
        {
          MITK_ERROR << "Detected inconsistent frame numbering in the left gold standard points, left GS should be even";
          exit(1);
        }
        m_LeftGSFramesAreEven = false;
      }
      leftPoints++;
    }
    else
    {
      if ( pickedObjects[i].m_Channel != "right" )
      {
        MITK_ERROR << "Attempted to set gold standard point with unknown channel type " << pickedObjects[i].m_Channel;
        exit(1);
      }
      if ( pickedObjects[i].m_IsLine && pickedObjects[i].m_Id > maxRightLinesGSIndex )
      {
        maxRightLinesGSIndex =  pickedObjects[i].m_Id;
      }
      if ( (!pickedObjects[i].m_IsLine) && pickedObjects[i].m_Id > maxRightGSIndex )
      {
        maxRightGSIndex =  pickedObjects[i].m_Id;
      }
      if ( pickedObjects[i].m_FrameNumber % 2 == 0 )
      {
        if ( ! m_RightGSFramesAreEven )
        {
          MITK_ERROR << "Detected inconsistent frame numbering in the right gold standard points, right GS should be odd, fn = " <<  pickedObjects[i].m_FrameNumber;
          exit(1);
        }
      }
      else
      {
        if ( ( rightPoints > 0 ) && ( m_RightGSFramesAreEven ) )
        {
          MITK_ERROR << "Detected inconsistent frame numbering in the right gold standard points, right GS should be even";
          exit(1);
        }
        m_RightGSFramesAreEven = false;
      }
      rightPoints++;
    }
    m_GoldStandardPoints.push_back(pickedObjects[i]);
  }

  if ( m_LeftGSFramesAreEven == m_RightGSFramesAreEven )
  {
    m_RightGSFrameOffset = 0 ;
  }
  else
  {
    m_RightGSFrameOffset = 1 ;
  }
  if ( maxLeftGSIndex > m_MaxGoldStandardPointIndex )
  {
    m_MaxGoldStandardPointIndex = maxLeftGSIndex;
  }
  if ( maxRightGSIndex > m_MaxGoldStandardPointIndex )
  {
    m_MaxGoldStandardPointIndex = maxRightGSIndex;
  }
  if ( maxLeftLinesGSIndex > m_MaxGoldStandardLineIndex )
  {
    m_MaxGoldStandardLineIndex = maxLeftLinesGSIndex;
  }
  if ( maxRightLinesGSIndex > m_MaxGoldStandardLineIndex )
  {
    m_MaxGoldStandardLineIndex = maxRightLinesGSIndex;
  }

}
//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::SetRightGoldStandardPoints (
    std::vector < mitk::GoldStandardPoint > points ,
    mitk::VideoTrackerMatching::Pointer matcher )
{
  int maxRightGSIndex = -1;

  for ( unsigned int i = 0 ; i < points.size() ; i ++ )
  {
    m_GoldStandardPoints.push_back(mitk::PickedObject(points[i],  matcher->GetVideoFrameTimeStamp ( points[i].m_FrameNumber)));
    m_GoldStandardPoints.back().m_Channel = "right";
    if ( m_GoldStandardPoints.back().m_Id > maxRightGSIndex )
    {
      maxRightGSIndex =  m_GoldStandardPoints[i].m_Id;
    }
    if ( m_GoldStandardPoints.back().m_FrameNumber % 2 == 0 )
    {
      if ( ! m_RightGSFramesAreEven )
      {
        MITK_ERROR << "Detected inconsistent frame numbering in the right gold standard points, right GS should be odd, fn = " <<  m_GoldStandardPoints[i].m_FrameNumber;
        exit(1);
      }
    }
    else
    {
      if ( ( i > 0 ) && ( m_RightGSFramesAreEven ) )
      {
        MITK_ERROR << "Detected inconsistent frame numbering in the right gold standard points, right GS should be even";
        exit(1);
      }
      m_RightGSFramesAreEven = false;
    }
  }
  if ( m_LeftGSFramesAreEven == m_RightGSFramesAreEven )
  {
    m_RightGSFrameOffset = 0 ;
  }
  else
  {
    m_RightGSFrameOffset = 1 ;
  }
  if ( maxRightGSIndex > m_MaxGoldStandardPointIndex )
  {
    m_MaxGoldStandardPointIndex = maxRightGSIndex;
  }
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::SetModelToWorldTransform(cv::Mat* modelToWorld)
{
  m_ModelToWorldTransform = modelToWorld;
}

//-----------------------------------------------------------------------------
bool ProjectPointsOnStereoVideo::TriangulateGoldStandardObjectList ( )
{
  if ( (! m_ProjectOK ) || (m_MaxGoldStandardPointIndex == -1 && m_MaxGoldStandardLineIndex == -1) )
  {
    MITK_ERROR << "Attempted to run TriangulateGoldStandardObjectList, before running project(), no result.";
    return false;
  }
  //everything should already be clasified and sorted so all we need to to do is go through the vector looking for clear();
  //matched pairs

  m_TriangulatedGoldStandardPoints.clear();
  for ( unsigned int i = 0 ; i < m_GoldStandardPoints.size() ; i ++ )
  {
    if ( m_GoldStandardPoints[i].m_Channel == "left" )
    {
      for ( unsigned int j = i + 1 ; j < m_GoldStandardPoints.size() ; j ++ )
      {
        if ( m_GoldStandardPoints[j].m_FrameNumber == ( m_GoldStandardPoints[i].m_FrameNumber + m_RightGSFrameOffset ) )
        {
          assert ( m_GoldStandardPoints[j].m_Channel == "right" );
          if ( ( m_GoldStandardPoints[i].m_IsLine == m_GoldStandardPoints[j].m_IsLine ) &&
                 (m_GoldStandardPoints[j].m_Id == m_GoldStandardPoints[i].m_Id ) )
          {
            m_TriangulatedGoldStandardPoints.push_back( TriangulatePickedObjects ( m_GoldStandardPoints[i], m_GoldStandardPoints[j] ) );
          }
        }
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::CalculateTriangulationErrors (std::string outPrefix )
{
  if ( ! m_GoldStandardPointsClassifiedOK )
  {
    ClassifyGoldStandardPoints ();
  }

  if ( ! m_TriangulateOK )
  {
    if ( this->TriangulateGoldStandardObjectList() )
    {
       m_TriangulateOK = true;
    }
    else
    {
      MITK_ERROR << "Attempted to run CalculateTriangulateErrors, before running project(), no result.";
      return;
    }
  }
  //everything should already be clasified and sorted so all we need to to do is go through the vector looking for
  //matched pairs
  m_TriangulationErrors.clear();
  for ( unsigned int i = 0 ; i < m_TriangulatedGoldStandardPoints.size() ; i ++ )
  {
    mitk::PickedObject leftLensObject = GetMatchingPickedObject ( m_TriangulatedGoldStandardPoints[i], *m_PointsInLeftLensCS[m_TriangulatedGoldStandardPoints[i].m_FrameNumber] );
    mitk::PickedObject triangulationError;
    leftLensObject.DistanceTo ( m_TriangulatedGoldStandardPoints[i], triangulationError, m_AllowableTimingError );
    m_TriangulationErrors.push_back ( triangulationError );
  }

  std::ofstream tout (std::string (outPrefix + "_triangulation.errors").c_str());
  tout << "#xmm ymm zmm" << std::endl;
  for ( unsigned int i = 0 ; i < m_TriangulationErrors.size() ; i ++ )
  {
    tout << m_TriangulationErrors[i].m_Points[0] << std::endl;
  }
  cv::Point3d error3dStdDev;
  cv::Point3d error3dMean;
  double xrms;
  double yrms;
  double zrms;
  double rms;
  std::vector<cv::Point3d> triangulationErrors;
  for ( std::vector<mitk::PickedObject>::iterator it = m_TriangulationErrors.begin() ;
      it < m_TriangulationErrors.end() ; ++ it )
  {
    triangulationErrors.push_back ( it->m_Points[0]);
  }

  error3dMean = mitk::GetCentroid(triangulationErrors, false, &error3dStdDev);
  tout << "#Mean Error      = " << error3dMean << std::endl;
  tout << "#StdDev          = " << error3dStdDev << std::endl;
  xrms = sqrt ( error3dMean.x * error3dMean.x + error3dStdDev.x * error3dStdDev.x );
  yrms = sqrt ( error3dMean.y * error3dMean.y + error3dStdDev.y * error3dStdDev.y );
  zrms = sqrt ( error3dMean.z * error3dMean.z + error3dStdDev.z * error3dStdDev.z );
  rms = sqrt ( xrms*xrms + yrms*yrms + zrms*zrms);
  tout << "#rms             = " << xrms << ", " << yrms << ", " << zrms << ", " << rms << std::endl;
  error3dMean = mitk::GetCentroid(triangulationErrors, true, &error3dStdDev);
  tout << "#Ref. Mean Error = " << error3dMean << std::endl;
  tout << "#Ref. StdDev     = " << error3dStdDev << std::endl;
  xrms = sqrt ( error3dMean.x * error3dMean.x + error3dStdDev.x * error3dStdDev.x );
  yrms = sqrt ( error3dMean.y * error3dMean.y + error3dStdDev.y * error3dStdDev.y );
  zrms = sqrt ( error3dMean.z * error3dMean.z + error3dStdDev.z * error3dStdDev.z );
  rms = sqrt ( xrms*xrms + yrms*yrms + zrms*zrms);
  tout << "#Ref. rms        = " << xrms << ", " << yrms << ", " << zrms << ", " << rms << std::endl;
  tout.close();
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::TriangulateGoldStandardPoints (std::string outPrefix, mitk::VideoTrackerMatching::Pointer trackerMatcher )
{

  if ( ! m_GoldStandardPointsClassifiedOK )
  {
    ClassifyGoldStandardPoints ();
  }

  if ( ! m_TriangulateOK )
  {
    if ( this->TriangulateGoldStandardObjectList() )
    {
       m_TriangulateOK = true;
    }
    else
    {
      MITK_ERROR << "Attempted to run CalculateTriangulateErrors, before running project(), no result.";
      return;
    }
  }
  //go through  m_TriangulatedGoldStandardPoints, for each m_Id (points and lines) find the centroid and output
  mitk::PointSet::Pointer triangulatedPoints = mitk::PointSet::New();
  std::vector < bool > pointIDTriangulated;
  std::vector < bool > lineIDTriangulated;
  for ( unsigned int i = 0 ; i < m_MaxGoldStandardPointIndex ; i ++ )
  {
     pointIDTriangulated.push_back(false);
  }
  for ( unsigned int i = 0 ; i < m_MaxGoldStandardLineIndex ; i ++ )
  {
     lineIDTriangulated.push_back(false);
  }
  for ( unsigned int i = 0 ; i < m_TriangulatedGoldStandardPoints.size() ; i ++ )
  {
    if (  m_TriangulatedGoldStandardPoints[i].m_IsLine == false )
    {
      if ( ! ( pointIDTriangulated [ m_TriangulatedGoldStandardPoints[i].m_Id ] ) )
      {
        std::vector < cv::Point3d > matchingPoints;

        for ( unsigned int j = 0 ; j < m_TriangulatedGoldStandardPoints.size() ; j ++ )
        {
          if (  m_TriangulatedGoldStandardPoints[j].m_IsLine == false )
          {
            if (  m_TriangulatedGoldStandardPoints[j].m_Id == m_TriangulatedGoldStandardPoints[i].m_Id )
            {
              cv::Mat leftCameraToWorld = trackerMatcher->GetCameraTrackingMatrix(m_TriangulatedGoldStandardPoints[j].m_FrameNumber, NULL, m_TrackerIndex, NULL, m_ReferenceIndex);
              matchingPoints.push_back ( leftCameraToWorld * m_TriangulatedGoldStandardPoints[j].m_Points[0]  );
            }
          }
        }
        cv::Point3d centroid;
        cv::Point3d stdDev;
        centroid = mitk::GetCentroid (matchingPoints,true, & stdDev);

        mitk::Point3D point;
        point[0] = centroid.x;
        point[1] = centroid.y;
        point[2] = centroid.z;
        triangulatedPoints->InsertPoint(m_TriangulatedGoldStandardPoints[i].m_Id,point);
        MITK_INFO << "Point " <<  m_TriangulatedGoldStandardPoints[i].m_Id << " triangulated mean " << centroid << " SD " << stdDev;
        pointIDTriangulated [ m_TriangulatedGoldStandardPoints[i].m_Id ] = true;
      }
    }
    else // it is a line
    {
      //I can't triangulate a line
      lineIDTriangulated [ m_TriangulatedGoldStandardPoints[i].m_Id ] = true;
    }
  }
  if ( m_TriangulatedPointsOutName != "" )
  {
    mitk::PointSetWriter::Pointer tpWriter = mitk::PointSetWriter::New();
    tpWriter->SetFileName(m_TriangulatedPointsOutName);
    tpWriter->SetInput( triangulatedPoints );
    tpWriter->Update();
  }

}

//-----------------------------------------------------------------------------
mitk::PickedObject ProjectPointsOnStereoVideo::TriangulatePickedObjects (mitk::PickedObject po_leftScreen,
    mitk::PickedObject po_rightScreen )
{
  assert ( po_leftScreen.m_Channel == "left" && po_rightScreen.m_Channel == "right" );
  assert ( po_leftScreen.m_IsLine == po_rightScreen.m_IsLine );

  mitk::PickedObject po_leftLens = po_leftScreen.CopyByHeader();
  po_leftLens.m_Channel = "left_lens";

  if ( po_leftScreen.m_IsLine )
  {
    //heres the plan, for each picked object resample the line to a vector of
    //1000 ? points, maybe do it based on length, so if line is n pixels long,
    //we resample it n times ?
    //Project the points to a vector of rays, so for each screen we have a
    //vector of rays.
    //then for each point in each vector we find the closest ray in the other data set.
    //if the rays correspond then we say it's a successful match, and add it to the
    //vector of points in the triangulated object. Also to check that things are'nt off the
    //end of the line we could look for a minimum distance ratio, or perhaps an
    //absolute distance. (2 mm??)
    MITK_WARN << "I don't know how to triangulate a line, giving up";
    return po_leftLens;
  }
  else
  {
    assert ( (po_leftScreen.m_Points[0].z == 0.0) && (po_rightScreen.m_Points[0].z == 0.0) );
    cv::Point2d leftUndistorted;
    cv::Point2d rightUndistorted;
    std::pair < cv::Point2d, cv::Point2d > pair = std::pair < cv::Point2d, cv::Point2d >
    ( cv::Point2d ( po_leftScreen.m_Points[0].x, po_leftScreen.m_Points[0].y ),
      cv::Point2d ( po_rightScreen.m_Points[0].x, po_rightScreen.m_Points[0].y ));
    bool cropUndistortedPointsToScreen = true;
    double cropValue = std::numeric_limits<double>::quiet_NaN();
    mitk::UndistortPoint(pair.first,
      *m_LeftIntrinsicMatrix,*m_LeftDistortionVector,leftUndistorted,
      cropUndistortedPointsToScreen ,
      0.0, m_VideoWidth, 0.0, m_VideoHeight,cropValue);
    mitk::UndistortPoint(pair.second,
      *m_RightIntrinsicMatrix,*m_RightDistortionVector,rightUndistorted,
      cropUndistortedPointsToScreen ,
      0.0, m_VideoWidth, 0.0, m_VideoHeight,cropValue);

    cv::Mat leftCameraTranslationVector = cv::Mat (3,1,CV_64FC1);
    cv::Mat leftCameraRotationVector = cv::Mat (3,1,CV_64FC1);
    cv::Mat rightCameraTranslationVector = cv::Mat (3,1,CV_64FC1);
    cv::Mat rightCameraRotationVector = cv::Mat (3,1,CV_64FC1);

    for ( int j = 0 ; j < 3 ; j ++ )
    {
      leftCameraTranslationVector.at<double>(j,0) = 0.0;
      leftCameraRotationVector.at<double>(j,0) = 0.0;
    }
    rightCameraTranslationVector = *m_RightToLeftTranslationVector * -1;
    cv::Rodrigues ( m_RightToLeftRotationMatrix->inv(), rightCameraRotationVector  );

    CvMat* leftScreenPointsMat = cvCreateMat (1,2,CV_64FC1);
    CvMat* rightScreenPointsMat = cvCreateMat (1,2,CV_64FC1);
    CvMat leftCameraIntrinsicMat= *m_LeftIntrinsicMatrix;
    CvMat leftCameraRotationVectorMat= leftCameraRotationVector;
    CvMat leftCameraTranslationVectorMat= leftCameraTranslationVector;
    CvMat rightCameraIntrinsicMat = *m_RightIntrinsicMatrix;
    CvMat rightCameraRotationVectorMat = rightCameraRotationVector;
    CvMat rightCameraTranslationVectorMat= rightCameraTranslationVector;
    CvMat* leftCameraTriangulatedWorldPoints = cvCreateMat (1,3,CV_64FC1);

    CV_MAT_ELEM(*leftScreenPointsMat,double,0,0) = leftUndistorted.x;
    CV_MAT_ELEM(*leftScreenPointsMat,double,0,1) = leftUndistorted.y;
    CV_MAT_ELEM(*rightScreenPointsMat,double,0,0) = rightUndistorted.x;
    CV_MAT_ELEM(*rightScreenPointsMat,double,0,1) = rightUndistorted.y;

    mitk::CStyleTriangulatePointPairsUsingSVD(
      *leftScreenPointsMat,
      *rightScreenPointsMat,
      leftCameraIntrinsicMat,
      leftCameraRotationVectorMat,
      leftCameraTranslationVectorMat,
      rightCameraIntrinsicMat,
      rightCameraRotationVectorMat,
      rightCameraTranslationVectorMat,
      *leftCameraTriangulatedWorldPoints);

    cv::Point3d triangulatedGS;
    triangulatedGS.x = CV_MAT_ELEM(*leftCameraTriangulatedWorldPoints,double,0,0);
    triangulatedGS.y = CV_MAT_ELEM(*leftCameraTriangulatedWorldPoints,double,0,1);
    triangulatedGS.z = CV_MAT_ELEM(*leftCameraTriangulatedWorldPoints,double,0,2);

    po_leftLens.m_Points.push_back(triangulatedGS);

    cvReleaseMat (&leftScreenPointsMat);
    cvReleaseMat (&rightScreenPointsMat);
    cvReleaseMat (&rightScreenPointsMat);
  }
  return po_leftLens;
}
//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::CalculateProjectionErrors (const std::string& outPrefix, const bool& useNewOutputFormat)
{
  if ( ! m_ProjectOK )
  {
    MITK_ERROR << "Attempted to run CalculateProjectionErrors, before running project(), no result.";
    return;
  }

  if ( ! m_GoldStandardPointsClassifiedOK )
  {
    ClassifyGoldStandardPoints ();
  }

  //clear the result vectors
  m_LeftProjectionErrors.clear();
  m_RightProjectionErrors.clear();
  m_LeftReProjectionErrors.clear();
  m_RightReProjectionErrors.clear();

  // for each point in the gold standard vectors m_LeftGoldStandardPoints
  // find the corresponding point in m_ProjectedPoints and calculate the projection
  // error in pixels. We don't define what the point correspondence is, so
  // maybe assume that the closest point is the match? Should be valid as long as the
  // density of the projected points is significantly less than the expected errors
  // Then, estimate the mm error by taking the z measure from m_PointsInLeftLensCS
  // and projecting onto it.
  //
  for ( unsigned int i = 0 ; i < m_GoldStandardPoints.size() ; i ++ )
  {
    bool left = true;
    this->CalculateProjectionError( m_GoldStandardPoints[i]);
    this->CalculateReProjectionError ( m_GoldStandardPoints[i]);
  }
  if ( ! useNewOutputFormat )
  {
    this->WriteProjectionErrorsInOldFormat (outPrefix);
  }
  else
  {
    this->WriteProjectionErrorsInNewFormat (outPrefix);
  }

}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::WriteProjectionErrorsInOldFormat (const std::string& outPrefix)
{

  std::ofstream lpout;
  std::ofstream rpout;
  std::ofstream lrpout;
  std::ofstream rrpout;

  std::vector < cv::Point2d > leftProjectionErrors;
  for ( std::vector<mitk::PickedObject>::iterator it = m_LeftProjectionErrors.begin() ;
      it < m_LeftProjectionErrors.end() ; ++ it )
  {
    leftProjectionErrors.push_back ( cv::Point2d (it->m_Points[0].x, it->m_Points[0].y ));
  }

  std::vector < cv::Point2d > rightProjectionErrors;
  for ( std::vector<mitk::PickedObject>::iterator it = m_RightProjectionErrors.begin() ;
      it < m_RightProjectionErrors.end() ; ++ it )
  {
    rightProjectionErrors.push_back ( cv::Point2d (it->m_Points[0].x, it->m_Points[0].y ));
  }

  lpout.open (std::string (outPrefix + "_leftProjection.errors").c_str());
  lpout << "#xpixels ypixels" << std::endl;
  for ( unsigned int i = 0 ; i < leftProjectionErrors.size() ; i ++ )
  {
    lpout << leftProjectionErrors[i] << std::endl;
  }
  cv::Point2d errorStdDev;
  cv::Point2d errorMean;
  errorMean = mitk::GetCentroid(leftProjectionErrors, false, &errorStdDev);
  lpout << "#Mean Error     = " << errorMean << std::endl;
  lpout << "#StdDev         = " << errorStdDev << std::endl;
  double xrms = sqrt ( errorMean.x * errorMean.x + errorStdDev.x * errorStdDev.x );
  double yrms = sqrt ( errorMean.y * errorMean.y + errorStdDev.y * errorStdDev.y );
  double rms = sqrt ( xrms*xrms + yrms*yrms);
  lpout << "#rms            = " << xrms << ", " << yrms << ", " << rms << std::endl;
  errorMean = mitk::GetCentroid(leftProjectionErrors, true, &errorStdDev);
  lpout << "#Ref Mean Error = " << errorMean << std::endl;
  lpout << "#Ref StdDev     = " << errorStdDev << std::endl;
  xrms = sqrt ( errorMean.x * errorMean.x + errorStdDev.x * errorStdDev.x );
  yrms = sqrt ( errorMean.y * errorMean.y + errorStdDev.y * errorStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  lpout << "#Ref. rms       = " << xrms << ", " << yrms << ", " << rms << std::endl;
  lpout.close();

  rpout.open (std::string (outPrefix + "_rightProjection.errors").c_str());
  rpout << "#xpixels ypixels" << std::endl;
  for ( unsigned int i = 0 ; i < rightProjectionErrors.size() ; i ++ )
  {
    rpout << rightProjectionErrors[i] << std::endl;
  }
  errorMean = mitk::GetCentroid(rightProjectionErrors, false, &errorStdDev);
  rpout << "#Mean Error      = " << errorMean << std::endl;
  rpout << "#StdDev          = " << errorStdDev << std::endl;
  xrms = sqrt ( errorMean.x * errorMean.x + errorStdDev.x * errorStdDev.x );
  yrms = sqrt ( errorMean.y * errorMean.y + errorStdDev.y * errorStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  rpout << "#rms             = " << xrms << ", " << yrms << ", " << rms << std::endl;
  errorMean = mitk::GetCentroid(rightProjectionErrors, true, &errorStdDev);
  rpout << "#Ref. Mean Error = " << errorMean << std::endl;
  rpout << "#Ref. StdDev     = " << errorStdDev << std::endl;
  xrms = sqrt ( errorMean.x * errorMean.x + errorStdDev.x * errorStdDev.x );
  yrms = sqrt ( errorMean.y * errorMean.y + errorStdDev.y * errorStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  rpout << "#Ref. rms        = " << xrms << ", " << yrms << ", " << rms << std::endl;
  rpout.close();

  std::vector<cv::Point3d> leftReProjectionErrors;
  for ( std::vector<mitk::PickedObject>::iterator it = m_LeftReProjectionErrors.begin() ;
      it < m_LeftReProjectionErrors.end() ; ++ it )
  {
    leftReProjectionErrors.push_back ( it->m_Points[0]);
  }
  std::vector<cv::Point3d> rightReProjectionErrors;
  for ( std::vector<mitk::PickedObject>::iterator it = m_RightReProjectionErrors.begin() ;
      it < m_RightReProjectionErrors.end() ; ++ it )
  {
    rightReProjectionErrors.push_back ( it->m_Points[0]);
  }

  lrpout.open(std::string (outPrefix + "_leftReProjection.errors").c_str());
  lrpout << "#xmm ymm zmm" << std::endl;
  for ( unsigned int i = 0 ; i < m_LeftReProjectionErrors.size() ; i ++ )
  {
    lrpout << m_LeftReProjectionErrors[i].m_Points[0] << std::endl;
  }
  cv::Point3d error3dStdDev;
  cv::Point3d error3dMean;
  error3dMean = mitk::GetCentroid(leftReProjectionErrors, false, &error3dStdDev);
  lrpout << "#Mean Error      = " << error3dMean << std::endl;
  lrpout << "#StdDev          = " << error3dStdDev << std::endl;
  xrms = sqrt ( error3dMean.x * error3dMean.x + error3dStdDev.x * error3dStdDev.x );
  yrms = sqrt ( error3dMean.y * error3dMean.y + error3dStdDev.y * error3dStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  lrpout << "#rms             = " << xrms << ", " << yrms << ", " << rms << std::endl;
  error3dMean = mitk::GetCentroid(leftReProjectionErrors, true, &error3dStdDev);
  lrpout << "#Ref. Mean Error = " << error3dMean << std::endl;
  lrpout << "#Ref. StdDev     = " << error3dStdDev << std::endl;
  xrms = sqrt ( error3dMean.x * error3dMean.x + error3dStdDev.x * error3dStdDev.x );
  yrms = sqrt ( error3dMean.y * error3dMean.y + error3dStdDev.y * error3dStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  lrpout << "#Ref. rms        = " << xrms << ", " << yrms << ", " << rms << std::endl;
  lrpout.close();

  rrpout.open(std::string (outPrefix + "_rightReProjection.errors").c_str());
  rrpout << "#xpixels ypixels" << std::endl;
  for ( unsigned int i = 0 ; i < m_RightReProjectionErrors.size() ; i ++ )
  {
    rrpout << m_RightReProjectionErrors[i].m_Points[0] << std::endl;
  }
  error3dMean = mitk::GetCentroid(rightReProjectionErrors, false, &error3dStdDev);
  rrpout << "#Mean Error      = " << error3dMean << std::endl;
  rrpout << "#StdDev          = " << error3dStdDev << std::endl;
  xrms = sqrt ( error3dMean.x * error3dMean.x + error3dStdDev.x * error3dStdDev.x );
  yrms = sqrt ( error3dMean.y * error3dMean.y + error3dStdDev.y * error3dStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  rrpout << "#rms             = " << xrms << ", " << yrms << ", " << rms << std::endl;
  error3dMean = mitk::GetCentroid(rightReProjectionErrors, true, &error3dStdDev);
  rrpout << "#Ref. Mean Error = " << error3dMean << std::endl;
  rrpout << "#Ref. StdDev     = " << error3dStdDev << std::endl;
  xrms = sqrt ( error3dMean.x * error3dMean.x + error3dStdDev.x * error3dStdDev.x );
  yrms = sqrt ( error3dMean.y * error3dMean.y + error3dStdDev.y * error3dStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  rrpout << "#Ref. rms        = " << xrms << ", " << yrms << ", " << rms << std::endl;
  rrpout.close();

}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::WriteProjectionErrorsInNewFormat (const std::string& outPrefix)
{

  std::ofstream out;

  out.open (std::string (outPrefix + "_ReProjectionErrors.txt").c_str());
  out << "# frame , type , id , channel , xmm , ymm , zmm , gsx , gsy , gsz , mpx , mpy , mpz << std::endl";

  std::vector<cv::Point3d> reProjectionErrors;
  for ( std::vector<mitk::PickedObject>::iterator it = m_LeftReProjectionErrors.begin() ;
      it < m_LeftReProjectionErrors.end() ; ++ it )
  {
    reProjectionErrors.push_back ( it->m_Points[0]);
    std::string type;
    if ( it->m_IsLine )
    {
      type = "line";
    }
    else
    {
      type = "point";
    }
    out << it->m_FrameNumber << " , " << type << " , " << it->m_Id << " , " << it->m_Channel << " , ";
    out << it->m_Points[0].x << " , " << it->m_Points[0].y << " , " << it->m_Points[0].z << " , ";
    out << it->m_Points[1].x << " , " << it->m_Points[1].y << " , " << it->m_Points[1].z << " , ";
    out << it->m_Points[2].x << " , " << it->m_Points[2].y << " , " << it->m_Points[2].z << " , ";
    out << std::endl;
  }

  for ( std::vector<mitk::PickedObject>::iterator it = m_RightReProjectionErrors.begin() ;
      it < m_RightReProjectionErrors.end() ; ++ it )
  {
    reProjectionErrors.push_back ( it->m_Points[0]);
    std::string type;
    if ( it->m_IsLine )
    {
      type = "line";
    }
    else
    {
      type = "point";
    }
    out << it->m_FrameNumber << " , " << type << " , " << it->m_Id << " , " << it->m_Channel << " , ";
    out << it->m_Points[0].x << " , " << it->m_Points[0].y << " , " << it->m_Points[0].z << " , ";
    out << it->m_Points[1].x << " , " << it->m_Points[1].y << " , " << it->m_Points[1].z << " , ";
    out << it->m_Points[2].x << " , " << it->m_Points[2].y << " , " << it->m_Points[2].z << " , ";
    out << std::endl;
  }


  cv::Point3d error3dStdDev;
  cv::Point3d error3dMean;
  double xrms;
  double yrms;
  double rms;

  error3dMean = mitk::GetCentroid(reProjectionErrors, false, &error3dStdDev);
  out << "#Mean Error      = " << error3dMean << std::endl;
  out << "#StdDev          = " << error3dStdDev << std::endl;
  xrms = sqrt ( error3dMean.x * error3dMean.x + error3dStdDev.x * error3dStdDev.x );
  yrms = sqrt ( error3dMean.y * error3dMean.y + error3dStdDev.y * error3dStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  out << "#rms             = " << xrms << ", " << yrms << ", " << rms << std::endl;
  error3dMean = mitk::GetCentroid(reProjectionErrors, true, &error3dStdDev);
  out << "#Ref. Mean Error = " << error3dMean << std::endl;
  out << "#Ref. StdDev     = " << error3dStdDev << std::endl;
  xrms = sqrt ( error3dMean.x * error3dMean.x + error3dStdDev.x * error3dStdDev.x );
  yrms = sqrt ( error3dMean.y * error3dMean.y + error3dStdDev.y * error3dStdDev.y );
  rms = sqrt ( xrms*xrms + yrms*yrms);
  out << "#Ref. rms        = " << xrms << ", " << yrms << ", " << rms << std::endl;
  out.close();

}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::CalculateReProjectionError ( mitk::PickedObject GSPoint )
{
  mitk::PickedObject toMatch = GSPoint.CopyByHeader();
  toMatch.m_Channel = "left_lens";
  mitk::PickedObject matchingObject = GetMatchingPickedObject ( toMatch, *m_PointsInLeftLensCS[GSPoint.m_FrameNumber] );
  assert ( matchingObject.m_FrameNumber == GSPoint.m_FrameNumber );

  if ( GSPoint.m_Channel != "left" )
  {
    //transform points from left right lens
    for ( unsigned int i = 0 ; i < matchingObject.m_Points.size() ; i ++ )
    {

      cv::Mat m1 = cvCreateMat(3,1,CV_64FC1);
      m1.at<double>(0,0) = matchingObject.m_Points[i].x;
      m1.at<double>(1,0) = matchingObject.m_Points[i].y;
      m1.at<double>(2,0) = matchingObject.m_Points[i].z;

      m1 = m_RightToLeftRotationMatrix->inv() * m1 - *m_RightToLeftTranslationVector;

      matchingObject.m_Points[i].x = m1.at<double>(0,0);
      matchingObject.m_Points[i].y = m1.at<double>(1,0);
      matchingObject.m_Points[i].z = m1.at<double>(2,0);
    }
  }

  mitk::PickedObject undistortedObject = UndistortPickedObject ( GSPoint );
  mitk::PickedObject reprojectedObject = ReprojectPickedObject ( undistortedObject, matchingObject );

  reprojectedObject.m_Channel = "left_lens";

  mitk::PickedObject reprojectionError;
  reprojectedObject.DistanceTo ( matchingObject, reprojectionError, m_AllowableTimingError);

  //for lines there will be a small residual z error, as the closest point to the projected line may not be
  //on the plane. Let's check that this remains fairly small
  if ( fabs (reprojectionError.m_Points[0].z ) < m_ReprojectionErrorZLimit )
  {
    if ( GSPoint.m_Channel == "left" )
    {
      m_LeftReProjectionErrors.push_back (reprojectionError);
    }
    else
    {
      m_RightReProjectionErrors.push_back (reprojectionError);
    }
  }
  else
  {
    MITK_WARN << "Rejecting reprojection error for point id " << reprojectedObject.m_Id <<
      " channel " << GSPoint.m_Channel <<
      " frame " << reprojectedObject.m_FrameNumber <<
      " as z component error is too high : " << reprojectionError.m_Points[0].z;
  }
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::CalculateProjectionError ( mitk::PickedObject GSPoint )
{
  mitk::PickedObject matchingObject = GetMatchingPickedObject ( GSPoint, *m_ProjectedPointLists[GSPoint.m_FrameNumber] );
  assert ( matchingObject.m_FrameNumber == GSPoint.m_FrameNumber );

  mitk::PickedObject projectionError;
  GSPoint.DistanceTo(matchingObject, projectionError, m_AllowableTimingError );
  if ( GSPoint.m_Channel == "left" )
  {
    m_LeftProjectionErrors.push_back( projectionError);
  }
  else
  {
    m_RightProjectionErrors.push_back( projectionError );
  }
}

//-----------------------------------------------------------------------------
bool ProjectPointsOnStereoVideo::FindNearestScreenPoint ( mitk::PickedObject& GSPickedObject )
{
  //we're assuming that  m_ClassifierProjectedPoints is in order of frames, better check this
  assert ( m_ProjectedPointLists[GSPickedObject.m_FrameNumber].IsNotNull() );
  assert ( GSPickedObject.m_FrameNumber ==  m_ProjectedPointLists[GSPickedObject.m_FrameNumber]->GetFrameNumber() );
  //let's check the timing errors while we're here
  long long timingError = static_cast<long long> ( GSPickedObject.m_TimeStamp ) -  static_cast <long long> (m_ProjectedPointLists[GSPickedObject.m_FrameNumber]->GetTimeStamp() ) ;
  if ( std::abs ( timingError ) > m_AllowableTimingError )

  {
    MITK_WARN << "Rejecting gold standard points at frame " << GSPickedObject.m_FrameNumber << " due to high timing error = " << timingError;
    return false;
  }

  if ( GSPickedObject.m_Id != -1 )
  {
    //don't need to do anything
    return true;
  }

  MITK_INFO << "Finding nearest screen point in frame " <<  GSPickedObject.m_FrameNumber;
  assert ( m_ClassifierProjectedPointLists[GSPickedObject.m_FrameNumber].IsNotNull() );
  assert ( GSPickedObject.m_FrameNumber ==  m_ClassifierProjectedPointLists[GSPickedObject.m_FrameNumber]->GetFrameNumber() );
  std::vector<mitk::PickedObject> ClassifierPoints = m_ClassifierProjectedPointLists[GSPickedObject.m_FrameNumber]->GetPickedObjects();

  double minRatio;
  mitk::PickedObject nearestObject = mitk::FindNearestPickedObject( GSPickedObject , ClassifierPoints , &minRatio );
  if ( minRatio < m_AllowablePointMatchingRatio || boost::math::isinf ( minRatio ) )
  {
    MITK_WARN << "Ambiguous object match or infinite match Ratio at frame " << GSPickedObject.m_FrameNumber << " discarding object from gold standard vector";
    return false;
  }

  GSPickedObject.m_Id = nearestObject.m_Id;
  return true;
}

//-----------------------------------------------------------------------------
mitk::PickedObject ProjectPointsOnStereoVideo::GetMatchingPickedObject ( const mitk::PickedObject& PickedObject ,
    const mitk::PickedPointList& PointList )
{
  assert ( PickedObject.m_FrameNumber ==  PointList.GetFrameNumber() );

  if ( PickedObject.m_Id == -1 )
  {
    MITK_ERROR << "Called get matching picked object with unclassified picked object, error";
    return mitk::PickedObject();
  }

  unsigned int matches = 0;
  mitk::PickedObject match;
  for ( unsigned int i = 0 ; i < PointList.GetListSize() ; i ++ )
  {
    if ( PickedObject.HeadersMatch ( PointList.GetPickedObjects()[i], m_AllowableTimingError ) )
    {
      match =  PointList.GetPickedObjects()[i];
      matches++;
    }
  }
  if ( matches == 0 )
  {
    MITK_ERROR << "Called get matching picked object but got not matches.";
  }
  if ( matches > 1 )
  {
    MITK_ERROR << "Called get matching picked object but multiple matches " << matches;
  }

  return match;
}

//-----------------------------------------------------------------------------
mitk::PickedObject ProjectPointsOnStereoVideo::UndistortPickedObject ( const mitk::PickedObject& po )
{
  assert ( po.m_Channel == "left" || po.m_Channel == "right" );
  mitk::PickedObject undistortedObject = po.CopyByHeader();
  bool cropUndistortedPointsToScreen = true;
  double cropValue = std::numeric_limits<double>::quiet_NaN();
  for ( unsigned int i = 0 ; i < po.m_Points.size () ; i ++ )
  {
    assert ( po.m_Points[i].z == 0 );
    cv::Point2d in = cv::Point2d ( po.m_Points[i].x, po.m_Points[i].y);
    cv::Point2d out;
    if ( po.m_Channel == "left" )
    {
      mitk::UndistortPoint (in, *m_LeftIntrinsicMatrix,
        *m_LeftDistortionVector, out,
        cropUndistortedPointsToScreen ,
        0.0, m_VideoWidth, 0.0, m_VideoHeight,cropValue);
    }
    else
    {
      mitk::UndistortPoint (in, *m_RightIntrinsicMatrix,
        *m_RightDistortionVector, out,
        cropUndistortedPointsToScreen ,
        0.0, m_VideoWidth, 0.0, m_VideoHeight,cropValue);
    }
    undistortedObject.m_Points.push_back(cv::Point3d ( out.x, out.y, 0.0 ));
  }

  return undistortedObject;

}

//-----------------------------------------------------------------------------
mitk::PickedObject ProjectPointsOnStereoVideo::ReprojectPickedObject ( const mitk::PickedObject& po, const mitk::PickedObject& reference )
{
  assert ( po.m_Channel == "left" || po.m_Channel == "right" );
  mitk::PickedObject reprojectedObject = po.CopyByHeader();

  for ( unsigned int i = 0 ; i < po.m_Points.size () ; i ++ )
  {
    assert ( po.m_Points[i].z == 0 );
    cv::Point2d in = cv::Point2d ( po.m_Points[i].x, po.m_Points[i].y);
    cv::Point3d out;
    if ( po.m_Channel == "left" )
    {
      out = mitk::ReProjectPoint (in , *m_LeftIntrinsicMatrix);
    }
    else
    {
      out= mitk::ReProjectPoint ( in , *m_RightIntrinsicMatrix);
    }

    double depth = mitk::GetCentroid ( reference.m_Points ).z;
    double shortestDistance = std::numeric_limits<double>::infinity();
    if ( reference.m_IsLine )
    {
      if ( reference.m_Points.size () > 1 )
      {
        for ( std::vector<cv::Point3d>::const_iterator it = reference.m_Points.begin() + 1 ; it < reference.m_Points.end() ; ++ it )
        {
          cv::Point3d closestPointOnReference;
          double distance = mitk::DistanceBetweenLineAndSegment ( cv::Point3d (0.0, 0.0, 0.0), out , *(it-1), *(it), closestPointOnReference);
          if ( distance < shortestDistance )
          {
            shortestDistance = distance;
            depth = closestPointOnReference.z;
          }
        }
      }
    }
    out.x *= depth;
    out.y *= depth;
    out.z *= depth;
    reprojectedObject.m_Points.push_back(out);
  }
  return reprojectedObject;
}

//-----------------------------------------------------------------------------
mitk::PickedPointList::Pointer  ProjectPointsOnStereoVideo::ProjectPickedPointList ( const mitk::PickedPointList::Pointer pl_leftLens,
    const double& screenBuffer )
{
  //these should be projected to the right or left lens depending on the frame number, even for left, odd for right
  assert ( pl_leftLens->GetChannel() == "left_lens" );

  mitk::PickedPointList::Pointer projected_pl = pl_leftLens->CopyByHeader();

  assert (m_LeftGSFramesAreEven); //I'm not sure what would happen here if this wasn't the case

  cv::Mat leftCameraPositionToFocalPointUnitVector = cv::Mat(1,3,CV_64FC1);
        leftCameraPositionToFocalPointUnitVector.at<double>(0,0)=0.0;
        leftCameraPositionToFocalPointUnitVector.at<double>(0,1)=0.0;
        leftCameraPositionToFocalPointUnitVector.at<double>(0,2)=1.0;

  bool cropUndistortedPointsToScreen = true;
  double cropValue = std::numeric_limits<double>::infinity();

  std::vector < mitk::PickedObject > pickedObjects = pl_leftLens->GetPickedObjects();
  std::vector < mitk::PickedObject > projectedObjects;
  for ( unsigned int i = 0 ; i < pickedObjects.size() ; i ++ )
  {
    //project onto screen
    mitk::PickedObject projectedObject = pickedObjects[i].CopyByHeader();
    CvMat* outputLeftCameraWorldPointsIn3D = NULL;
    CvMat* outputLeftCameraWorldNormalsIn3D = NULL ;
    CvMat* output2DPointsLeft = NULL ;
    CvMat* output2DPointsRight = NULL;

    cv::Mat leftCameraWorldPoints = cv::Mat (pickedObjects[i].m_Points.size(),3,CV_64FC1);
    cv::Mat leftCameraWorldNormals = cv::Mat (pickedObjects[i].m_Points.size(),3,CV_64FC1);

    for ( unsigned int j = 0 ; j < pickedObjects[i].m_Points.size() ; j ++ )
    {
      leftCameraWorldPoints.at<double>(j,0) = pickedObjects[i].m_Points[j].x;
      leftCameraWorldPoints.at<double>(j,1) = pickedObjects[i].m_Points[j].y;
      leftCameraWorldPoints.at<double>(j,2) = pickedObjects[i].m_Points[j].z;
      leftCameraWorldNormals.at<double>(j,0) = 0.0;
      leftCameraWorldNormals.at<double>(j,1) = 0.0;
      leftCameraWorldNormals.at<double>(j,2) = -1.0;
    }
    //this isn't the most efficient way of doing it but it is consistent with previous implementation
    mitk::ProjectVisible3DWorldPointsToStereo2D
      ( leftCameraWorldPoints,leftCameraWorldNormals,
        leftCameraPositionToFocalPointUnitVector,
        *m_LeftIntrinsicMatrix,*m_LeftDistortionVector,
        *m_RightIntrinsicMatrix,*m_RightDistortionVector,
        *m_RightToLeftRotationMatrix,*m_RightToLeftTranslationVector,
        outputLeftCameraWorldPointsIn3D,
        outputLeftCameraWorldNormalsIn3D,
        output2DPointsLeft,
        output2DPointsRight,
        cropUndistortedPointsToScreen ,
        0.0 - m_ProjectorScreenBuffer, m_VideoWidth + m_ProjectorScreenBuffer,
        0.0 - m_ProjectorScreenBuffer, m_VideoHeight + m_ProjectorScreenBuffer,
        cropValue);

    if ( pl_leftLens->GetFrameNumber ()  % 2 == 0 )
    {
      for ( unsigned int j = 0 ; j < pickedObjects[i].m_Points.size() ; j ++ )
      {
        projectedObject.m_Points.push_back ( cv::Point3d ( CV_MAT_ELEM(*output2DPointsLeft,double,j,0), CV_MAT_ELEM(*output2DPointsLeft,double,j,1), 0.0 ) );
      }
      projectedObject.m_Channel = "left";
    }
    else
    {
      for ( unsigned int j = 0 ; j < pickedObjects[i].m_Points.size() ; j ++ )
      {
        projectedObject.m_Points.push_back ( cv::Point3d ( CV_MAT_ELEM(*output2DPointsRight,double,j,0), CV_MAT_ELEM(*output2DPointsRight,double,j,1), 0.0 ) );
      }
      projectedObject.m_Channel = "right";
    }
    projectedObjects.push_back (projectedObject);

    cvReleaseMat(&outputLeftCameraWorldPointsIn3D);
    cvReleaseMat(&outputLeftCameraWorldNormalsIn3D);
    cvReleaseMat(&output2DPointsLeft);
    cvReleaseMat(&output2DPointsRight);
  }
  projected_pl->SetPickedObjects ( projectedObjects );
  if ( pl_leftLens->GetFrameNumber ()  % 2 == 0 )
  {
    projected_pl->SetChannel ( "left" );
  }
  else
  {
    projected_pl->SetChannel ( "right" );
  }

  return projected_pl;
}

//-----------------------------------------------------------------------------
mitk::PickedPointList::Pointer  ProjectPointsOnStereoVideo::TransformPickedPointListToLeftLens ( const mitk::PickedPointList::Pointer pl_world, const cv::Mat& transform, const unsigned long long& timestamp, const int& framenumber)
{
  //these should be projected to the right or left lens depending on the frame number, even for left, odd for right
  assert ( pl_world->GetChannel() == "world" );

  mitk::PickedPointList::Pointer transformedList = pl_world->CopyByHeader();
  transformedList->SetTimeStamp ( timestamp );
  transformedList->SetFrameNumber (framenumber);

  std::vector < mitk::PickedObject > pickedObjects = pl_world->GetPickedObjects();

  for ( unsigned int i = 0 ; i < pickedObjects.size() ; i ++ )
  {
      pickedObjects[i].m_Points = transform * pickedObjects[i].m_Points;
      pickedObjects[i].m_TimeStamp = timestamp;
      pickedObjects[i].m_FrameNumber = framenumber;
      pickedObjects[i].m_Channel = "left_lens";
  }

  transformedList->SetPickedObjects (pickedObjects);
  transformedList->SetChannel ("left_lens");
  return transformedList;
}


//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::ClassifyGoldStandardPoints ()
{
  assert (m_ProjectOK);
  std::sort ( m_GoldStandardPoints.begin(), m_GoldStandardPoints.end());
  unsigned int startsize = m_GoldStandardPoints.size();
  MITK_INFO << "MITK classifing " << startsize << " gold standard points ";
  for ( std::vector<mitk::PickedObject>::iterator it = m_GoldStandardPoints.end() - 1  ; it >= m_GoldStandardPoints.begin() ; --it )
  {
    if ( ! this->FindNearestScreenPoint ( *it ) )
    {
      m_GoldStandardPoints.erase ( it );
    }
    else
    {
      if ( it->m_IsLine )
      {
        if ( it->m_Id > m_MaxGoldStandardLineIndex )
        {
          m_MaxGoldStandardLineIndex = it->m_Id ;
        }
      }
      else
      {
        if ( it->m_Id > m_MaxGoldStandardPointIndex )
        {
          m_MaxGoldStandardPointIndex = it->m_Id ;
        }
      }
    }
  }
  //now go through the vector and reset m_MaxGSLineIndex and m_MaxGSPointIndex
  MITK_INFO << "Removed " << startsize - m_GoldStandardPoints.size() << " objects from gold standard vector, " <<  m_GoldStandardPoints.size() << " objects left.";

  m_GoldStandardPointsClassifiedOK = true;
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::AppendWorldPoints (
    std::vector < mitk::WorldPoint > points )
{
  if ( m_WorldPoints.IsNull() )
  {
    m_WorldPoints = mitk::PickedPointList::New();
    m_WorldPoints->SetChannel ("world");
  }
  m_WorldPoints->SetInLineMode(false);
  m_WorldPoints->SetInOrderedMode(true);
  for ( unsigned int i = 0 ; i < points.size() ; i ++ )
  {
    m_WorldPoints->AddPoint ( points[i].m_Point, points[i].m_Scalar );
  }
  m_ProjectOK = false;
}
//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::AppendClassifierWorldPoints (
    std::vector < mitk::WorldPoint > points )
{
  if ( m_ClassifierWorldPoints.IsNull() )
  {
    m_ClassifierWorldPoints = mitk::PickedPointList::New();
    m_ClassifierWorldPoints->SetChannel ("world");
  }
  m_ClassifierWorldPoints->SetInLineMode(false);
  m_ClassifierWorldPoints->SetInOrderedMode(true);
  for ( unsigned int i = 0 ; i < points.size() ; i ++ )
  {
    m_ClassifierWorldPoints->AddPoint (points[i].m_Point, points[i].m_Scalar);
  }
  m_ProjectOK = false;
}

//-----------------------------------------------------------------------------
std::vector <mitk::PickedPointList::Pointer> ProjectPointsOnStereoVideo::GetPointsInLeftLensCS ()
{
  return m_PointsInLeftLensCS;
}

//-----------------------------------------------------------------------------
std::vector <mitk::ProjectedPointPairsWithTimingError> ProjectPointsOnStereoVideo::GetProjectedPoints ()
{
   MITK_ERROR << "get projected points is  broke";
   assert (false);
   return std::vector<mitk::ProjectedPointPairsWithTimingError>();
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::AppendWorldPointsByTriangulation
    (std::vector< mitk::ProjectedPointPair > onScreenPointPairs,
     std::vector< unsigned int > framenumber  ,
     mitk::VideoTrackerMatching::Pointer trackerMatcher,
     std::vector<double> * perturbation)
{
  assert ( framenumber.size() == onScreenPointPairs.size() );

  if ( m_WorldPoints.IsNull() )
  {
    m_WorldPoints = mitk::PickedPointList::New();
    m_WorldPoints->SetChannel ("world");
  }
  if ( ! trackerMatcher->IsReady () )
  {
    MITK_ERROR << "Attempted to triangulate points without tracking matrices.";
    return;
  }

  std::vector < mitk::WorldPoint > leftLensPoints =
    mitk::Triangulate ( onScreenPointPairs,
      *m_LeftIntrinsicMatrix,
      *m_LeftDistortionVector,
      *m_RightIntrinsicMatrix,
      *m_RightDistortionVector,
      *m_RightToLeftRotationMatrix,
      *m_RightToLeftTranslationVector,
      true,
      0.0, m_VideoWidth, 0.0 , m_VideoHeight,
      std::numeric_limits<double>::quiet_NaN());

    mitk::WorldPoint point;
    unsigned int wpSize=m_WorldPoints->GetListSize();

    for ( unsigned int i = 0 ; i < onScreenPointPairs.size() ; i ++ )
    {
      point = leftLensPoints[i];
      long long timingError;
      point =  trackerMatcher->GetCameraTrackingMatrix(
          framenumber[i] , &timingError , m_TrackerIndex, perturbation, m_ReferenceIndex) * point;
    if ( std::abs(timingError) < m_AllowableTimingError )
    {
      m_WorldPoints->AddPoint ( point.m_Point, point.m_Scalar );
      MITK_INFO << framenumber[i] << " " << onScreenPointPairs[i].m_Left << ","
        << onScreenPointPairs[i].m_Right << " => " << point.m_Point << " => " << m_WorldPoints->GetPickedObjects()[i+wpSize].m_Points[0];
    }
    else
    {
      MITK_WARN << framenumber[i] << "Point rejected due to excessive timing error: " << timingError << " > " << m_AllowableTimingError;
    }

  }
  m_ProjectOK = false;
}

void ProjectPointsOnStereoVideo::ProjectAxes()
{
  cv::Mat leftCameraAxesPoints = cv::Mat (4,3,CV_64FC1);
  cv::Mat leftCameraAxesNormals = cv::Mat (4,3,CV_64FC1);
  for ( int i = 0 ; i < 4 ; i ++ )
  {
    int j;
    for ( j = 0 ; j < 2 ; j ++ )
    {
      leftCameraAxesPoints.at<double>(i,j) = 0.0;
      leftCameraAxesNormals.at<double>(i,j) = 0.0;
    }
      leftCameraAxesPoints.at<double>(i,j) = 100.0;
      leftCameraAxesNormals.at<double>(i,j) = -1.0;
  }
  leftCameraAxesPoints.at<double>(1,0) = 100;
  leftCameraAxesPoints.at<double>(2,1) = 100;
  leftCameraAxesPoints.at<double>(3,2) = 200;

  CvMat* outputLeftCameraAxesPointsIn3D = NULL;
  CvMat* outputLeftCameraAxesNormalsIn3D = NULL ;
  CvMat* output2DAxesPointsLeft = NULL ;
  CvMat* output2DAxesPointsRight = NULL;

  cv::Mat CameraUnitVector = cv::Mat(1,3,CV_64FC1);
  CameraUnitVector.at<double>(0,0)=0;
  CameraUnitVector.at<double>(0,1)=0;
  CameraUnitVector.at<double>(0,2)=1.0;
  mitk::ProjectVisible3DWorldPointsToStereo2D
    ( leftCameraAxesPoints,leftCameraAxesNormals,
        CameraUnitVector,
        *m_LeftIntrinsicMatrix,*m_LeftDistortionVector,
        *m_RightIntrinsicMatrix,*m_RightDistortionVector,
        *m_RightToLeftRotationMatrix,*m_RightToLeftTranslationVector,
        outputLeftCameraAxesPointsIn3D,
        outputLeftCameraAxesNormalsIn3D,
        output2DAxesPointsLeft,
        output2DAxesPointsRight);

  MITK_INFO << leftCameraAxesPoints;
  for ( unsigned int i = 0 ; i < 4 ; i ++ )
  {
    MITK_INFO << i;
    mitk::ProjectedPointPair pointPair;
    pointPair.m_Left = cv::Point2d(CV_MAT_ELEM(*output2DAxesPointsLeft,double,i,0),CV_MAT_ELEM(*output2DAxesPointsLeft,double,i,1));
    pointPair.m_Right = cv::Point2d(CV_MAT_ELEM(*output2DAxesPointsRight,double,i,0),CV_MAT_ELEM(*output2DAxesPointsRight,double,i,1));
    MITK_INFO << "Left" << pointPair.m_Left << "Right" << pointPair.m_Right;

    m_ScreenAxesPoints.m_Points.push_back(pointPair);
  }
}
//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::ClearWorldPoints()
{
  m_WorldPoints->ClearList();
  m_WorldPoints = NULL;
  m_ProjectOK = false;
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::ClearModelPoints()
{
  m_ModelPoints->ClearList();
  m_ModelPoints = NULL;
  m_ProjectOK = false;
}

//-----------------------------------------------------------------------------
void ProjectPointsOnStereoVideo::ClearGoldStandardPoints()
{
  m_GoldStandardPoints.clear();
}


} // end namespace
