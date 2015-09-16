/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkFitPlaneToPointCloudWrapper.h"
#include <mitkIOUtil.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/segmentation/sac_segmentation.h>

namespace niftk
{

//-----------------------------------------------------------------------------
FitPlaneToPointCloudWrapper::FitPlaneToPointCloudWrapper()
  : m_PlaneCoefficients(new pcl::ModelCoefficients)
  , m_MinPlaneDistance(std::numeric_limits<float>::max())
  , m_MaxPlaneDistance(-std::numeric_limits<float>::max())
  , m_AvgPlaneDistance(0)
  , m_RmsPlaneDistance(0)
{
}


//-----------------------------------------------------------------------------
FitPlaneToPointCloudWrapper::~FitPlaneToPointCloudWrapper()
{
  // non-smarty-pants pointer
  delete m_PlaneCoefficients;
}


//-----------------------------------------------------------------------------
void FitPlaneToPointCloudWrapper::GetParameters(float& a, float& b, float& c, float& d) const
{
  if (m_PlaneCoefficients->values.size() != 4)
  {
    throw std::logic_error("Need to call FitPlane() first!");
  }

  a = m_PlaneCoefficients->values[0];
  b = m_PlaneCoefficients->values[1];
  c = m_PlaneCoefficients->values[2];
  d = m_PlaneCoefficients->values[3];
}


//-----------------------------------------------------------------------------
void FitPlaneToPointCloudWrapper::PrintOutput(std::ostream& log) const
{
  if (m_PlaneCoefficients->values.size() != 4)
  {
    throw std::logic_error("Need to call FitPlane() first!");
  }

  log << m_PlaneCoefficients->values[0] << ' '
      << m_PlaneCoefficients->values[1] << ' '
      << m_PlaneCoefficients->values[2] << ' '
      << m_PlaneCoefficients->values[3]
      << std::endl;

  log << "# plane coefficients above." << std::endl;
  log << "# plane fitting errors:" << std::endl;
  log << "# minimum distance to estimated plane: " << m_MinPlaneDistance << std::endl
      << "# maximum distance to estimated plane: " << m_MaxPlaneDistance << std::endl
      << "# average distance to estimated plane: " << m_AvgPlaneDistance << std::endl
      << "# rms distance to estimated plane: " << m_RmsPlaneDistance << std::endl
  ;
}


//-----------------------------------------------------------------------------
void FitPlaneToPointCloudWrapper::FitPlane(const std::string& filename)
{
  if (filename.empty())
    throw std::runtime_error("Point cloud file name cannot be empty");

  mitk::PointSet::Pointer pointset = mitk::IOUtil::LoadPointSet(filename);
  assert(pointset.IsNotNull());

  FitPlane(pointset);
}


//-----------------------------------------------------------------------------
void FitPlaneToPointCloudWrapper::FitPlane(const mitk::PointSet::Pointer& pointset)
{
  if (pointset.IsNull())
    throw std::runtime_error("Null pointset passed in");

  if (pointset->GetSize() < 4)
    throw std::runtime_error("Point set too small, need at least 4 points");

  // now convert it to a pcl representation.
  // this is infact a simple std::vector with all the points.
  pcl::PointCloud<pcl::PointXYZ>::Ptr  cloud(new pcl::PointCloud<pcl::PointXYZ>);
  for (mitk::PointSet::PointsConstIterator i = pointset->Begin(); i != pointset->End(); ++i)
  {
    const mitk::PointSet::PointType& p = i->Value();
    cloud->push_back(pcl::PointXYZ(p[0], p[1], p[2]));
  }

  // this is effectively the same as in the pcl tutorials.
  pcl::SACSegmentation<pcl::PointXYZ>   seg;
  seg.setOptimizeCoefficients(true);
  seg.setModelType(pcl::SACMODEL_PLANE);
  seg.setMethodType(pcl::SAC_RANSAC);
  seg.setDistanceThreshold(0.01);         // arbitrary?
  seg.setInputCloud(cloud);

  pcl::PointIndices::Ptr        inliers(new pcl::PointIndices);
  seg.segment(*inliers, *m_PlaneCoefficients);

  if (inliers->indices.size () == 0)
  {
    throw std::runtime_error("Could not estimate a planar model for the given dataset.");
  }

  // sanity check
  if (m_PlaneCoefficients->values.size() != 4)
  {
    throw std::runtime_error("Plane estimation did not come out with the expected 4 parameters");
  }

  // compute distance of each point to the fitted plane.
  float   planecoeffthingy = std::sqrt(m_PlaneCoefficients->values[0] * m_PlaneCoefficients->values[0] +
                                       m_PlaneCoefficients->values[1] * m_PlaneCoefficients->values[1] +
                                       m_PlaneCoefficients->values[2] * m_PlaneCoefficients->values[2]
                                      );

  for (pcl::PointCloud<pcl::PointXYZ>::const_iterator i = cloud->begin(); i != cloud->end(); ++i)
  {
    float   dist = std::abs(m_PlaneCoefficients->values[0] * i->x +
                            m_PlaneCoefficients->values[1] * i->y +
                            m_PlaneCoefficients->values[2] * i->z +
                            m_PlaneCoefficients->values[3]
                           );
    dist /= planecoeffthingy;

    m_RmsPlaneDistance += dist * dist;
    m_AvgPlaneDistance += dist;
    m_MinPlaneDistance = std::min(m_MinPlaneDistance, dist);
    m_MaxPlaneDistance = std::max(m_MaxPlaneDistance, dist);
  }
  m_AvgPlaneDistance /= cloud->size();
  m_RmsPlaneDistance = std::sqrt(m_RmsPlaneDistance / cloud->size());

  // done.
}

} // namespace
