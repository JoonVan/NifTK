/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkCoordinateAxesData.h"

#include <itkMatrix.h>
#include <itkVector.h>

#include <vtkSmartPointer.h>

#include <mitkVector.h>

#include "niftkFileIOUtils.h"

namespace niftk
{

const char* CoordinateAxesData::FILE_EXTENSION = ".4x4";
const char* CoordinateAxesData::FILE_EXTENSION_WITH_ASTERISK = "*.4x4";
const char* CoordinateAxesData::FILE_NAME = "CoordinateAxesTransform.4x4";
const char* CoordinateAxesData::FILE_DIALOG_PATTERN = "Coordinate Axes Transform (*.4x4)";
const char* CoordinateAxesData::FILE_DIALOG_NAME = "Coordinate Axes Transform 4x4";

//-----------------------------------------------------------------------------
CoordinateAxesData::CoordinateAxesData()
{
  IndexType i;
  i.Fill(0);

  SizeType s;
  s.Fill(10);

  m_LargestPossibleRegion.SetSize(s);
  m_LargestPossibleRegion.SetIndex(i);
  m_RequestedRegion.SetSize(s);
  m_RequestedRegion.SetIndex(i);

  Superclass::InitializeTimeGeometry();
}


//-----------------------------------------------------------------------------
CoordinateAxesData::~CoordinateAxesData()
{
}


//-----------------------------------------------------------------------------
void CoordinateAxesData::UpdateOutputInformation()
{
  Superclass::UpdateOutputInformation();
  this->GetTimeGeometry()->Update();
}


//-----------------------------------------------------------------------------
void CoordinateAxesData::SetRequestedRegionToLargestPossibleRegion()
{
  // Deliberately blank, as nothing to do.
}


//-----------------------------------------------------------------------------
bool CoordinateAxesData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}


//-----------------------------------------------------------------------------
bool CoordinateAxesData::VerifyRequestedRegion()
{
  return true;
}


//-----------------------------------------------------------------------------
void CoordinateAxesData::SetRequestedRegion(const itk::DataObject *data)
{
  // Deliberately blank, as nothing to do.
}


//-----------------------------------------------------------------------------
const CoordinateAxesData::RegionType& CoordinateAxesData::GetLargestPossibleRegion() const
{
  m_LargestPossibleRegion.SetIndex(3, 0);
  m_LargestPossibleRegion.SetSize(3, 1);
  return m_LargestPossibleRegion;
}


//-----------------------------------------------------------------------------
const CoordinateAxesData::RegionType& CoordinateAxesData::GetRequestedRegion() const
{
  return GetLargestPossibleRegion();
}


//-----------------------------------------------------------------------------
void CoordinateAxesData::GetVtkMatrix(vtkMatrix4x4& matrixToWriteTo) const
{
  mitk::BaseGeometry* geometry = this->GetGeometry();
  if (geometry)
  {
    mitk::AffineTransform3D::ConstPointer itkTrans = geometry->GetIndexToWorldTransform();
    itk::Matrix<mitk::ScalarType, 3,3> matrix = itkTrans->GetMatrix();
    itk::Vector<mitk::ScalarType, 3> offset = itkTrans->GetOffset();

    matrixToWriteTo.Identity();
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        matrixToWriteTo[i][j] = matrix[i][j];
      }
      matrixToWriteTo[i][3] = offset[i];
    }
  }
}


//-----------------------------------------------------------------------------
void CoordinateAxesData::SetVtkMatrix(const vtkMatrix4x4& matrix)
{
  mitk::BaseGeometry* geometry = this->GetGeometry();
  if (geometry)
  {
    geometry->SetIndexToWorldTransformByVtkMatrix(const_cast<vtkMatrix4x4*>(&matrix));
    geometry->Modified();
    this->UpdateOutputInformation();
    this->Modified();
  }
}


//-----------------------------------------------------------------------------
bool CoordinateAxesData::SaveToFile(const std::string& fileName) const
{
  vtkSmartPointer<vtkMatrix4x4> tmp = vtkSmartPointer<vtkMatrix4x4>::New();
  this->GetVtkMatrix(*tmp);
  return SaveVtkMatrix4x4ToFile(fileName, *tmp);
}


//-----------------------------------------------------------------------------
mitk::Point3D CoordinateAxesData::MultiplyPoint(const mitk::Point3D& point) const
{
  // ToDo: This could be made faster, by avoiding copying stuff.

  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->GetVtkMatrix(*matrix);

  double p[4];
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];
  p[3] = 1;
  matrix->MultiplyPoint(p, p);

  mitk::Point3D out;
  out[0] = p[0];
  out[1] = p[1];
  out[2] = p[2];
  return out;
}


//-----------------------------------------------------------------------------
void CoordinateAxesData::SetTranslation(const mitk::Point3D& translation)
{
  this->SetTranslation(translation[0], translation[1], translation[2]);
}


//-----------------------------------------------------------------------------
void CoordinateAxesData::SetTranslation(double tx, double ty, double tz)
{
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  matrix->Identity();
  matrix->SetElement(0, 3, tx);
  matrix->SetElement(1, 3, ty);
  matrix->SetElement(2, 3, tz);
  this->SetVtkMatrix(*matrix);
}

}
