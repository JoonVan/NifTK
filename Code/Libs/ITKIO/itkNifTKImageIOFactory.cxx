/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "itkNifTKImageIOFactory.h"
#include <itkCreateObjectFunction.h>
#include <itkPNGImageIOFactory.h>
#include <itkVTKImageIOFactory.h>
#include <itkTxtTransformIOFactory.h>
#include "itkDRCAnalyzeImageIO.h"
#include "itkNiftiImageIO3201.h"
#include "itkINRImageIO.h"
#include <itkVersion.h>

#include <itkObjectFactory.h>
#include <niftkEnvironmentHelper.h>

namespace itk
{

NifTKImageIOFactory::NifTKImageIOFactory()
{
  /// Important note:
  ///
  /// Registering ITK image IOs to the ITK object factories here must follow the same
  /// logic as registering them to mitk::FileReaderRegistry in mitk::NifTKCoreObjectFactory
  /// in the niftkCore module.

  std::string useDRCAnalyze = niftk::GetEnvironmentVariable("NIFTK_DRC_ANALYZE");

  this->RegisterOverride("itkImageIOBase", "itkNiftiImageIO3201", "Nifti Image IO 3201", 1,
    itk::CreateObjectFunction<NiftiImageIO3201>::New());

  if (useDRCAnalyze == "ON"
      || useDRCAnalyze == "1"
      || useDRCAnalyze == "TRUE"
      || useDRCAnalyze == "YES"
      || useDRCAnalyze == "PREFERRED")
  {
    this->RegisterOverride("itkImageIOBase", "itkDRCAnalyzeImageIO", "DRC Analyze Image IO", 1,
       itk::CreateObjectFunction<DRCAnalyzeImageIO>::New());
  }

  this->RegisterOverride("itkImageIOBase", "itkINRImageIO", "INR Image IO", 1,
    itk::CreateObjectFunction<INRImageIO>::New());
}


NifTKImageIOFactory::~NifTKImageIOFactory()
{
}


const char*
NifTKImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}


const char*
NifTKImageIOFactory::GetDescription() const
{
  return "NifTK ImageIO Factory. Supports DRC Analyze, NIfTI (reads Analyze) and INR image formats.";
}


void
NifTKImageIOFactory::Initialize()
{
  /// Dumb function. Calling this function ensures that the static instance of RegisterImageIOFactories
  /// is created and the ITK image IOs are actually registered.
}

}


//-----------------------------------------------------------------------------
class RegisterImageIOFactories
{

  RegisterImageIOFactories()
  : m_NifTKImageIOFactory( itk::NifTKImageIOFactory::New() )
  , m_PNGImageIOFactory( itk::PNGImageIOFactory::New() )
  , m_VTKImageIOFactory( itk::VTKImageIOFactory::New() )
  , m_TxtTransformIOFactory( itk::TxtTransformIOFactory::New() )
  {
    itk::ObjectFactoryBase::RegisterFactory(m_NifTKImageIOFactory, itk::ObjectFactoryBase::INSERT_AT_FRONT);
    itk::ObjectFactoryBase::RegisterFactory(m_PNGImageIOFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_VTKImageIOFactory);
    itk::ObjectFactoryBase::RegisterFactory(m_TxtTransformIOFactory);
  }

  ~RegisterImageIOFactories()
  {
    itk::ObjectFactoryBase::UnRegisterFactory(m_TxtTransformIOFactory);
    itk::ObjectFactoryBase::UnRegisterFactory(m_VTKImageIOFactory);
    itk::ObjectFactoryBase::UnRegisterFactory(m_PNGImageIOFactory);
    itk::ObjectFactoryBase::UnRegisterFactory(m_NifTKImageIOFactory);
  }

  itk::NifTKImageIOFactory::Pointer m_NifTKImageIOFactory;
  itk::PNGImageIOFactory::Pointer m_PNGImageIOFactory;
  itk::VTKImageIOFactory::Pointer m_VTKImageIOFactory;
  itk::TxtTransformIOFactory::Pointer m_TxtTransformIOFactory;

  static RegisterImageIOFactories s_RegisterImageIOFactories;

};


//-----------------------------------------------------------------------------
RegisterImageIOFactories RegisterImageIOFactories::s_RegisterImageIOFactories;
