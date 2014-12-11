/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <mitkConfig.h>
#include "mitkNifTKItkImageFileReader.h"
#include <mitkImageCast.h>
#include <itkImageFileReader.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageIOFactory.h>
#include <itkImageIORegion.h>
#include <itkImageIOBase.h>
#include <itkNiftiImageIO3201.h>
#include <niftkEnvironmentHelper.h>

#include "itkDRCAnalyzeImageIO3160.h"

mitk::NifTKItkImageFileReader::NifTKItkImageFileReader()
{
}

mitk::NifTKItkImageFileReader::~NifTKItkImageFileReader()
{
}

void mitk::NifTKItkImageFileReader::GenerateData()
{

  // Basic plan here:
  // If its Analyze and NIFTK_DRC_ANALYZE is true
  //   call DRC specific functionality
  // If its Nifti
  //   call NifTK specific functionality
  // Else (or in case of any failure scenario).
  //   call base class functionality

  itk::DRCAnalyzeImageIO3160::Pointer drcAnalyzeIO = itk::DRCAnalyzeImageIO3160::New();
  itk::NiftiImageIO3201::Pointer niftiIO = itk::NiftiImageIO3201::New();

  bool useDRCAnalyze = niftk::BooleanEnvironmentVariableIsOn("NIFTK_DRC_ANALYZE");

  if (useDRCAnalyze && drcAnalyzeIO->CanReadFile(m_FileName.c_str()))
  {
    mitk::Image::Pointer image = this->GetOutput();
    try
    {
      itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(m_FileName.c_str(), itk::ImageIOFactory::ReadMode);
      bool result = false;

      if (imageIO.IsNotNull())
      {
        imageIO->SetFileName(m_FileName.c_str());
        imageIO->ReadImageInformation();

        itk::ImageIOBase::IOComponentType componentType = imageIO->GetComponentType();
        unsigned int numberOfDimensions = imageIO->GetNumberOfDimensions();

        MITK_INFO << "NifTKItkImageFileReader::GenerateData(), Reading Analyze data using NifTK DRC specific code, numberOfDimensions=" << numberOfDimensions << ", componentType=" << imageIO->GetComponentTypeAsString(componentType) << std::endl;

        switch(componentType)
        {
        case itk::ImageIOBase::UCHAR:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, unsigned char>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, unsigned char>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::CHAR:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, char>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, char>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::USHORT:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, unsigned short>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, unsigned short>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::SHORT:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, short>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, short>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::UINT:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, unsigned int>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, unsigned int>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::INT:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, int>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, int>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::ULONG:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, unsigned long>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, unsigned long>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::LONG:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, long>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, long>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::FLOAT:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, float>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, float>(image, m_FileName);
            }
          break;
        case itk::ImageIOBase::DOUBLE:
          if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, double>(image, m_FileName);
            }
          else
            {
              result = LoadImageUsingItk<3, double>(image, m_FileName);
            }
          break;
        default:
          std::cerr << "non standard pixel format" << std::endl;
        }
      }

      if (!result)
      {
        MITK_ERROR << "Reading file " << m_FileName << " failed, so trying default MITK behaviour." << std::endl;

        // Default to normal to avoid crashing.
        ItkImageFileReader::GenerateData();
      }
    }
    catch( itk::ExceptionObject& err )
    {
      std::string msg = std::string("Failed to Load image:") \
        + m_FileName \
        + std::string("\nEXCEPTION:") \
        + std::string(err.GetDescription()) \
        + std::string("\n\tat location:") \
        + std::string(err.GetLocation()) \
        + std::string("\n\tat file:") \
        + std::string(err.GetFile()) \
      ;
      MITK_ERROR << msg;

      // Default to normal to avoid crashing.
      ItkImageFileReader::GenerateData();
    }
  }
  else if (niftiIO->CanReadFile(m_FileName.c_str()))
  {
    mitk::Image::Pointer image = this->GetOutput();
    try
    {
      itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(m_FileName.c_str(), itk::ImageIOFactory::ReadMode);
      bool result = false;

      if (imageIO.IsNotNull())
      {
        imageIO->SetFileName(m_FileName.c_str());
        imageIO->ReadImageInformation();

        itk::ImageIOBase::IOComponentType componentType = imageIO->GetComponentType();
	itk::ImageIOBase::IOPixelType PixelType = imageIO->GetPixelType();

        unsigned int numberOfDimensions = imageIO->GetNumberOfDimensions();

        MITK_INFO << "NifTKItkImageFileReader::GenerateData(), Reading data using NifTK sform specific code, numberOfDimensions=" << numberOfDimensions << ", componentType=" << imageIO->GetComponentTypeAsString(componentType) << ", pixelType=" << imageIO->GetPixelTypeAsString(PixelType) << std::endl;

	switch ( PixelType )
	{
	case itk::ImageIOBase::SCALAR:
	{
	  switch(componentType)
	  {
	  case itk::ImageIOBase::UCHAR:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, unsigned char>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, unsigned char>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, unsigned char>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::CHAR:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, char>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, char>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, char>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::USHORT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, unsigned short>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, unsigned short>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, unsigned short>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::SHORT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, short>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, short>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, short>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::UINT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, unsigned int>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, unsigned int>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, unsigned int>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::INT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, int>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, int>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, int>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::ULONG:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, unsigned long>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, unsigned long>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, unsigned long>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::LONG:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, long>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, long>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, long>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::FLOAT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, float>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, float>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, float>(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::DOUBLE:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2, double>(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3, double>(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4, double>(image, m_FileName);
            }
	    break;
	  default:
	    std::cerr << "non standard pixel component type" << std::endl;
	  }

	  break;
	}

	case itk::ImageIOBase::RGB:
	{
	  switch(componentType)
	  {
	  case itk::ImageIOBase::UCHAR:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<unsigned char> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<unsigned char> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<unsigned char> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::CHAR:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<char> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<char> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<char> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::USHORT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<unsigned short> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<unsigned short> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<unsigned short> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::SHORT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<short> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<short> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<short> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::UINT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<unsigned int> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<unsigned int> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<unsigned int> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::INT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<int> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<int> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<int> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::ULONG:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<unsigned long> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<unsigned long> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<unsigned long> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::LONG:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<long> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<long> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<long> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::FLOAT:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<float> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<float> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<float> >(image, m_FileName);
            }
	    break;
	  case itk::ImageIOBase::DOUBLE:
	    if (numberOfDimensions == 2)
            {
              result = LoadImageUsingItk<2,  itk::RGBPixel<double> >(image, m_FileName);
            }
	    else if (numberOfDimensions == 3)
            {
              result = LoadImageUsingItk<3,  itk::RGBPixel<double> >(image, m_FileName);
            }
	    else
            {
              result = LoadImageUsingItk<4,  itk::RGBPixel<double> >(image, m_FileName);
            }
	    break;
	  default:
	    std::cerr << "non standard pixel component type" << std::endl;
	  }

	  break;
	}

	default:
	  std::cerr << "non standard pixel format" << std::endl;
	}

      }

      if (!result)
      {
        MITK_ERROR << "Reading file " << m_FileName << " failed so trying default MITK Nifti functionality" << std::endl;
        // Default to normal to avoid crashing.
        ItkImageFileReader::GenerateData();
      }
    }
    catch( itk::ExceptionObject& err )
    {
      std::string msg = std::string("Failed to Load image:") \
        + m_FileName \
        + std::string("\nEXCEPTION:") \
        + std::string(err.GetDescription()) \
        + std::string("\n\tat location:") \
        + std::string(err.GetLocation()) \
        + std::string("\n\tat file:") \
        + std::string(err.GetFile()) \
      ;
      MITK_ERROR << msg;

      // Default to normal to avoid crashing.
      ItkImageFileReader::GenerateData();
    }
  }
  else
  {
    MITK_INFO << "NifTKItkImageFileReader::GenerateData(), reverting to ItkImageFileReader::GenerateData()" << std::endl;
    ItkImageFileReader::GenerateData();
  }
}

template<unsigned int VImageDimension, typename TPixel>
bool
mitk::NifTKItkImageFileReader
::LoadImageUsingItk(mitk::Image::Pointer mitkImage, std::string fileName)
{
  MITK_INFO << "NifTKItkImageFileReader::LoadImageUsingItk loading filename=" << fileName << std::endl;

  bool result = false;

  typename itk::DRCAnalyzeImageIO3160::Pointer drcAnalyzeIO = itk::DRCAnalyzeImageIO3160::New();
  typename itk::NiftiImageIO3201::Pointer niftiIO = itk::NiftiImageIO3201::New();

  if (drcAnalyzeIO->CanReadFile(m_FileName.c_str())){
      try
      {
        typedef itk::Image<TPixel, VImageDimension> ImageType;

        typename itk::ImageFileReader<ImageType>::Pointer reader = itk::ImageFileReader<ImageType>::New();

        reader->SetImageIO(drcAnalyzeIO);
        reader->SetFileName(fileName);
        reader->Update();

        mitk::CastToMitkImage<ImageType>(reader->GetOutput(), mitkImage);

        MITK_INFO << "NifTKItkImageFileReader::LoadImageUsingItk finished" << std::endl;
        result = true;
      }
      catch( itk::ExceptionObject& err )
      {
        std::string msg = std::string("Failed to Load image:") \
          + m_FileName \
          + std::string("\nEXCEPTION:") \
          + std::string(err.GetDescription()) \
          + std::string("\n\tat location:") \
          + std::string(err.GetLocation()) \
          + std::string("\n\tat file:") \
          + std::string(err.GetFile()) \
        ;
        MITK_ERROR << msg;
      }
  }
  else if(niftiIO->CanReadFile(m_FileName.c_str())){
      try
      {
        typedef itk::Image<TPixel, VImageDimension> ImageType;

        typename itk::ImageFileReader<ImageType>::Pointer reader = itk::ImageFileReader<ImageType>::New();

        reader->SetImageIO(niftiIO);
        reader->SetFileName(fileName);
        reader->Update();

        mitk::CastToMitkImage<ImageType>(reader->GetOutput(), mitkImage);

        MITK_INFO << "NifTKItkImageFileReader::LoadImageUsingItk finished" << std::endl;
        result = true;
      }
      catch( itk::ExceptionObject& err )
      {
        std::string msg = std::string("Failed to Load image:") \
          + m_FileName \
          + std::string("\nEXCEPTION:") \
          + std::string(err.GetDescription()) \
          + std::string("\n\tat location:") \
          + std::string(err.GetLocation()) \
          + std::string("\n\tat file:") \
          + std::string(err.GetFile()) \
        ;
        MITK_ERROR << msg;
      }

  }

  return result;
}
