/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkIGINVidiaDataSourceImpl_h
#define QmitkIGINVidiaDataSourceImpl_h

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QGLWidget>
#include <cuda.h>
#include <video/sdiinput.h>
#include <video/compress.h>
#include <video/decompress.h>
#include <opencv2/core/types_c.h>
#include <string>
#include "QmitkIGITimerBasedThread.h"


// after construction, call start() to kick off capture
// beware: all signal/event processing for this object needs to happen on this thread!
// so if you didnt call start(), trying to emit Bump will mess up.
class QmitkIGINVidiaDataSourceImpl : public QmitkIGITimerBasedThread
{
  Q_OBJECT


public:
  enum CaptureState
  {
    PRE_INIT,
    HW_ENUM,
    FAILED,     // something is broken. as of recently, signal drop out is considered failed!
    RUNNING,    // trying to capture
    DEAD,       // e.g. no suitable hardware in the system
    PLAYBACK
  };


public:
  QmitkIGINVidiaDataSourceImpl();
  ~QmitkIGINVidiaDataSourceImpl();

  int GetTextureId(unsigned int stream) const;
  QGLWidget* GetCaptureContext();

  /** Returns the SDI format. Might be different from capture image format. */
  video::StreamFormat GetFormat() const;
  /** Returns the pixel dimensions of the video data that is being captured. Can be different from the SDI format. */
  std::pair<int, int> GetCaptureFormat() const;
  int GetStreamCount() const;
  // FIXME: should have a getter for the above two to avoid excessive locking

  CaptureState GetCaptureState() const;
  std::string GetStateMessage() const;
  void Reset();

  video::SDIInput::InterlacedBehaviour GetFieldMode() const;
  void SetFieldMode(video::SDIInput::InterlacedBehaviour mode);

  /**
   * targetbuffer needs to have the correct(!) size to fit all channels stacked
   * together. Try something like cvCreateImage(cvSize(GetCaptureFormat().first, GetCaptureFormat().second * numstreams), IPL_DEPTH_8U, 4).
   * @returns number of channels, or zero in case of some error.
   * @throws nothing
   */
  int GetRGBAImage(unsigned int sequencenumber, IplImage* targetbuffer);

  // returns the next sequence number that has already been captured
  // following ihavealready.
  // returns zero if no new ones have arrived yet.
  video::FrameInfo GetNextSequenceNumber(unsigned int ihavealready) const;

  const char* GetWireFormatString() const;

  unsigned int GetCookie() const;

  bool IsRunning() const;

  std::string GetCompressionOutputFilename() const;
  void setCompressionOutputFilename(const std::string& name);
  // instead of emitting the compress signal directly you should call this function
  unsigned int CompressFrame(unsigned int sequencenumber);
  void StopCompression();

  /**
   * @throws std::runtime_error if something goes wrong.
   */
  void TryPlayback(const std::string& filename);
  // stops realtime capture if on=true and enables decompressor.
  // use GetRGBAImage() to retrieve a frame.
  void SetPlayback(bool on, int expectedstreamcount = 0);


protected:
  // repeatedly called by timer to check for new frames.
  virtual void OnTimeoutImpl();

  bool HasHardware() const;
  bool HasInput() const;

  // qt thread
  virtual void run();

  bool DumpNALIndex() const;


protected slots:
  void DoWakeUp();
  // can only be used with Qt::BlockingQueuedConnection!
  void DoCompressFrame(unsigned int sequencenumber, unsigned int* frameindex);
  void DoStopCompression();
  void DoGetRGBAImage(unsigned int sequencenumber, IplImage** img, unsigned int* streamcount);
  void DoTryPlayback(const char* filename, bool* ok, const char** errormsg);

signals:
  // bumping this thread means to wake it up from its timer sleep.
  // this is a queued connection.
  void SignalBump();
  // these are blocking queued connections!
  void SignalCompress(unsigned int sequencenumber, unsigned int* frameindex);
  void SignalStopCompression();
  void SignalGetRGBAImage(unsigned int sequencenumber, IplImage** img, unsigned int* streamcount);
  void SignalTryPlayback(const char* filename, bool* ok, const char** errormsg);

  // emitted when capture setup dies. should be connected with a non-blocking queued connection!
  void SignalFatalError(QString msg);

private:
  // has to be called with lock held!
  void InitVideo();

  /**
   * Reads from GPU-side textures into CPU-side memory, or a PBO (if one is bound to pixelpack).
   * BEWARE: this does not flip the image, it stays in bottom-left orientation!
   * @param bufferpitch in bytes
   * @param width in pixels
   * @param height in lines (equiv to pixels) of the full receiving image (i.e. with all channels stacked)
   */
  void ReadbackRGBA(char* buffer, std::size_t bufferpitch, int width, int height, int slot);

  /**
   * BEWARE: this will flip the image to top-left orientation!
   */
  void ReadbackViaPBO(char* buffer, std::size_t bufferpitch, int width, int height, int slot);

  void DecompressRGBA(unsigned int sequencenumber, IplImage** img, unsigned int* streamcountinimg);

  // any access to members needs to be locked
  mutable QMutex          lock;


  // all the sdi stuff needs an opengl context
  //  so we'll create our own
  QGLWidget*              oglwin;
  // we want to share our capture context with other render contexts (e.g. the preview widget)
  // but for that to work we need a hack because for sharing to work, the share-source cannot
  //  be current at the time of call. but our capture context is current (to the capture thread)
  //  all the time! so we just create a dummy context that shares with capture-context but itself
  //  is never ever current to any thread and hence can be shared with new widgets while capture-context
  //  is happily working away. and tada it works :)
  QGLWidget*              oglshare;

  CUcontext               cuContext;


  CaptureState            current_state;
  std::string             state_message;

  video::SDIDevice*       sdidev;
  video::SDIInput*        sdiin;
  video::StreamFormat     format;
  int                     streamcount;
  const char*             wireformat;

  // cached capture image dimensions, so that getters can be called from any thread.
  int                     m_CaptureWidth;
  int                     m_CaptureHeight;

  // we keep our own copy of the texture ids (instead of relying on sdiin)
  //  so that another thread can easily get these.
  // SDIInput is actively enforcing an opengl context check that's incompatible
  //  with the current threading situation.
  int                     textureids[4];

  video::Compressor*      compressor;

  struct SequenceNumberComparator
  {
    bool operator()(const video::FrameInfo& a, const video::FrameInfo& b) const;
  };

  // maps sequence numbers to ringbuffer slots
  std::map<video::FrameInfo, int, SequenceNumberComparator>   sn2slot_map;
  // maps ringbuffer slots to sequence numbers
  std::map<int, video::FrameInfo>                             slot2sn_map;


  video::Decompressor*    decompressor;


  // time stamp of the previous successfully captured frame.
  // this is used to detect a capture glitch without unconditionally blocking for new frames.
  // see QmitkIGINVidiaDataSourceImpl::OnTimeoutImpl().
  DWORD     m_LastSuccessfulFrame;

  // used in a log file to correlate times stamps, frame index and sequence number
  unsigned int    m_NumFramesCompressed;


  video::SDIInput::InterlacedBehaviour    m_FieldMode;

  // used to check whether any in-flight IGINVidiaDataType are still valid.
  // it is set during InitVideo().
  unsigned int        m_Cookie;


  std::string         m_CompressionOutputFilename;

  // pixel buffer objects used for async readback of the video frames.
  // some perf testing and profiling revealed that we spend a lot of time in the driver
  // for synchronously reading a texture image.
  std::vector<int>    m_ReadbackPBOs;
};


#endif // QmitkIGINVidiaDataSourceImpl_H
