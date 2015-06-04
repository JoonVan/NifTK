/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkSequentialCpuQds_h
#define niftkSequentialCpuQds_h

#include "niftkSurfReconExports.h"
#include "niftkQDSCommon.h"
#include <opencv2/core/types_c.h>
// FIXME: typedefs.hpp should be enough! but there seem to be some issues...
//#include <boost/gil/typedefs.hpp>
#include <boost/gil/gil_all.hpp>
#include <vector>

#ifndef NIFTKSURFRECON_EXPORT
#define NIFTKSURFRECON_EXPORT
#endif

namespace niftk 
{

struct PropagationParameters
{
  int   WinSizeX;   // similarity window, in pixels
  int   WinSizeY;

  int   BorderX;    // border in pixels to ignore
  int   BorderY;

  float Ct;         // correlation threshold
  float Tt;         // texture threshold
//float Et;         // epipolar threshold

  int   N;          // search neighbourhood size, in pixels
  int   Dg;         // disparity gradient threshold, in pixels
};


// FIXME: this is actually shared between all qds variants, so refactor this (later)
struct NIFTKSURFRECON_EXPORT RefPoint
{
  unsigned short  x;
  unsigned short  y;

  RefPoint(unsigned short _x = -1, unsigned short _y = -1)
    : x(_x), y(_y)
  {
  }

  RefPoint& operator=(const RefPoint& p)
  {
    x = p.x;
    y = p.y;
    return *this;
  }

  // conversion operator! exists for convinience with refmaps
  // FIXME: pixel assignment is done with a template parameter
  //        so this conversion is not actually called automagically!
  operator boost::gil::dev2n16_pixel_t() const
  {
    return boost::gil::dev2n16_pixel_t(x, y);
  }

  // compares coordinates, for use in priority_queue
  bool operator<(const RefPoint& rhs) const;
};


#ifdef _MSC_VER
// various bits rely on safely dll-exporting class members which may reference
//  crt components (that may not be explicitly declared to be exported).
// this checks that we are building against the dll version of the crt.
#ifndef _DLL
#ifdef _MT
#error You are compiling against the static version of the CRT. This is not supported! Choose DLL instead!
#else
#pragma message("Warning: cannot tell which CRT version you are building with. Stuff might fail.")
#endif
#endif

#pragma warning(push)
#pragma warning(disable: 4251)      //  class '...' needs to have dll-interface to be used by clients of class '...'
#endif

class NIFTKSURFRECON_EXPORT SequentialCpuQds : public QDSInterface
{

public:
  SequentialCpuQds(int width, int height);
  virtual ~SequentialCpuQds();


  // supports greyscale, RGB or RGBA images.
  // input dimensions need to match the width and height passed into constructor.
  virtual void Process(const IplImage* left, const IplImage* right);


  virtual int GetWidth() const;
  virtual int GetHeight() const;

  // caller needs to clean up
  virtual IplImage* CreateDisparityImage() const;

  virtual CvPoint GetMatch(int x, int y) const;

private:
  SequentialCpuQds(const SequentialCpuQds& copyme);
  SequentialCpuQds& operator=(const SequentialCpuQds& assignme);


private:
  void InitSparseFeatures();

  void QuasiDensePropagation();


private:
  // internal buffers are of fixed size
  int     m_Width;
  int     m_Height;

  PropagationParameters   m_PropagationParams;

  // internal buffers
  // grayscale version of the input passed to Process()
  boost::gil::gray8_image_t   m_LeftImg;
  boost::gil::gray8_image_t   m_RightImg;
  // integral image of the grayscale version
  boost::gil::gray32s_image_t m_LeftIntegral;
  boost::gil::gray32s_image_t m_RightIntegral;
  // squared-integral
  boost::gil::gray64f_image_t m_LeftSquaredIntegral;
  boost::gil::gray64f_image_t m_RightSquaredIntegral;
  // texture descriptor, i.e. cornerness image
  boost::gil::gray8_image_t   m_LeftTexture;
  boost::gil::gray8_image_t   m_RightTexture;
  // refmap: pixel coordinate into the other view
  boost::gil::dev2n16_image_t m_LeftRefMap;
  boost::gil::dev2n16_image_t m_RightRefMap;

  // these have no mem allocated themselves!
  // they only reference the gil images
  IplImage    m_LeftIpl;
  IplImage    m_RightIpl;
  IplImage    m_LeftIntegralIpl;
  IplImage    m_RightIntegralIpl;
  IplImage    m_LeftSquaredIntegralIpl;
  IplImage    m_RightSquaredIntegralIpl;

  static const int            NUM_MAX_FEATURES = 500;
  std::vector<CvPoint2D32f>   m_SparseFeaturesLeft;
  std::vector<CvPoint2D32f>   m_SparseFeaturesRight;
  std::vector<char>           m_FeatureStatus;

  // used only for disparity image debugging
  const int   m_MaxDisparity;
};


#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // end namespace

#endif
