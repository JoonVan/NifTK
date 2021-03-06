/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkCoreIOObjectFactory.h"

#include <itkObjectFactory.h>

#include <mitkAbstractFileIO.h>
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkImageVtkMapper2D.h>
#include <mitkIOMimeTypes.h>
#include <mitkItkImageIO.h>
#include <mitkPointSet.h>
#include <mitkPointSetVtkMapper3D.h>
#include <mitkProperties.h>

#include <niftkEnvironmentHelper.h>

#include <itkDRCAnalyzeImageIO.h>
#include <itkINRImageIO.h>
#include <itkNiftiImageIO3201.h>

#include <niftkCoordinateAxesVtkMapper3D.h>
#include <niftkFastPointSetVtkMapper3D.h>

#include "niftkCoreIOMimeTypes.h"

namespace niftk
{

//-----------------------------------------------------------------------------
CoreIOObjectFactory::CoreIOObjectFactory()
:CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "niftk::CoreIOObjectFactory c'tor" << std::endl;

    /// Important note:
    ///
    /// Registering ITK image IOs to mitk::FileReaderRegistry here must follow the same
    /// logic as registering them to the ITK object factories in itk::NifTKImageIOFactory
    /// in the niftkITK library.

    /// TODO
    /// ITK readers and writers should be registered from itk::NifTKImageIOFactory.

    const char* useDRCAnalyzeCStr = std::getenv("NIFTK_DRC_ANALYZE");
    std::string useDRCAnalyze = useDRCAnalyzeCStr ? std::string(useDRCAnalyzeCStr) : std::string();

    if (useDRCAnalyze == "ON"
        || useDRCAnalyze == "PREFERRED"
        || useDRCAnalyze == "OPTIONAL"
        || useDRCAnalyze.empty())
    {
      int rank = useDRCAnalyze == "PREFERRED";

      itk::DRCAnalyzeImageIO::Pointer itkDrcAnalyzeIO = itk::DRCAnalyzeImageIO::New();
      mitk::ItkImageIO* drcAnalyzeIO =
          new mitk::ItkImageIO(niftk::CoreIOMimeTypes::ANALYZE_MIMETYPE(),
                               itkDrcAnalyzeIO.GetPointer(),
                               rank);
      m_FileIOs.push_back(drcAnalyzeIO);
    }

    if (useDRCAnalyze == "OFF"
        || useDRCAnalyze == "PREFERRED"
        || useDRCAnalyze == "OPTIONAL"
        || useDRCAnalyze.empty())
    {
      int rank = useDRCAnalyze == "OPTIONAL" || useDRCAnalyze.empty();

      itk::NiftiImageIO3201::Pointer itkNiftiIO = itk::NiftiImageIO3201::New();
      mitk::ItkImageIO* niftiIO =
          new mitk::ItkImageIO(niftk::CoreIOMimeTypes::ANALYZE_MIMETYPE(),
                               itkNiftiIO.GetPointer(),
                               rank);
      m_FileIOs.push_back(niftiIO);
    }

    itk::NiftiImageIO3201::Pointer itkNiftiIO = itk::NiftiImageIO3201::New();
    mitk::ItkImageIO* niftiIO =
        new mitk::ItkImageIO(niftk::CoreIOMimeTypes::NIFTI_MIMETYPE(),
                             itkNiftiIO.GetPointer(),
                             0);
    m_FileIOs.push_back(niftiIO);

    itk::INRImageIO::Pointer itkINRImageIO = itk::INRImageIO::New();
    mitk::ItkImageIO* inrImageIO =
        new mitk::ItkImageIO(niftk::CoreIOMimeTypes::INRIA_MIMETYPE(),
                             itkINRImageIO.GetPointer(),
                             0);
    m_FileIOs.push_back(inrImageIO);

    CreateFileExtensionsMap();
    alreadyDone = true;

    MITK_DEBUG << "niftk::CoreIOObjectFactory c'tor" << std::endl;
  }

}


//-----------------------------------------------------------------------------
CoreIOObjectFactory::~CoreIOObjectFactory()
{
  /// TODO
  /// ITK readers and writers should be unregistered from itk::NifTKImageIOFactory.

  for(std::vector<mitk::AbstractFileIO*>::iterator iter = m_FileIOs.begin(),
      endIter = m_FileIOs.end();
      iter != endIter;
      ++iter)
  {
    delete *iter;
  }
}


//-----------------------------------------------------------------------------
mitk::Mapper::Pointer CoreIOObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId slotId)
{
  return NULL;
}


//-----------------------------------------------------------------------------
void CoreIOObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{

}


//-----------------------------------------------------------------------------
void CoreIOObjectFactory::CreateFileExtensionsMap()
{
  MITK_DEBUG << "Registering additional file extensions." << std::endl;
}


//-----------------------------------------------------------------------------
CoreIOObjectFactory::MultimapType CoreIOObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}


//-----------------------------------------------------------------------------
CoreIOObjectFactory::MultimapType CoreIOObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}


//-----------------------------------------------------------------------------
const char* CoreIOObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}


//-----------------------------------------------------------------------------
const char* CoreIOObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}


//-----------------------------------------------------------------------------
struct RegisterNifTKCoreIOObjectFactory{
  RegisterNifTKCoreIOObjectFactory()
    : m_Factory( niftk::CoreIOObjectFactory::New() )
  {
    MITK_DEBUG << "Registering niftk::CoreIOObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterNifTKCoreIOObjectFactory()
  {
    MITK_DEBUG << "Un-Registering niftk::CoreIOObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  niftk::CoreIOObjectFactory::Pointer m_Factory;
};


//-----------------------------------------------------------------------------
static RegisterNifTKCoreIOObjectFactory registerNifTKCoreIOObjectFactory;

}
