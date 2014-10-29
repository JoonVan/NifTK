/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkIGINVidiaDataSource.h"
#include <mitkIGINVidiaDataType.h>
#include <../Conversion/ImageConversion.h>
#include <igtlTimeStamp.h>
#include <QTimer>
#include <QCoreApplication>
#include <QGLContext>
#include <QGLWidget>
#include <QWaitCondition>
#include <QMessageBox>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include "QmitkIGINVidiaDataSourceImpl.h"
#include <boost/typeof/typeof.hpp>
#include <mitkProperties.h>
#include <mitkLogMacros.h>


// note the trailing space
const char* QmitkIGINVidiaDataSource::s_NODE_NAME = "NVIDIA SDI stream ";


//-----------------------------------------------------------------------------
QmitkIGINVidiaDataSource::QmitkIGINVidiaDataSource(mitk::DataStorage* storage)
: QmitkIGILocalDataSource(storage)
, m_Pimpl(0), m_MipmapLevel(0), m_MostRecentSequenceNumber(1)
, m_WasSavingMessagesPreviously(false)
, m_ExpectedCookie(0)
, m_MostRecentlyPlayedbackTimeStamp(0)
, m_MostRecentlyUpdatedTimeStamp(0)
, m_CachedUpdate((IplImage*) 0, 0)
{
  this->SetName("QmitkIGINVidiaDataSource");
  this->SetType("Frame Grabber");
  this->SetDescription("NVidia SDI");
  this->SetStatus("Initialising...");

  try
  {
    m_Pimpl = new QmitkIGINVidiaDataSourceImpl;

    bool ok = false;
    ok = QObject::connect(m_Pimpl, SIGNAL(SignalFatalError(QString)), this, SLOT(ShowFatalErrorMessage(QString)), Qt::QueuedConnection);
    assert(ok);

    // pre-create any number of datastorage nodes to avoid threading issues
    for (int i = 0; i < 4; ++i)
    {
      std::ostringstream  nodename;
      nodename << s_NODE_NAME << i;

      mitk::DataNode::Pointer node = this->GetDataNode(nodename.str());
    }

    // needs to match GUI combobox's default!
    m_Pimpl->SetFieldMode((video::SDIInput::InterlacedBehaviour) DROP_ONE_FIELD);
    StartCapturing();
  }
  catch (const std::exception& e)
  {
    // FIXME: should we rethrow this?
    this->SetStatus(e.what());
  }
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::SetMipmapLevel(unsigned int l)
{
  if (IsCapturing())
  {
    throw std::runtime_error("Cannot change capture parameter while capture is in progress");
  }

  // whether the requested level is meaningful or not depends on the incoming video size.
  // i dont want to check that here!
  // so we just silently accept most values and clamp it later.

  if (l > 32)
  {
    // this would mean we have an input video of more than 4 billion by 4 billion pixels.
    // while technology will eventually get there, i think it's safe to assume that
    // somebody would have checked this code here by then.
    throw std::runtime_error("Requested mipmap level is not sane (> 32)");
  }

  m_MipmapLevel = l;
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::SetFieldMode(InterlacedBehaviour b)
{
  if (IsCapturing())
  {
    throw std::runtime_error("Cannot change capture parameter while capture is in progress");
  }

  if (b == STACK_FIELDS)
  {
    throw std::runtime_error("Support for interlaced field mode STACK_FIELDS has been removed");
  }

  // until i've figured out suitable error handling lets just assert
  assert(m_Pimpl);
  m_Pimpl->SetFieldMode((video::SDIInput::InterlacedBehaviour) b);
}


//-----------------------------------------------------------------------------
QmitkIGINVidiaDataSource::InterlacedBehaviour QmitkIGINVidiaDataSource::GetFieldMode() const
{
  assert(m_Pimpl);

  // we fudge the actual field mode to prevent STACK_FIELDS from showing up.
  // this (atm) only affects the data source gui. Update() will still do the right thing.
  InterlacedBehaviour internalfieldmode = (InterlacedBehaviour) m_Pimpl->GetFieldMode();
  switch (internalfieldmode)
  {
    case STACK_FIELDS:
      internalfieldmode = DROP_ONE_FIELD;
      break;
  }

  return internalfieldmode;
}


//-----------------------------------------------------------------------------
QmitkIGINVidiaDataSource::~QmitkIGINVidiaDataSource()
{
  // Try stop grabbing and threading etc.
  // We do need quite a bit of control over the actual threading setup because
  // we need to manage which thread is currently in charge of the capture context!
  this->StopCapturing();

  if (m_Pimpl)
  {
    bool ok = false;
    ok = QObject::disconnect(m_Pimpl, SIGNAL(SignalFatalError(QString)), this, SLOT(ShowFatalErrorMessage(QString)));
    assert(ok);

    delete m_Pimpl;
  }
}


//-----------------------------------------------------------------------------
bool QmitkIGINVidiaDataSource::CanHandleData(mitk::IGIDataType* data) const
{
  bool result = false;
  if (static_cast<mitk::IGINVidiaDataType*>(data) != NULL)
  {
    result = true;
  }
  return result;
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::SaveInBackground(bool s)
{
  // this method is probably not used anymore by the other data-source-manager bits: see GetSaveInBackground().
  // but we keep it here because the base-class still has that SetSaveInBackground property.

  // reset it
  IGIDataSource::SetSaveInBackground(false);

  if (s)
  {
    MITK_WARN << "WARNING: sdi data source does not support SaveInBackground(true)";
  }
}


//-----------------------------------------------------------------------------
bool QmitkIGINVidiaDataSource::GetSaveInBackground() const
{ 
  return false;
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::StartCapturing()
{
  m_MostRecentSequenceNumber = 1;
  // until i've figured out suitable error handling lets just assert
  assert(m_Pimpl);
  m_Pimpl->start();
  this->InitializeAndRunGrabbingThread(20);
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::StopCapturing()
{
  // grabbing thread needs to stop before we can stop sdi thread
  // otherwise it could still be sending signals to the not-anymore-existing sdi thread.
  StopGrabbingThread();

  m_ExpectedCookie = 0;
  if (m_Pimpl)
  {
    m_Pimpl->ForciblyStop();
  }
}


//-----------------------------------------------------------------------------
bool QmitkIGINVidiaDataSource::IsCapturing()
{
  bool result = m_Pimpl != 0;
  if (result)
  {
    result = m_Pimpl->isRunning();
  }
  return result;
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::GrabData()
{
  assert(m_Pimpl != 0);

  // always update status message
  this->SetStatus(m_Pimpl->GetStateMessage());

  // in case we got new sdi bits since last time we need to start counting from the beginning again.
  unsigned int cookie = m_Pimpl->GetCookie();
  if (cookie != m_ExpectedCookie)
  {
    m_MostRecentSequenceNumber = 1;
    m_ExpectedCookie = cookie;
  }

  if (m_Pimpl->IsRunning())
  {
    video::FrameInfo  sn = {0};
    sn.sequence_number = m_MostRecentSequenceNumber;

    while (sn.sequence_number != 0)
    {
      cookie = m_Pimpl->GetCookie();
      if (cookie != m_ExpectedCookie)
      {
        // sdi bits died while we were enumerating sequence numbers.
        // not much we can do so lets just stop.
        MITK_WARN << "SDI capture setup seems to have died while enumerating sequence numbers. Expect a few glitches.";
        break;
      }

      sn = m_Pimpl->GetNextSequenceNumber(m_MostRecentSequenceNumber);
      if (sn.sequence_number != 0)
      {
        mitk::IGINVidiaDataType::Pointer wrapper = mitk::IGINVidiaDataType::New();

        wrapper->SetValues(cookie, sn.sequence_number, sn.arrival_time);
        wrapper->SetTimeStampInNanoSeconds(sn.id);
        wrapper->SetDuration(this->m_TimeStampTolerance); // nanoseconds
        m_MostRecentSequenceNumber = sn.sequence_number;

        // under certain circumstances, this might call back into SaveData()
        this->AddData(wrapper.GetPointer());
      }
    }
  }
  else
  {
    m_MostRecentSequenceNumber = 1;
  }


  // because there's currently no notification when the user clicked stop-record
  // we need to check this way to clean up the compressor.
  if (m_WasSavingMessagesPreviously && !GetSavingMessages())
  {
    m_Pimpl->StopCompression();

    m_WasSavingMessagesPreviously = false;
    m_FrameMapLogFile.close();
  }
}


//-----------------------------------------------------------------------------
bool QmitkIGINVidiaDataSource::Update(mitk::IGIDataType* data)
{
  // until i've figured out suitable error handling lets just assert
  assert(m_Pimpl);

  bool result = true;

  mitk::IGINVidiaDataType::Pointer dataType = static_cast<mitk::IGINVidiaDataType*>(data);
  if (dataType.IsNotNull())
  {
    if (m_Pimpl->GetCookie() == dataType->GetCookie())
    {
      // we cache data storage updates, mainly for decompression.
      // its current implementation is quite heavy-weight and Update() will be called
      // at the GUI refresh rate, even if no new timestamp has been selected.
      if (m_MostRecentlyUpdatedTimeStamp != dataType->GetTimeStampInNanoSeconds())
      {
        std::pair<int, int> captureformat = m_Pimpl->GetCaptureFormat();
        int                 numstreams    = m_Pimpl->GetStreamCount();

        bool  neednewcacheimg = false;
        if (m_CachedUpdate.first)
        {
          neednewcacheimg |=  captureformat.first                != m_CachedUpdate.first->width;
          neednewcacheimg |= (captureformat.second * numstreams) != m_CachedUpdate.first->height;
        }
        else
          neednewcacheimg = true;

        if (neednewcacheimg)
        {
          if (m_CachedUpdate.first)
          {
            cvReleaseImage(&m_CachedUpdate.first);
          }
          m_CachedUpdate.first = cvCreateImage(cvSize(captureformat.first, captureformat.second * numstreams), IPL_DEPTH_8U, 4);
        }

        // one massive image, with all streams stacked in
        m_CachedUpdate.second = m_Pimpl->GetRGBAImage(dataType->GetSequenceNumber(), m_CachedUpdate.first);
      }

      // if copy-out failed then capture setup is broken, e.g. someone unplugged a cable
      if (m_CachedUpdate.second)
      {
        video::SDIInput::InterlacedBehaviour  currentFieldMode = m_Pimpl->GetFieldMode();
        // remember: stream format can be different from capture format. stream is what comes off the wire, capture is what goes to gpu.
        const video::StreamFormat             currentStreamFormat = m_Pimpl->GetFormat();
        // if the stream format is not interlaced then field mode will have no effect on the capture format.
        if (!currentStreamFormat.is_interlaced && !GetIsPlayingBack())
          currentFieldMode = video::SDIInput::DO_NOTHING_SPECIAL;

        // max 4 streams
        const int streamcount = m_CachedUpdate.second;
        for (int i = 0; i < streamcount; ++i)
        {
          std::ostringstream  nodename;
          nodename << s_NODE_NAME << i;

          mitk::DataNode::Pointer node = this->GetDataNode(nodename.str());
          if (node.IsNull())
          {
            MITK_ERROR << "Can't find mitk::DataNode with name " << nodename.str() << std::endl;
            this->SetStatus("Failed");
            cvReleaseImage(&m_CachedUpdate.first);
            m_CachedUpdate.first = 0;
            return false;
          }

          // height of each channel in the frame (possibly including two fields)
          int   channelHeight = m_CachedUpdate.first->height / streamcount;
          // by default, each channel has only one field (ie. full frame progressive or drop).
          int   fieldHeight = channelHeight;
          if (currentFieldMode == STACK_FIELDS)
          {
            // we are rounding down here, intentionally.
            // there are video formats with differing field heights (eg. ntsc) but because STACK_FIELDS was never
            // implemented correctly we dont know which field we are actually returning here. rounding down is the safe option.
            fieldHeight = channelHeight / 2;
          }
          IplImage  subimg;
          cvInitImageHeader(&subimg, cvSize((int) m_CachedUpdate.first->width, fieldHeight), IPL_DEPTH_8U, m_CachedUpdate.first->nChannels);
          cvSetData(&subimg, &m_CachedUpdate.first->imageData[i * channelHeight * m_CachedUpdate.first->widthStep], m_CachedUpdate.first->widthStep);

          // Check if we already have an image on the node.
          // We dont want to create a new one in that case (there is a lot of stuff going on
          // for allocating a new image).
          mitk::Image::Pointer imageInNode = dynamic_cast<mitk::Image*>(node->GetData());

          if (!imageInNode.IsNull())
          {
            // check size of image that is already attached to data node!
            bool haswrongsize = false;
            haswrongsize |= imageInNode->GetDimension(0) != subimg.width;
            haswrongsize |= imageInNode->GetDimension(1) != subimg.height;
            haswrongsize |= imageInNode->GetDimension(2) != 1;
            // check image type as well.
            haswrongsize |= imageInNode->GetPixelType().GetBitsPerComponent()   != subimg.depth;
            haswrongsize |= imageInNode->GetPixelType().GetNumberOfComponents() != subimg.nChannels;

            if (haswrongsize)
            {
              imageInNode = mitk::Image::Pointer();
            }
          }

          if (imageInNode.IsNull())
          {
            mitk::Image::Pointer convertedImage = niftk::CreateMitkImage(&subimg);
            node->SetData(convertedImage);
          }
          else
          {
            try
            {
              mitk::ImageWriteAccessor writeAccess(imageInNode);
              void* vPointer = writeAccess.GetData();

              // the mitk image is tightly packed
              // but the opencv image might not
              const unsigned int numberOfBytesPerLine = subimg.width * subimg.nChannels;
              if (numberOfBytesPerLine == static_cast<unsigned int>(subimg.widthStep))
              {
                std::memcpy(vPointer, subimg.imageData, numberOfBytesPerLine * subimg.height);
              }
              else
              {
                // if that is not true then something is seriously borked
                assert(subimg.widthStep >= numberOfBytesPerLine);

                // "slow" path: copy line by line
                for (int y = 0; y < subimg.height; ++y)
                {
                  // widthStep is in bytes while width is in pixels
                  std::memcpy(&(((char*) vPointer)[y * numberOfBytesPerLine]), &(subimg.imageData[y * subimg.widthStep]), numberOfBytesPerLine); 
                }
              }
            }
            catch(mitk::Exception& e)
            {
              MITK_ERROR << "Failed to copy OpenCV image to DataStorage due to " << e.what() << std::endl;
            }
          }

          imageInNode = dynamic_cast<mitk::Image*>(node->GetData());
          assert(imageInNode.IsNotNull());


          mitk::Vector3D    currentImageSpacing = imageInNode->GetGeometry()->GetSpacing();
          mitk::Vector3D    shouldbeImageSpacing;

          // this is internal field mode, which might have the obsolete stack configured (i.e. during playback).
          switch (currentFieldMode)
          {
            case STACK_FIELDS:
              // in case of stack, the subimage-stuffing-into-mitk will have discarded the bottom half.
            case DROP_ONE_FIELD:
            {
              shouldbeImageSpacing[0] = 1;
              shouldbeImageSpacing[1] = 2;
              shouldbeImageSpacing[2] = 1;
              break;
            }
            case DO_NOTHING_SPECIAL:
            {
              shouldbeImageSpacing[0] = 1;
              shouldbeImageSpacing[1] = 1;
              shouldbeImageSpacing[2] = 1;
              break;
            }
          }

          // only update spacing if necessary. it has a huge overhead because mitk keeps
          // allocating itk objects everytime we do this.
          // BUT: always check whether the image spacing we currently have and what we want match!
          // otherwise we run into some weird problems if we actually do have a progressive video source.
          if ((std::abs(currentImageSpacing[0] - shouldbeImageSpacing[0]) > 0.01) ||
              (std::abs(currentImageSpacing[1] - shouldbeImageSpacing[1]) > 0.01) ||
              (std::abs(currentImageSpacing[2] - shouldbeImageSpacing[2]) > 0.01))
          {
            imageInNode->GetGeometry()->SetSpacing(shouldbeImageSpacing);
          }


          node->Modified();
        } // for

        m_MostRecentlyUpdatedTimeStamp = dataType->GetTimeStampInNanoSeconds();
      }
      else
      {
        this->SetStatus("Failed");
        MITK_WARN << "Looks like SDI capture setup dropped out.";
      }
    }
    else
    {
      // this is not an error. there are simply stale IGINVidiaDataType in flight.
      this->SetStatus("...");
    }
  }
  else
    this->SetStatus("...");

  // We signal every time we receive data, rather than at the GUI refresh rate, otherwise video looks very odd.
  emit UpdateDisplay();

  return result;
}


//-----------------------------------------------------------------------------
bool QmitkIGINVidiaDataSource::SaveData(mitk::IGIDataType* data, std::string& outputFileName)
{
  assert(m_Pimpl != 0);

  // cannot record while playing back
  assert(!GetIsPlayingBack());

  bool success = false;
  outputFileName = "";

  // are we starting to record now
  if (m_WasSavingMessagesPreviously == false)
  {
    // FIXME: use qt for this
    //        see https://cmicdev.cs.ucl.ac.uk/trac/ticket/2546
    SYSTEMTIME  now;
    // we used to have utc here but all the other data sources use local time too.
    GetLocalTime(&now);

    std::string directoryPath = this->GetSaveDirectoryName();
    QDir directory(QString::fromStdString(directoryPath));
    if (directory.mkpath(QString::fromStdString(directoryPath)))
    {
      std::ostringstream    filename;
      filename << directoryPath << "/capture-" 
        << now.wYear << '_' << now.wMonth << '_' << now.wDay << '-' << now.wHour << '_' << now.wMinute << '_' << now.wSecond;

      std::string filenamebase = filename.str();

      // we need a map for frame number -> wall clock
      assert(!m_FrameMapLogFile.is_open());
      m_FrameMapLogFile.open((filenamebase + ".framemap.log").c_str());
      if (!m_FrameMapLogFile.is_open())
      {
        // should we continue if we dont have a frame map?
        std::cerr << "WARNING: could not create frame map file!" << std::endl;
      }
      else
      {
        // dump a header line
        m_FrameMapLogFile << "#framenumber_starting_at_zero sequencenumber channel timestamp" << std::endl;
      }

      m_Pimpl->setCompressionOutputFilename(filenamebase + ".264");

      // we need to keep track of what our current field mode is so that we can restore it during playback.
      std::ofstream   fieldmodefile((filenamebase + ".fieldmode").c_str());
      fieldmodefile << m_Pimpl->GetFieldMode() << std::endl;
      fieldmodefile <<
        "\n"
        "# only the single number above is interpreted, anything that follows is discarded.\n"
        "# field mode determines how fields of interlaced video are packed into full video frames.\n"
        "# this only applies to interlaced video, it has no meaning for progressive input.\n"
        "# " << DO_NOTHING_SPECIAL << " = DO_NOTHING_SPECIAL: interlaced video is treated as full frame.\n"
        "# " << DROP_ONE_FIELD << " = DROP_ONE_FIELD: only one field is captured, the other discarded.\n"
        "# " << STACK_FIELDS << " = STACK_FIELDS: [deprecated] both fields are stacked vertically.\n";
      fieldmodefile.close();
    }
  }

  // no record-stop-notification work around
  // this is checked in GrabData(), which is called periodically by the data-grabbing-thread.
  m_WasSavingMessagesPreviously = true;

  mitk::IGINVidiaDataType::Pointer dataType = static_cast<mitk::IGINVidiaDataType*>(data);
  if (dataType.IsNotNull())
  {
    if (m_Pimpl->GetCookie() == dataType->GetCookie())
    {
      unsigned int  seqnum = dataType->GetSequenceNumber();
      unsigned int  frameindex = m_Pimpl->CompressFrame(seqnum);
      if (frameindex > 0)
      {
        unsigned int t = frameindex - m_Pimpl->GetStreamCount();
        for (unsigned int i = t; i < frameindex; ++i)
        if (m_FrameMapLogFile.is_open())
        {
          m_FrameMapLogFile
            << (i) << '\t'
            << seqnum << '\t'
            << (i - t) << '\t'
            << dataType->GetTimeStampInNanoSeconds() << '\t'
            << std::endl;
        }
        success = true;
      }
    }
  }

  return success;
}


//-----------------------------------------------------------------------------
int QmitkIGINVidiaDataSource::GetNumberOfStreams()
{
  if (m_Pimpl == 0)
  {
    return 0;
  }

  return m_Pimpl->GetStreamCount();
}


//-----------------------------------------------------------------------------
const char* QmitkIGINVidiaDataSource::GetWireFormatString()
{
  if (m_Pimpl == 0)
  {
    return "Nothing";
  }

  return m_Pimpl->GetWireFormatString();
}


//-----------------------------------------------------------------------------
int QmitkIGINVidiaDataSource::GetCaptureWidth()
{
  if (m_Pimpl == 0)
  {
    return 0;
  }

  video::StreamFormat format = m_Pimpl->GetFormat();
  return format.get_width();
}


//-----------------------------------------------------------------------------
int QmitkIGINVidiaDataSource::GetCaptureHeight()
{
  if (m_Pimpl == 0)
  {
    return 0;
  }

  video::StreamFormat format = m_Pimpl->GetFormat();
  return format.get_height();
}


//-----------------------------------------------------------------------------
int QmitkIGINVidiaDataSource::GetRefreshRate()
{
  if (m_Pimpl == 0)
  {
    return 0;
  }

  video::StreamFormat format = m_Pimpl->GetFormat();
  return format.get_refreshrate();
}


//-----------------------------------------------------------------------------
QGLWidget* QmitkIGINVidiaDataSource::GetCaptureContext()
{
  if (m_Pimpl == 0)
  {
    return 0;
  }

  return m_Pimpl->GetCaptureContext();
}


//-----------------------------------------------------------------------------
int QmitkIGINVidiaDataSource::GetTextureId(int stream)
{
  if (m_Pimpl == 0)
  {
    return 0;
  }

  return m_Pimpl->GetTextureId(stream);
}


//-----------------------------------------------------------------------------
bool QmitkIGINVidiaDataSource::InitWithRecordedData(std::map<igtlUint64, PlaybackPerFrameInfo>& index, const std::string& path, igtlUint64* firstTimeStampInStore, igtlUint64* lastTimeStampInStore, bool forReal)
{
  igtlUint64    firstTimeStampFound = 0;
  igtlUint64    lastTimeStampFound  = 0;

  QDir directory(QString::fromStdString(path));
  if (directory.exists())
  {
    QStringList filters;
    filters << QString("capture-*.264");
    directory.setNameFilters(filters);
    directory.setFilter(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);

    QStringList nalfiles = directory.entryList();
    // currently, we are only writing a single huge file.
    if (nalfiles.size() > 1)
    {
      MITK_ERROR << "QmitkIGINVidiaDataSource: Warning: found more than one NAL file, will use " + nalfiles[0].toStdString() + " only!" << std::endl;
    }
    if (nalfiles.size() >= 1)
    {
      QString     basename = nalfiles[0].split(".264")[0];
      std::string nalfilename = (directory.path() + QDir::separator() + basename + ".264").toStdString();

      // try to open video file.
      // it will throw if something goes wrong.
      m_Pimpl->TryPlayback(nalfilename);

      // now we need to correlate frame numbers with timestamps
      index.clear();
      std::string     framemapfilename = (directory.path() + QDir::separator() + basename + ".framemap.log").toStdString();
      std::ifstream   framemapfile(framemapfilename.c_str());
      if (framemapfile.good())
      {
        std::string   commentline;
        std::getline(framemapfile, commentline);
        if (commentline[0] != '#')
        {
          throw std::runtime_error("Frame map has been tampered with");
        }

        unsigned int    framenumber     = -1;
        unsigned int    sequencenumber  = -1;
        unsigned int    channelnumber   = -1;
        igtlUint64      timestamp       = -1;

        while (framemapfile.good())
        {
          framemapfile >> framenumber;
          framemapfile >> sequencenumber;
          framemapfile >> channelnumber;
          framemapfile >> timestamp;

          if (timestamp == -1)
          {
            break;
          }

          // remember: the frame number stored in the framemap was intended to be used with ffmpeg.
          // but ffmpeg starts counting at 1 instead of zero.
          // so we need to substract one to get the framenumber for the decompressor.
          // WARNING: actually no! all pig data was recorded with zero-based index.
          //framenumber -= 1;

          assert(channelnumber < 4);
          index[timestamp].m_SequenceNumber = sequencenumber;
          index[timestamp].m_frameNumber[channelnumber] = framenumber;
        }

        if (!index.empty())
        {
          firstTimeStampFound = index.begin()->first;
          lastTimeStampFound  = (--(index.end()))->first;
        }

        int   fieldmode = -1;
        std::string     fieldmodefilename = (directory.path() + QDir::separator() + basename + ".fieldmode").toStdString();
        std::ifstream   fieldmodefile(fieldmodefilename.c_str());
        if (fieldmodefile.good())
        {
          fieldmodefile >> fieldmode;
        }
        fieldmodefile.close();
        switch (fieldmode)
        {
          default:
            MITK_ERROR << "Warning: unknown field mode for file " << basename.toStdString() << std::endl;
            fieldmode = STACK_FIELDS;
            // STACK_FIELDS used to be the default for previous pig recordings. but it has been deprecated since.
            // we still set this on pimpl, so that Update() will do the right thing of implicitly converting it to DROP_ONE_FIELD.
          case STACK_FIELDS:
          case DROP_ONE_FIELD:
          case DO_NOTHING_SPECIAL:
            if (forReal)
            {
              m_Pimpl->SetFieldMode((video::SDIInput::InterlacedBehaviour) fieldmode);
            }
            break;
        }
      }
      else
      {
        throw std::runtime_error("Frame map not readable");
      }
    }
  }

  if (firstTimeStampInStore)
  {
    *firstTimeStampInStore = firstTimeStampFound;
  }
  if (lastTimeStampInStore)
  {
    *lastTimeStampInStore = lastTimeStampFound;
  }

  return firstTimeStampFound != 0;
}


//-----------------------------------------------------------------------------
bool QmitkIGINVidiaDataSource::ProbeRecordedData(const std::string& path, igtlUint64* firstTimeStampInStore, igtlUint64* lastTimeStampInStore)
{
  // nothing we can do if we dont have suitable decoder bits
  if (m_Pimpl == 0)
  {
    return false;
  }

  bool  ok = false;
  try
  {
    std::map<igtlUint64, PlaybackPerFrameInfo> index;
    ok = InitWithRecordedData(index, path, firstTimeStampInStore, lastTimeStampInStore, false);
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Caught exception while probing for playback data: " << e.what();
    ok = false;
  }

  return ok;
}


//-----------------------------------------------------------------------------
QmitkIGINVidiaDataSource::PlaybackPerFrameInfo::PlaybackPerFrameInfo()
  : m_SequenceNumber(-1)
{
  // zero is a valid frame index, so use -1.
  m_frameNumber[0] = m_frameNumber[1] = m_frameNumber[2] = m_frameNumber[3] = -1;
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::StartPlayback(const std::string& path, igtlUint64 firstTimeStamp, igtlUint64 lastTimeStamp)
{
  // if we dont have decoder then other things should have failed earlier
  // and this method should not have been called.
  assert(m_Pimpl);

  StopGrabbingThread();
  ClearBuffer();

  m_MostRecentlyUpdatedTimeStamp = 0;

  bool ok = InitWithRecordedData(m_PlaybackIndex, path, 0, 0, true);
  assert(ok);

  // lets guess how many streams we have dumped into that file.
  int   streamcount = 0;
  for (int j = 0; j < 4; ++j, ++streamcount)
  {
    if (m_PlaybackIndex.begin()->second.m_frameNumber[j] == -1)
    {
      break;
    }
  }

  SetIsPlayingBack(true);
  m_Pimpl->SetPlayback(true, streamcount);

  emit UpdateDisplay();
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::StopPlayback()
{
  m_PlaybackIndex.clear();
  ClearBuffer();

  SetIsPlayingBack(false);
  m_Pimpl->SetPlayback(false);

  m_MostRecentlyUpdatedTimeStamp = 0;

  this->InitializeAndRunGrabbingThread(20); // 40ms = 25fps

  emit UpdateDisplay();
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::PlaybackData(igtlUint64 requestedTimeStamp)
{
  assert(GetIsPlayingBack());

  // dont replay the same timestamp over and over again.
  if (m_MostRecentlyPlayedbackTimeStamp != requestedTimeStamp)
  {
    BOOST_AUTO(i, m_PlaybackIndex.upper_bound(requestedTimeStamp));
    // so we need to pick the previous
    if (i != m_PlaybackIndex.begin())
    {
      --i;
    }
    if (i != m_PlaybackIndex.end())
    {
      mitk::IGINVidiaDataType::Pointer wrapper = mitk::IGINVidiaDataType::New();

      // gpu arrival time is bogus here. we've never used it for anything anyway.
      // also note: the pimpl decompressor index does not know anything about sequence numbers.
      // so we are using the frame number as a sequence number.
      wrapper->SetValues(m_Pimpl->GetCookie(), i->second.m_frameNumber[0], i->first);
      wrapper->SetTimeStampInNanoSeconds(i->first);
      wrapper->SetDuration(this->m_TimeStampTolerance); // nanoseconds
      m_MostRecentSequenceNumber = 1;
      this->AddData(wrapper.GetPointer());
      this->SetStatus("Playing");

      m_MostRecentlyPlayedbackTimeStamp = requestedTimeStamp;
    }
  }
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSource::ShowFatalErrorMessage(QString msg)
{
  QMessageBox msgBox;
  msgBox.setText("SDI video capture failed.");
  msgBox.setInformativeText(msg);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.exec();
}
