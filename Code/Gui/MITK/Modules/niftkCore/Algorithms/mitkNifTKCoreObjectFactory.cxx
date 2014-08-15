/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkNifTKCoreObjectFactory.h"

#include <itkObjectFactory.h>
#include <mitkItkImageFileIOFactory.h>
#include <mitkProperties.h>
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkCoordinateAxesData.h>
#include <mitkCoordinateAxesDataWriter.h>
#include <mitkCoordinateAxesDataReaderFactory.h>
#include <mitkCoordinateAxesDataWriterFactory.h>
#include <mitkCoordinateAxesVtkMapper3D.h>
#include <mitkVolumeDataVtkMapper3D.h>
#include <mitkImageVtkMapper2D.h>
#include <itkPNMImageIOFactory.h>
#include <mitkNifTKItkImageFileIOFactory.h>
#include <mitkFastPointSetVtkMapper3D.h>
#include <mitkPointSetVtkMapper3D.h>

//-----------------------------------------------------------------------------
mitk::NifTKCoreObjectFactory::NifTKCoreObjectFactory()
:CoreObjectFactoryBase()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "NifTKCoreObjectFactory c'tor" << std::endl;

    // At this point in this constructor, the main MITK CoreObjectFactory has been created,
    // so MITKs file reader for ITK images will already be available. So, now we remove it.
    std::list<itk::ObjectFactoryBase*> listOfObjectFactories = itk::ObjectFactoryBase::GetRegisteredFactories();
    std::list<itk::ObjectFactoryBase*>::iterator iter;
    mitk::ItkImageFileIOFactory::Pointer itkIOFactory = NULL;
    for (iter = listOfObjectFactories.begin(); iter != listOfObjectFactories.end(); iter++)
    {
      itkIOFactory = dynamic_cast<mitk::ItkImageFileIOFactory*>(*iter);
      if (itkIOFactory.IsNotNull())
      {
        break;
      }
    }
    itk::ObjectFactoryBase::UnRegisterFactory(itkIOFactory.GetPointer());

    // Load our specific factory, which will be used to load all ITK images, just like the MITK one,
    // but then in addition, will load DRC Analyze files differently.
    mitk::NifTKItkImageFileIOFactory::RegisterOneFactory();

    // Register our extra factories.
    itk::PNMImageIOFactory::RegisterOneFactory();
    mitk::CoordinateAxesDataReaderFactory::RegisterOneFactory();
    mitk::CoordinateAxesDataWriterFactory::RegisterOneFactory();
    m_FileWriters.push_back(mitk::CoordinateAxesDataWriter::New().GetPointer());

    // Carry on as per normal.
    CreateFileExtensionsMap();
    alreadyDone = true;
    MITK_DEBUG << "NifTKCoreObjectFactory c'tor finished" << std::endl;
  }

}


//-----------------------------------------------------------------------------
mitk::Mapper::Pointer mitk::NifTKCoreObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = NULL;
  mitk::BaseData *data = node->GetData();

  if ( id == mitk::BaseRenderer::Standard3D )
  {
    if (dynamic_cast<PointSet*>(data) != NULL )
    {
      mitk::PointSet* pointSet = dynamic_cast<PointSet*>(data);
      if (pointSet->GetSize() > 1000)
      {
        newMapper = mitk::FastPointSetVtkMapper3D::New();
      }
      else
      {
        newMapper = mitk::PointSetVtkMapper3D::New();
      }
      newMapper->SetDataNode(node);
    }
    else if (dynamic_cast<CoordinateAxesData*>(data) != NULL)
    {
      newMapper = mitk::CoordinateAxesVtkMapper3D::New();
      newMapper->SetDataNode(node);
    }
  }
  return newMapper;
}


//-----------------------------------------------------------------------------
void mitk::NifTKCoreObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{

  if(node == NULL)
  {
    return;
  }

  mitk::CoordinateAxesData::Pointer coordinateAxesData = dynamic_cast<mitk::CoordinateAxesData*>(node->GetData());
  if (coordinateAxesData.IsNotNull())
  {
    mitk::CoordinateAxesVtkMapper3D::SetDefaultProperties(node);
  }
}


//-----------------------------------------------------------------------------
void mitk::NifTKCoreObjectFactory::CreateFileExtensionsMap()
{
  MITK_DEBUG << "Registering additional file extensions." << std::endl;

  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pgm", "Portable Gray Map"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.ppm", "Portable Pixel Map"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pbm", "Portable Binary Map"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.pnm", "Portable aNy Map"));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>(mitk::CoordinateAxesData::FILE_EXTENSION_WITH_ASTERISK, mitk::CoordinateAxesData::FILE_DIALOG_NAME));

  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pgm", "Portable Gray Map"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.ppm", "Portable Pixel Map"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pbm", "Portable Binary Map"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>("*.pnm", "Portable aNy Map"));
  m_SaveFileExtensionsMap.insert(std::pair<std::string, std::string>(mitk::CoordinateAxesData::FILE_EXTENSION_WITH_ASTERISK, mitk::CoordinateAxesData::FILE_DIALOG_NAME));
}


//-----------------------------------------------------------------------------
mitk::NifTKCoreObjectFactory::MultimapType mitk::NifTKCoreObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}


//-----------------------------------------------------------------------------
mitk::NifTKCoreObjectFactory::MultimapType mitk::NifTKCoreObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}


//-----------------------------------------------------------------------------
const char* mitk::NifTKCoreObjectFactory::GetFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}


//-----------------------------------------------------------------------------
const char* mitk::NifTKCoreObjectFactory::GetSaveFileExtensions()
{
  std::string fileExtension;
  this->CreateFileExtensions(m_SaveFileExtensionsMap, fileExtension);
  return fileExtension.c_str();
}


//-----------------------------------------------------------------------------
struct RegisterNifTKCoreObjectFactory{
  RegisterNifTKCoreObjectFactory()
    : m_Factory( mitk::NifTKCoreObjectFactory::New() )
  {
    MITK_DEBUG << "Registering NifTKCoreObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterNifTKCoreObjectFactory()
  {
    MITK_DEBUG << "Un-Registering NifTKCoreObjectFactory..." << std::endl;
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::NifTKCoreObjectFactory::Pointer m_Factory;
};


//-----------------------------------------------------------------------------
static RegisterNifTKCoreObjectFactory registerNifTKCoreObjectFactory;

