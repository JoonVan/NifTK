/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "itkUltrasoundPinCalibrationCostFunction.h"
#include <sstream>
#include <mitkOpenCVMaths.h>

namespace itk {

//-----------------------------------------------------------------------------
UltrasoundPinCalibrationCostFunction::UltrasoundPinCalibrationCostFunction()
: m_NumberOfParameters(1)
, m_NumberOfValues(1)
{
  m_InvariantPoint.x = 0;
  m_InvariantPoint.y = 0;
  m_InvariantPoint.z = 0;
  m_MillimetresPerPixel.x = 1;
  m_MillimetresPerPixel.y = 1;
}


//-----------------------------------------------------------------------------
UltrasoundPinCalibrationCostFunction::~UltrasoundPinCalibrationCostFunction()
{
}


//-----------------------------------------------------------------------------
unsigned int UltrasoundPinCalibrationCostFunction::GetNumberOfParameters(void) const
{
  return m_NumberOfParameters;
}


//-----------------------------------------------------------------------------
unsigned int UltrasoundPinCalibrationCostFunction::GetNumberOfValues(void) const
{
  return m_NumberOfValues;
}


//-----------------------------------------------------------------------------
void UltrasoundPinCalibrationCostFunction::SetMatrices(const std::vector< cv::Mat >& matrices)
{
  m_Matrices = matrices;
  this->Modified();
}


//-----------------------------------------------------------------------------
void UltrasoundPinCalibrationCostFunction::SetPoints(const std::vector< cv::Point2d > points)
{
  m_Points = points;
  this->Modified();
}


//-----------------------------------------------------------------------------
void UltrasoundPinCalibrationCostFunction::SetNumberOfParameters(const int& numberOfParameters)
{
  m_NumberOfParameters = numberOfParameters;
  this->Modified();
}


//-----------------------------------------------------------------------------
void UltrasoundPinCalibrationCostFunction::SetInvariantPoint(const cv::Point3d& invariantPoint)
{
  m_InvariantPoint = invariantPoint;
  this->Modified();
}


//-----------------------------------------------------------------------------
void UltrasoundPinCalibrationCostFunction::SetMillimetresPerPixel(const cv::Point2d& mmPerPix)
{
  m_MillimetresPerPixel = mmPerPix;
  this->Modified();
}


//-----------------------------------------------------------------------------
UltrasoundPinCalibrationCostFunction::MeasureType UltrasoundPinCalibrationCostFunction::GetValue(
  const ParametersType & parameters
  ) const
{
  if (parameters.GetSize() != m_NumberOfParameters)
  {
    std::ostringstream oss;
    oss << "UltrasoundPinCalibrationCostFunction::GetValue given " << parameters.GetSize() << ", but was expecting " << m_NumberOfParameters << " parameters";
    throw std::logic_error(oss.str());
  }

  if (m_Matrices.size() == 0)
  {
    throw std::logic_error("UltrasoundPinCalibrationCostFunction::GetValue(): No matrices available");
  }

  if (m_Points.size() == 0)
  {
    throw std::logic_error("UltrasoundPinCalibrationCostFunction::GetValue():No points available");
  }

  if (m_Matrices.size() != m_Points.size())
  {
    std::ostringstream oss;
    oss << "UltrasoundPinCalibrationCostFunction::GetValue(): The number of matrices (" << m_Matrices.size() << ") differs from the number of points (" << m_Points.size() << ")";
    throw std::logic_error(oss.str());
  }

  cv::Matx44d rigidTransformation = mitk::ConstructRigidTransformationMatrix(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5]);

  cv::Matx44d scalingTransformation;
  scalingTransformation.eye();

  cv::Matx44d invariantPointTranslation;
  invariantPointTranslation.eye();

  if (parameters.size() == 8 || parameters.size() == 11)
  {
    scalingTransformation(0, 0) = parameters[6];
    scalingTransformation(1, 1) = parameters[7];
  }
  else
  {
    // i.e. its not being optimised.
    scalingTransformation(0, 0) = m_MillimetresPerPixel.x;
    scalingTransformation(1, 1) = m_MillimetresPerPixel.y;
  }

  if (parameters.size() == 9 || parameters.size() == 11)
  {
    if (parameters.size() == 9)
    {
      invariantPointTranslation(0, 3) = parameters[6];
      invariantPointTranslation(1, 3) = parameters[7];
      invariantPointTranslation(2, 3) = parameters[8];
    }
    else
    {
      invariantPointTranslation(0, 3) = parameters[8];
      invariantPointTranslation(1, 3) = parameters[9];
      invariantPointTranslation(2, 3) = parameters[10];
    }
  }
  else
  {
    // i.e. its not being optimised.
    invariantPointTranslation(0, 3) = m_InvariantPoint.x;
    invariantPointTranslation(1, 3) = m_InvariantPoint.y;
    invariantPointTranslation(2, 3) = m_InvariantPoint.z;
  }

  MeasureType value;
  value[0] = 0;
  value[1] = 0;
  value[2] = 0;

  for (unsigned int i = 0; i < m_Matrices.size(); i++)
  {
    cv::Matx44d trackerTransformation(m_Matrices[i]);
    cv::Matx44d combinedTransformation = (invariantPointTranslation * (trackerTransformation * (rigidTransformation * scalingTransformation)));
    cv::Matx41d point, transformedPoint;

    point(0,0) = m_Points[i].x;
    point(1,0) = m_Points[i].y;
    point(2,0) = 1;
    point(3,0) = 1;

    transformedPoint = combinedTransformation * point;

    value[0] += transformedPoint(0,0) * transformedPoint(0,0);
    value[1] += transformedPoint(1,0) * transformedPoint(1,0);
    value[2] += transformedPoint(2,0) * transformedPoint(2,0);
  }

  return value;
}


//-----------------------------------------------------------------------------
void UltrasoundPinCalibrationCostFunction::GetDerivative(
  const ParametersType & parameters,
  DerivativeType  & derivative
  ) const
{

}



} // end namespace
