/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-10-31 06:45:24 +0000 (Mon, 31 Oct 2011) $
 Revision          : $Revision: 7634 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#include "mitkAffineTransformParametersDataNodeProperty.h"

namespace mitk
{

const std::string AffineTransformParametersDataNodeProperty::PropertyKey = "niftk.affinetransformparameters";

AffineTransformParametersDataNodeProperty::AffineTransformParametersDataNodeProperty()
{
  this->Identity();
}

AffineTransformParametersDataNodeProperty::AffineTransformParametersDataNodeProperty(const ParametersType& parameters)
{
  SetAffineTransformParameters(parameters);
}

AffineTransformParametersDataNodeProperty::~AffineTransformParametersDataNodeProperty()
{
}

void AffineTransformParametersDataNodeProperty::Identity()
{
  m_Parameters.resize(13);  // extra element at the end to tell if we are rotating about centre

  for (int i = 0; i < 13; i++)
  {
    m_Parameters[i] = 0;
  }
  m_Parameters[6] = 100; // scaling
  m_Parameters[7] = 100;
  m_Parameters[8] = 100;
}

const AffineTransformParametersDataNodeProperty::ParametersType& AffineTransformParametersDataNodeProperty::GetAffineTransformParameters() const
{
    return m_Parameters;
}

void AffineTransformParametersDataNodeProperty::SetAffineTransformParameters(const ParametersType& parameters)
{
  if (m_Parameters != parameters)
  {
    m_Parameters = parameters;
    this->Modified();
  }
}

std::string AffineTransformParametersDataNodeProperty::GetValueAsString() const
{
  std::stringstream myStr;
  myStr <<   "rx:" << m_Parameters[0] \
        << ", ry:" << m_Parameters[1] \
        << ", rz:" << m_Parameters[2] \
        << ", tx:" << m_Parameters[3] \
        << ", ty:" << m_Parameters[4] \
        << ", tz:" << m_Parameters[5] \
        << ", sx:" << m_Parameters[6] \
        << ", sy:" << m_Parameters[7] \
        << ", sz:" << m_Parameters[8] \
        << ", k1:" << m_Parameters[9] \
        << ", k2:" << m_Parameters[10] \
        << ", k3:" << m_Parameters[11] \
        << ", cc:" << m_Parameters[12];
  return myStr.str();
}

bool AffineTransformParametersDataNodeProperty::IsEqual(const BaseProperty& property) const
{
  const Self *other = dynamic_cast<const Self*>(&property);

  if(other==NULL) return false;

  ParametersType otherParameters = other->GetAffineTransformParameters();
  if (otherParameters.size() != m_Parameters.size()) return false;

  return m_Parameters == otherParameters;
}

bool AffineTransformParametersDataNodeProperty::Assign(const BaseProperty& property)
{
  const Self *other = dynamic_cast<const Self*>(&property);

  if(other==NULL) return false;

  ParametersType otherParameters = other->GetAffineTransformParameters();
  this->m_Parameters = otherParameters;

  return true;
}

} // end namespace
