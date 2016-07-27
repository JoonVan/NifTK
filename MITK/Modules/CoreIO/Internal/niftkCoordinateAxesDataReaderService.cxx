/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkCoordinateAxesDataReaderService.h"

#include <iostream>
#include <fstream>
#include <locale>

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

#include <niftkCoordinateAxesData.h>
#include <niftkFileIOUtils.h>

#include "niftkCoreIOMimeTypes.h"

namespace niftk
{

//-----------------------------------------------------------------------------
CoordinateAxesDataReaderService::CoordinateAxesDataReaderService()
: AbstractFileReader(mitk::CustomMimeType(niftk::CoreIOMimeTypes::TRANSFORM4X4_MIMETYPE_NAME()),
                     "NifTK Coordinate Axes Reader")
{
  RegisterService();
}


//-----------------------------------------------------------------------------
CoordinateAxesDataReaderService::CoordinateAxesDataReaderService(const CoordinateAxesDataReaderService& other)
: mitk::AbstractFileReader(other)
{
}


//-----------------------------------------------------------------------------
CoordinateAxesDataReaderService::~CoordinateAxesDataReaderService()
{
}


//-----------------------------------------------------------------------------
CoordinateAxesDataReaderService* CoordinateAxesDataReaderService::Clone() const
{
  return new CoordinateAxesDataReaderService(*this);
}


//-----------------------------------------------------------------------------
std::vector< itk::SmartPointer<mitk::BaseData> > CoordinateAxesDataReaderService::Read()
{
  std::locale::global(std::locale("C"));
  std::vector< itk::SmartPointer<mitk::BaseData> > result;

  std::string fileName = this->GetInputLocation();
  MITK_DEBUG << "Reading .4x4 transform from:" << fileName << std::endl;

  vtkSmartPointer<vtkMatrix4x4> matrix = LoadVtkMatrix4x4FromFile(fileName);
  CoordinateAxesData::Pointer transform = CoordinateAxesData::New();
  transform->SetVtkMatrix(*matrix);

  result.push_back(itk::SmartPointer<mitk::BaseData>(transform));
  return result;
}

}
