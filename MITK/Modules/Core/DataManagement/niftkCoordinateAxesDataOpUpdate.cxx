/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkCoordinateAxesDataOpUpdate.h"

namespace niftk
{

//-----------------------------------------------------------------------------
CoordinateAxesDataOpUpdate::CoordinateAxesDataOpUpdate(
    mitk::OperationType type,
    const vtkMatrix4x4& matrix,
    const std::string &nodeName
    )
: mitk::Operation(type)
, m_Matrix(NULL)
, m_NodeName(nodeName)
{
  m_Matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  m_Matrix->DeepCopy(&matrix);
}


//-----------------------------------------------------------------------------
CoordinateAxesDataOpUpdate::~CoordinateAxesDataOpUpdate()
{
}


//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4> CoordinateAxesDataOpUpdate::GetMatrix() const
{
  vtkSmartPointer<vtkMatrix4x4> result = vtkSmartPointer<vtkMatrix4x4>::New();
  result->DeepCopy(this->m_Matrix);
  return result;
}


//-----------------------------------------------------------------------------
std::string CoordinateAxesDataOpUpdate::GetNodeName() const
{
  return m_NodeName;
}

}
