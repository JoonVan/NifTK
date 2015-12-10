/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkTrackingAndTimeStampsContainer_h
#define mitkTrackingAndTimeStampsContainer_h

#include "niftkOpenCVUtilsExports.h"
#include "mitkTimeStampsContainer.h"

namespace mitk
{

/**
 * \class TrackingAndTimeStampsContainer
 * \brief Contains a matched vector of timestamps, and 4x4 tracking Matrices.
 *
 * See also mitk::TimeStampsContainer.
 *
 * This class is not thread-safe.
 */
class NIFTKOPENCVUTILS_EXPORT TrackingAndTimeStampsContainer
{
public:

  /**
   * \brief Empties the list.
   */
  void Clear();

  /**
   * \brief Loads tracking data from directory.
   */
  int LoadFromDirectory(const std::string& dirName);

  /**
   * \brief Saves tracking data to directory.
   */
  int SaveToDirectory(const std::string& dirName);

  /**
   * \brief Simply adds a TimeStamp+Matrix combo to the list.
   *
   * Most use-cases will load call LoadFromDirectory().
   */
  void Insert(const TimeStampsContainer::TimeStamp& timeStamp, const cv::Matx44d& matrix);

  /**
   * \brief Returns the time stamp at a given frameNumber.
   */
  TimeStampsContainer::TimeStamp GetTimeStamp(std::vector<TimeStampsContainer::TimeStamp>::size_type frameNumber) const;

  /**
   * \brief Returns the matrix at a given frameNumber.
   */
  cv::Matx44d GetMatrix(std::vector<TimeStampsContainer::TimeStamp>::size_type frameNumber) const;

  /**
   * \brief Gets the number of items.
   */
  std::vector<TimeStampsContainer::TimeStamp>::size_type GetSize() const;

  /**
   * \see mitk::TimeStampsContainer::GetFrameNumber().
   */
  std::vector<TimeStampsContainer::TimeStamp>::size_type GetFrameNumber(const TimeStampsContainer::TimeStamp& timeStamp) const;

  /**
   * \see mitk::TimeStampsContainer::GetNearestTimeStamp()
   */
  TimeStampsContainer::TimeStamp GetNearestTimeStamp(const TimeStampsContainer::TimeStamp& timeStamp, long long *delta = NULL ) const;

  /**
   * \brief Extracts a matrix for the given time-stamp, by interpolating.
   * \param the desired time stamp and a holder to return the timing error.
   * \param and a bool which will be true if the timestamp is within the high and low bounds
   */
  cv::Matx44d InterpolateMatrix(const TimeStampsContainer::TimeStamp& timeStamp,
      long long& minError, bool& inBounds);

  /**
   * \brief Extracts a matrix for the given time-stamp, by using the nearest time stamp
   * \param the desired time stamp and a holder to return the timing error.
   * \param and a bool which will be true if the timestamp is within the high and low bounds
   */
  cv::Matx44d GetNearestMatrix(const TimeStampsContainer::TimeStamp& timeStamp,
      long long& error, bool& inBounds);

private:

  mitk::TimeStampsContainer  m_TimeStamps;
  std::vector<cv::Matx44d>   m_TrackingMatrices;
};

} // end namespace

#endif
