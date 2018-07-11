/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkPickPointsOnStereoVideo.h"
#include <mitkCameraCalibrationFacade.h>
#include <mitkOpenCVMaths.h>
#include <mitkOpenCVFileIOUtils.h>
#include <mitkOpenCVPointTypes.h>
#include <cv.h>
//#include <opencv2/highgui/highgui.hpp>
#include <highgui.h>
#include <niftkFileHelper.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

namespace mitk {

//-----------------------------------------------------------------------------
PickPointsOnStereoVideo::PickPointsOnStereoVideo()
: 
m_VideoIn("")
, m_Directory("")
, m_TrackerIndex(0)
, m_ReferenceIndex(-1)
, m_InitOK(false)
, m_ProjectOK(false)
, m_VideoWidth(1920)
, m_VideoHeight(540)
, m_Capture(NULL)
, m_AllowableTimingError (20e6) // 20 milliseconds 
, m_OrderedPoints(false)
, m_PickingLine(false)
, m_AskOverWrite(false)
, m_HaltOnVideoReadFail(true)
, m_FlipVideo(false)
, m_WriteAnnotatedImages(false)
, m_StartFrame(0)
, m_EndFrame(0)
, m_Frequency(50)
{
}

//-----------------------------------------------------------------------------
PickPointsOnStereoVideo::~PickPointsOnStereoVideo()
{
}

//-----------------------------------------------------------------------------
void PickPointsOnStereoVideo::Initialise(std::string directory)
{
  m_InitOK = false;
  m_Directory = directory;
  m_OutDirectory = m_Directory + niftk::GetFileSeparator() +  "PickedVideoPoints";  
  m_InitOK = true;
  return;

}

//-----------------------------------------------------------------------------
void PickPointsOnStereoVideo::Project(mitk::VideoTrackerMatching::Pointer trackerMatcher)
{
  if ( ! m_InitOK )
  {
    MITK_WARN << "Called project before initialise.";
    return;
  }
 
  if ( m_Capture == NULL ) 
  {
    m_VideoIn = niftk::FindVideoFile(m_Directory, niftk::Basename (niftk::Basename ( trackerMatcher->GetFrameMap() )));
    if ( m_VideoIn == "" ) 
    {
      m_InitOK = false;
      return;
    }
    try
    {
      m_Capture = mitk::InitialiseVideoCapture(m_VideoIn, (! m_HaltOnVideoReadFail )); 
    }
    catch (std::exception& e)
    {
      MITK_ERROR << "Caught exception " << e.what();
      exit(1);
    }
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

   
  m_ProjectOK = false;

  int framenumber = 0 ;
  int key = 0;

  int annotationLineThickness = 1;
  cvNamedWindow ("Left Channel", CV_WINDOW_AUTOSIZE);
  cvNamedWindow ("Right Channel", CV_WINDOW_AUTOSIZE);
     
  cv::Mat blankMat = cvCreateMat(10,100,CV_32FC3);

  cv::imshow("Left Channel", blankMat);
  cv::imshow("Right Channel", blankMat);

  unsigned long long startTime;
  trackerMatcher->GetVideoFrame(0, &startTime);
  while ( framenumber < trackerMatcher->GetNumberOfFrames() && key != 'q')
  {
    if ( ( m_StartFrame < m_EndFrame ) && ( framenumber < m_StartFrame || framenumber > m_EndFrame ) )
    {
      cv::Mat videoImage;
      m_Capture->read(videoImage);
      MITK_INFO << "Skipping frame " << framenumber;
      framenumber ++;
    }
    else
    {
      long long timingError;
      cv::Mat WorldToLeftCamera = trackerMatcher->GetCameraTrackingMatrix(framenumber, &timingError, m_TrackerIndex, NULL, m_ReferenceIndex).inv();
      
      double xScale = 1.0;
      if ( m_HalfImageWidth )
      {
        xScale = 0.5;
      }
      cv::Mat tempMat;
      cv::Mat leftVideoImage;
      cv::Mat rightVideoImage;
      m_Capture->read(tempMat);
      if ( m_FlipVideo )
      {
        int flipMode = 0 ; // flip around the x axis
        cv::flip(tempMat,leftVideoImage,flipMode);
      }
      else
      {
        leftVideoImage = tempMat.clone();
      }

      m_Capture->read(tempMat);
      if ( m_FlipVideo )
      {
        int flipMode = 0 ; // flip around the x axis
        cv::flip(tempMat,rightVideoImage,flipMode);
      }
      else
      {
        rightVideoImage = tempMat.clone();
      }

      if ( std::abs(timingError) <  m_AllowableTimingError )
      {
        key = cvWaitKey (20);

        if ( framenumber %m_Frequency == 0 ) 
        {
          unsigned long long timeStamp;
          trackerMatcher->GetVideoFrame(framenumber, &timeStamp);
          if ( m_PickingLine )
          {
            if ( m_OrderedPoints )
            {
              MITK_INFO << "Picking ordered line on frame pair " << framenumber << ", " << framenumber+1 << " [ " <<  (timeStamp - startTime)/1e9 << " s ] t to pick unordered, l to finish line, n for next frame, q to quit";
            }
            else
            {
              MITK_INFO << "Picking un ordered line on frame pair " << framenumber << ", " << framenumber+1 << " [ " << (timeStamp - startTime)/1e9 << " s ] t to pick ordered, l to finish line, n for next frame, q to quit";
            }
          } 
          else
          {
            if ( m_OrderedPoints )
            {
              MITK_INFO << "Picking ordered points on frame pair " << framenumber << ", " << framenumber+1 << " [ " <<  (timeStamp - startTime)/1e9 << " s ] t to pick unordered, l to pick a line, n for next frame, q to quit";
            }
            else 
            {
              MITK_INFO << "Picking un ordered points on frame pair " << framenumber << ", " << framenumber+1 << " [ " << (timeStamp - startTime)/1e9 << " s ] t to pick ordered, l to pick a line, n for next frame, q to quit";
            }
          }
          
          std::string leftOutName = m_OutDirectory + niftk::GetFileSeparator() + boost::lexical_cast<std::string>(timeStamp) + "_leftPoints";
          trackerMatcher->GetVideoFrame(framenumber+1, &timeStamp);
          std::string rightOutName = m_OutDirectory + niftk::GetFileSeparator() + boost::lexical_cast<std::string>(timeStamp) + "_rightPoints";
          bool overWriteLeft = true;
          bool overWriteRight = true;
          key = 0;
          if ( boost::filesystem::exists (leftOutName + ".xml") )
          {
            if ( m_AskOverWrite )
            {
              MITK_INFO << leftOutName + ".xml" << " exists, overwrite (y/n)";
              key = 0;
              while ( ! ( key == 'n' || key == 'y' ) )
              {
                key = cvWaitKey(20);
              }
              if ( key == 'n' ) 
              {
                overWriteLeft = false;
              }
              else
              {
                overWriteLeft = true;
              }
            }
            else
            {
              MITK_INFO << leftOutName + ".xml" << " exists, skipping.";
              overWriteLeft = false;
            }
          }

          key = 0;
          if ( boost::filesystem::exists (rightOutName + ".xml") )
          {
            if ( m_AskOverWrite ) 
            {
              MITK_INFO << rightOutName  + ".xml" << " exists, overwrite (y/n)";
              while ( ! ( key == 'n' || key == 'y' ) )
              {
                key = cv::waitKey(20);
              }
              if ( key == 'n' ) 
              {
                overWriteRight = false;
              }
              else
              {
                overWriteRight = true;
              }
            }
            else 
            {
              MITK_INFO << rightOutName + ".xml" << " exists, skipping.";
              overWriteRight = false;
            }
          }

          PickedPointList::Pointer leftPickedPoints = PickedPointList::New();
          PickedPointList::Pointer rightPickedPoints = PickedPointList::New();
          leftPickedPoints->SetFrameNumber (framenumber);
          leftPickedPoints->SetChannel ("left");
          leftPickedPoints->SetTimeStamp(timeStamp);
          leftPickedPoints->SetInLineMode (m_PickingLine);
          leftPickedPoints->SetInOrderedMode (m_OrderedPoints);
          leftPickedPoints->SetXScale ( 1.0 / xScale ); 
          rightPickedPoints->SetFrameNumber (framenumber + 1);
          rightPickedPoints->SetChannel ("right");
          rightPickedPoints->SetTimeStamp(timeStamp);
          rightPickedPoints->SetInLineMode (m_PickingLine);
          rightPickedPoints->SetInOrderedMode ( m_OrderedPoints);
          rightPickedPoints->SetXScale ( 1.0 / xScale ); 

          cv::Mat leftAnnotatedVideoImage = leftVideoImage.clone();
          cv::Mat rightAnnotatedVideoImage = rightVideoImage.clone();
          key = 0;
          if ( overWriteLeft  ||  overWriteRight  )
          {
            while ( key != 'n' && key != 'q' )
            {
              key = cv::waitKey(20);
              if ( key == 't' )
              {
                m_OrderedPoints = ! m_OrderedPoints;
                if ( m_OrderedPoints ) 
                {
                  MITK_INFO << "Switched to ordered points mode";
                }
                else
                {
                  MITK_INFO << "Switched to un ordered points mode";
                }
                leftPickedPoints->SetInOrderedMode (m_OrderedPoints);
                rightPickedPoints->SetInOrderedMode (m_OrderedPoints);
              }
              if ( key == 'l' )
              {
                m_PickingLine = ! m_PickingLine;
                if ( m_PickingLine ) 
                {
                  MITK_INFO << "Switched to line picking mode";
                }
                else
                {
                  MITK_INFO << "Exited line picking mode";
                }
                leftPickedPoints->SetInLineMode (m_PickingLine);
                rightPickedPoints->SetInLineMode (m_PickingLine);
              }
              if ( overWriteLeft )
              {
                cvSetMouseCallback("Left Channel",PointPickingCallBackFunc, leftPickedPoints);
                if ( leftPickedPoints->GetIsModified() )
                {
                  leftAnnotatedVideoImage = leftVideoImage.clone();
                  leftPickedPoints->AnnotateImage(leftAnnotatedVideoImage, annotationLineThickness);

                  std::ofstream leftPointOut ((leftOutName+ ".xml").c_str());
                  leftPickedPoints->PutOut( leftPointOut );
                  leftPointOut.close();
                  if ( m_WriteAnnotatedImages )
                  {
                    cv::imwrite(leftOutName + ".png" ,leftAnnotatedVideoImage);
                  }
                }
                
                cv::Mat limage;
                cv::resize ( leftAnnotatedVideoImage, limage, cv::Size(0,0), xScale, 1.0, CV_INTER_CUBIC);
                cv::imshow("Left Channel" , limage);
              }

              if ( overWriteRight )
              {
                cvSetMouseCallback("Right Channel",PointPickingCallBackFunc, rightPickedPoints);
                if ( rightPickedPoints->GetIsModified() )
                {
                  rightAnnotatedVideoImage = rightVideoImage.clone();
                  rightPickedPoints->AnnotateImage(rightAnnotatedVideoImage, annotationLineThickness);

                  std::ofstream rightPointOut ((rightOutName + ".xml").c_str());
                  rightPickedPoints->PutOut (rightPointOut);
                  rightPointOut.close();

                  if ( m_WriteAnnotatedImages )
                  {
                     cv::imwrite(rightOutName + ".png" ,rightAnnotatedVideoImage);
                  }
                }
                
                cv::Mat rimage;
                cv::resize ( rightAnnotatedVideoImage, rimage, cv::Size(0,0), xScale, 1.0, CV_INTER_CUBIC);
                cv::imshow("Right Channel" ,rimage);
              }
            }
          }
          cv::imshow("Left Channel", blankMat);
          cv::imshow("Right Channel", blankMat);
        } 
      }
      else
      {
        MITK_INFO << "Skipping frame " << framenumber << " high timing error " << timingError;
      }
      framenumber += 2;
    }
  }
  m_ProjectOK = true;
}

} // end namespace
