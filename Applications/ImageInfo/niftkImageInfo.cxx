/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <niftkLogHelper.h>
#include <niftkConversionUtils.h>
#include <itkConversionUtils.h>
#include <itkCommandLineHelper.h>
#include <itkImageFileReader.h>
#include <itkNifTKImageIOFactory.h>
#include <itkImageIOBase.h>
#include <itkImageIOFactory.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionConstIterator.h>
#include <itkSpatialOrientationAdapter.h>
#include <itkContinuousIndex.h>
#include <itkPoint.h>

/*!
 * \file niftkImageInfo.cxx
 * \page niftkImageInfo
 * \section niftkImageInfoSummary Prints image hader information.
 */
void Usage(char *exec)
  {
    niftk::LogHelper::PrintCommandLineHeader(std::cout);
    std::cout << "  " << std::endl;
    std::cout << "  Prints image hader information." << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "  " << exec << " -i <image> [options]" << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "*** [mandatory] ***" << std::endl << std::endl;
    std::cout << "    -i    <filename>        Input image " << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "*** [options]   ***" << std::endl << std::endl; 
    std::cout << "    -back <float> [0]       Background value, to be ignored in computing stats." << std::endl;   
  }

struct arguments
{
  std::string inputImage;
  float backgroundValue;
};

/** Can only be called for 3D due to limitation in itkSpatialOrientationAdaptor. */
template <int Dimension, class ScalarType>
int PrintOrientationInfo(arguments args)
{
  typedef typename itk::SpatialOrientationAdapter AdaptorType;
  typedef typename itk::Image< ScalarType, Dimension > InputImageType;
  typedef typename InputImageType::DirectionType DirectionType;

  itk::ImageIOBase::Pointer imageIO;
  InitialiseImageIO(args.inputImage, imageIO);

  DirectionType direction;

  for (unsigned int i = 0; i < Dimension; i++)
    {
      for (unsigned int j = 0; j < Dimension; j++)
        {
          direction[i][j] = imageIO->GetDirection(j)[i];
        }
    }

  std::cout << "Direction       :" << std::endl;
  std::cout << direction << std::endl;

  AdaptorType adaptor;
  std::cout << "Orientation     :" << itk::ConvertSpatialOrientationToString(adaptor.FromDirectionCosines(direction)) << std::endl;

  return EXIT_SUCCESS;
}

template <int Dimension, class ScalarType>
int PrintHeaderInfo(arguments args)
{
  typedef typename itk::Image< ScalarType, Dimension > InputImageType;
  typedef typename InputImageType::SizeType SizeType;
  typedef typename InputImageType::SpacingType SpacingType;
  typedef typename InputImageType::PointType OriginType;

  itk::ImageIOBase::Pointer imageIO;
  InitialiseImageIO(args.inputImage, imageIO);

  std::cout << "Dimensions      :" << imageIO->GetNumberOfDimensions() << std::endl;
  std::cout << "PixelType       :" << imageIO->GetPixelTypeAsString(imageIO->GetPixelType()) << std::endl;
  std::cout << "ComponentType   :" << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()) << std::endl;
  std::cout << "FileType        :" << imageIO->GetFileTypeAsString(imageIO->GetFileType()) << std::endl;
  std::cout << "ByteOrder       :" << imageIO->GetByteOrderAsString(imageIO->GetByteOrder()) << std::endl;
  std::cout << "SizeInPixels    :" << imageIO->GetImageSizeInPixels() << std::endl;
  std::cout << "SizeInBytes     :" << imageIO->GetImageSizeInBytes() << std::endl;
  std::cout << "SizeInComponents:" << imageIO->GetImageSizeInComponents() << std::endl;
  
  SizeType size;
  SpacingType spacing;
  SpacingType sizeInMM;
  OriginType origin;
  
  for (unsigned int i = 0; i < Dimension; i++)
    {
      size[i] = imageIO->GetDimensions(i);
      spacing[i] = imageIO->GetSpacing(i);
      sizeInMM[i] = size[i]*spacing[i];
      origin[i] = imageIO->GetOrigin(i);
    }
  
  std::cout << "Size            :" << size << std::endl;
  std::cout << "Spacing         :" << spacing << std::endl;
  std::cout << "Size x Spacing  :" << sizeInMM << std::endl;
  std::cout << "Origin          :" << origin << std::endl;

  return EXIT_SUCCESS;
}

template <int Dimension, class ScalarType>
int PrintImageInfo(arguments args)
{

  typedef typename itk::Image< ScalarType, Dimension > InputImageType;
  typedef typename itk::ImageFileReader< InputImageType > InputImageReaderType;
  typedef typename InputImageType::IndexType IndexType;
  typedef typename InputImageType::SpacingType SpacingType;
  typedef typename InputImageType::SizeType SizeType;
  typedef typename InputImageType::PointType OriginType;

  typename InputImageReaderType::Pointer imageReader= InputImageReaderType::New();
  imageReader->SetFileName(args.inputImage);

  try
  {
    imageReader->Update(); 
  }
  catch( itk::ExceptionObject & err ) 
  { 
    std::cerr << "Failed: " << err << std::endl; 
    return EXIT_FAILURE;
  }                

  IndexType minIndex;
  IndexType maxIndex;
  SpacingType spacing = imageReader->GetOutput()->GetSpacing();
  SizeType size = imageReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  OriginType origin = imageReader->GetOutput()->GetOrigin();

  double volumePerPixel = 1;
  for (int i = 0; i < Dimension; i++)
  {
    volumePerPixel *= spacing[i];
  }

  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::min();
  double mean = 0;
  double sum = 0;
  double volume = 0;

  unsigned long int counter = 0;
  
  IndexType lowerLeft; 
  IndexType upperRight; 
  for (int i = 0; i < Dimension; i++)
  {
    lowerLeft[i] = std::numeric_limits<typename IndexType::IndexValueType>::max(); 
    upperRight[i] = 0; 
  }
  
  ScalarType value;
  itk::ImageRegionConstIteratorWithIndex<InputImageType> imageIterator(imageReader->GetOutput(), imageReader->GetOutput()->GetLargestPossibleRegion());
  for (imageIterator.GoToBegin(); !imageIterator.IsAtEnd(); ++imageIterator)
  {
    value = imageIterator.Get();
    if (value != args.backgroundValue)
    {
      if (value > max) 
      {
	max = value;
	maxIndex = imageIterator.GetIndex();
      }
      else if (value < min)
      {
	min = value;
	minIndex = imageIterator.GetIndex();
      }
      sum += value;
      volume += value*volumePerPixel;
      counter++;
          
      IndexType index = imageIterator.GetIndex(); 
      for (int i = 0; i < Dimension; i++)
      {
	if (index[i] > upperRight[i])
	{
	  upperRight[i] = index[i]; 
	}
	if (index[i] < lowerLeft[i])
	{
	  lowerLeft[i] = index[i]; 
	}
      }
    }
  }
  mean = sum / (double)counter;
  
  std::cout << "Min                     :" << min << ", taken from " << minIndex << std::endl;
  std::cout << "Max                     :" << max << ", taken from " << maxIndex << std::endl;
  std::cout << "Count                   :" << counter << std::endl;
  std::cout << "Vol                     :" << counter*volumePerPixel << std::endl;
  std::cout << "Mean                    :" << mean << std::endl;
  std::cout << "Sum                     :" << sum << std::endl;
  std::cout << "(Vol per pix)*intensity :" << volume << std::endl;
  
  itk::ContinuousIndex<double, Dimension> voxelCoordinate;
  itk::Point<double, Dimension> millimetreCoordinateForITK;
  itk::Point<double, Dimension> millimetreCoordinateForVTK;

  for (int i = 0; i < Dimension; i++)
  {
    voxelCoordinate[i] = (size[i] - 1)/2.0;
    millimetreCoordinateForVTK[i] = origin[i] + voxelCoordinate[i]*spacing[i];
  }
  imageReader->GetOutput()->TransformContinuousIndexToPhysicalPoint(voxelCoordinate, millimetreCoordinateForITK);

  std::cout << "Image centre            :" << voxelCoordinate << " (vox), " << millimetreCoordinateForITK << " (mm) " << millimetreCoordinateForVTK << " (VTK) " << std::endl;

  for (int i = 0; i < Dimension; i++)
  {
    voxelCoordinate[i] = 0;
  }
  imageReader->GetOutput()->TransformContinuousIndexToPhysicalPoint(voxelCoordinate, millimetreCoordinateForITK);
  std::cout << "Image first voxel       :" << voxelCoordinate << " (vox), " << millimetreCoordinateForITK << std::endl;

  for (int i = 0; i < Dimension; i++)
  {
    voxelCoordinate[i] = size[i] - 1;
  }
  imageReader->GetOutput()->TransformContinuousIndexToPhysicalPoint(voxelCoordinate, millimetreCoordinateForITK);
  std::cout << "Image last voxel        :" << voxelCoordinate << " (vox), " << millimetreCoordinateForITK << std::endl;
  
  std::cout << "Bounding box: " << lowerLeft << " (vox)," << upperRight << " (vox)." << std::endl; 

  return EXIT_SUCCESS;
}

/**
 * \brief To print out image information.
 */
int main(int argc, char** argv)
{
  itk::NifTKImageIOFactory::Initialize();

  // To pass around command line args
  struct arguments args;
  args.backgroundValue = 0;
  

  for(int i=1; i < argc; i++){
    if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0 || strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--h")==0){
      Usage(argv[0]);
      return -1;
    }
    else if(strcmp(argv[i], "-i") == 0){
      args.inputImage=argv[++i];
      std::cout << "Set -i=" << args.inputImage << std::endl;
    }
    else if(strcmp(argv[i], "-back") == 0){
      args.backgroundValue=atof(argv[++i]);
      std::cout << "Set -back=" << niftk::ConvertToString(args.backgroundValue) << std::endl;
    }
    else{
      std::cerr << argv[0] << ":\tParameter " << argv[i] << " unknown." << std::endl;
      return -1;
    }    
  }

  // Validate command line args
  if (args.inputImage.length() == 0)
    {
      Usage(argv[0]);
      return EXIT_FAILURE;
    }

  int dims = itk::PeekAtImageDimension(args.inputImage);
  if (dims != 2 && dims != 3 && dims != 4)
    {
      std::cout << "Unsupported image dimension" << std::endl;
      return EXIT_FAILURE;
    }
  
  int result = 0;

  switch (itk::PeekAtComponentType(args.inputImage))
    {
    case itk::ImageIOBase::UCHAR:
    {
      if (dims == 2)
        {
	  result = PrintHeaderInfo<2, unsigned char>(args);

	  if ( itk::PeekAtPixelType(args.inputImage) == itk::ImageIOBase::SCALAR )
	  {
	    result = PrintImageInfo<2, unsigned char>(args);
	  }
	  else
	  {
	    std::cout << std::endl
		      << "WARNING: Image intensity stats not implemented for non-scalar images" 
		      << std::endl;
	  }
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, unsigned char>(args);
          result = PrintOrientationInfo<3, unsigned char>(args);
          result = PrintImageInfo<3, unsigned char>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, unsigned char>(args);
          result = PrintImageInfo<4, unsigned char>(args);
        }
      break;
    }
    case itk::ImageIOBase::CHAR:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, char>(args);
          result = PrintImageInfo<2, char>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, char>(args);
          result = PrintOrientationInfo<3, char>(args);
          result = PrintImageInfo<3, char>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, char>(args);
          result = PrintImageInfo<4, char>(args);
        }
      break;
    }
    case itk::ImageIOBase::USHORT:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, unsigned short>(args);
          result = PrintImageInfo<2, unsigned short>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, unsigned short>(args);
          result = PrintOrientationInfo<3, unsigned short>(args);
          result = PrintImageInfo<3, unsigned short>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, unsigned short>(args);
          result = PrintImageInfo<4, unsigned short>(args);
        }
      break;
    }
    case itk::ImageIOBase::SHORT:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, short>(args);
          result = PrintImageInfo<2, short>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, short>(args);
          result = PrintOrientationInfo<3, short>(args);
          result = PrintImageInfo<3, short>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, short>(args);
          result = PrintImageInfo<4, short>(args);
        }
      break;
    }
    case itk::ImageIOBase::UINT:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, unsigned int>(args);
          result = PrintImageInfo<2, unsigned int>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, unsigned int>(args);
          result = PrintOrientationInfo<3, unsigned int>(args);
          result = PrintImageInfo<3, unsigned int>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, unsigned int>(args);
          result = PrintImageInfo<4, unsigned int>(args);
        }
      break;
    }
    case itk::ImageIOBase::INT:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, int>(args);
          result = PrintImageInfo<2, int>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, int>(args);
          result = PrintOrientationInfo<3, int>(args);
          result = PrintImageInfo<3, int>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, int>(args);
          result = PrintImageInfo<4, int>(args);
        }
      break;
    }
    case itk::ImageIOBase::ULONG:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, unsigned long>(args);
          result = PrintImageInfo<2, unsigned long>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, unsigned long>(args);
          result = PrintOrientationInfo<3, unsigned long>(args);
          result = PrintImageInfo<3, unsigned long>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, unsigned long>(args);
          result = PrintImageInfo<4, unsigned long>(args);
        }
      break;
    }
    case itk::ImageIOBase::LONG:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, long>(args);
          result = PrintImageInfo<2, long>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, long>(args);
          result = PrintOrientationInfo<3, long>(args);
          result = PrintImageInfo<3, long>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, long>(args);
          result = PrintImageInfo<4, long>(args);
        }
      break;
    }
    case itk::ImageIOBase::FLOAT:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, float>(args);
          result = PrintImageInfo<2, float>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, float>(args);
          result = PrintOrientationInfo<3, float>(args);
          result = PrintImageInfo<3, float>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, float>(args);
          result = PrintImageInfo<4, float>(args);
        }
      break;
    }
    case itk::ImageIOBase::DOUBLE:
    {
      if (dims == 2)
        {
          result = PrintHeaderInfo<2, double>(args);
          result = PrintImageInfo<2, double>(args);
        }
      else if (dims == 3)
        {
          result = PrintHeaderInfo<3, double>(args);
          result = PrintOrientationInfo<3, double>(args);
          result = PrintImageInfo<3, double>(args);
        }
      else if (dims == 4)
        {
          result = PrintHeaderInfo<4, double>(args);
          result = PrintImageInfo<4, double>(args);
        }
      break;
    }
    default:
      std::cerr << "non standard pixel format" << std::endl;
      return EXIT_FAILURE;
    }
  return result;
  
}
