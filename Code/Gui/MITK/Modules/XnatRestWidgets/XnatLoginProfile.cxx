/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "XnatLoginProfile.h"

XnatLoginProfile::XnatLoginProfile()
{
  m_default = false;
}

XnatLoginProfile::~XnatLoginProfile()
{
}

QString XnatLoginProfile::name() const
{
  return m_name;
}

void XnatLoginProfile::setName(const QString& name)
{
  m_name = name;
}

QString XnatLoginProfile::serverUri() const
{
  return m_serverUri;
}

void XnatLoginProfile::setServerUri(const QString& serverUri)
{
  m_serverUri = serverUri;
}

QString XnatLoginProfile::userName() const
{
  return m_userName;
}

void XnatLoginProfile::setUserName(const QString& userName)
{
  m_userName = userName;
}

QString XnatLoginProfile::password() const
{
  return m_password;
}

void XnatLoginProfile::setPassword(const QString& password)
{
  m_password = password;
}

bool XnatLoginProfile::isDefault() const
{
  return m_default;
}

void XnatLoginProfile::setDefault(const bool& default_)
{
  m_default = default_;
}