/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkIGIDataType.h"

namespace niftk
{

//-----------------------------------------------------------------------------
IGIDataType::~IGIDataType()
{
}

//-----------------------------------------------------------------------------
IGIDataType::IGIDataType()
: m_TimeStamp(0)
, m_Duration(0)
, m_FrameId(0)
, m_IsSaved(false)
, m_ShouldBeSaved(false)
, m_FileName("")
{
}


//-----------------------------------------------------------------------------
IGIDataType::IGIDataType(const IGIDataType& other)
: m_TimeStamp(other.m_TimeStamp)
, m_Duration(other.m_Duration)
, m_FrameId(other.m_FrameId)
, m_IsSaved(other.m_IsSaved)
, m_ShouldBeSaved(other.m_ShouldBeSaved)
, m_FileName(other.m_FileName)
{
}


//-----------------------------------------------------------------------------
IGIDataType::IGIDataType(IGIDataType&& other)
: m_TimeStamp(std::move(other.m_TimeStamp))
, m_Duration(std::move(other.m_Duration))
, m_FrameId(std::move(other.m_FrameId))
, m_IsSaved(std::move(other.m_IsSaved))
, m_ShouldBeSaved(std::move(other.m_ShouldBeSaved))
, m_FileName(std::move(other.m_FileName))
{

}


//-----------------------------------------------------------------------------
IGIDataType& IGIDataType::operator=(const IGIDataType& other)
{
  this->Clone(other);
  return *this;
}


//-----------------------------------------------------------------------------
IGIDataType& IGIDataType::operator=(IGIDataType&& other)
{
  m_TimeStamp = other.m_TimeStamp;
  m_Duration = other.m_Duration;
  m_FrameId = other.m_FrameId;
  m_IsSaved = other.m_IsSaved;
  m_ShouldBeSaved = other.m_ShouldBeSaved;
  m_FileName = other.m_FileName;
  return *this;
}


//-----------------------------------------------------------------------------
void IGIDataType::Clone(const IGIDataType& other)
{
  m_TimeStamp = other.m_TimeStamp;
  m_Duration = other.m_Duration;
  m_FrameId = other.m_FrameId;
  m_IsSaved = other.m_IsSaved;
  m_ShouldBeSaved = other.m_ShouldBeSaved;
  m_FileName = other.m_FileName;
}

} // end namespace
