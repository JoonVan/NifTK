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
#include <itkCommandLineHelper.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNifTKImageIOFactory.h>
#include <itkZeroCrossingImageFilter.h>

/*!
 * \file niftkExtractZeroCrossing.cxx
 * \page niftkExtractZeroCrossing
 * \section niftkExtractZeroCrossingSummary Runs ITK ZeroCrossingImageFilter.
 */
void Usage(char *exec)
{
  niftk::LogHelper::PrintCommandLineHeader(std::cout);
  std::cout << "  " << std::endl;
  std::cout << "  Runs ITK ZeroCrossingImageFilter." << std::endl;
  std::cout << "  " << std::endl;
  std::cout << "  " << exec << " -i inputFileName -o outputFileName [options]" << std::endl;
  std::cout << "  " << std::endl;
  std::cout << "*** [mandatory] ***" << std::endl << std::endl;
  std::cout << "    -i <filename>        Input image " << std::endl;
  std::cout << "    -o <filename>        Output image" << std::endl << std::endl;
  std::cout << "*** [options]   ***" << std::endl << std::endl;
  std::cout << "    -f <float> [1]       Foreground value" << std::endl;
  std::cout << "    -b <float> [0]       Background value" << std::endl;
}

struct arguments
{
  std::string inputImage;
  std::string outputImage;
  float foregroundValue;
  float backgroundValue;
};

template <int Dimension, class PixelType>
int DoMain(arguments args)
{
  typedef typename itk::Image< PixelType, Dimension >     InputImageType;
  typedef typename itk::ImageFileReader< InputImageType > InputImageReaderType;
  typedef typename itk::ImageFileWriter< InputImageType > OutputImageWriterType;
  typedef typename itk::ZeroCrossingImageFilter<InputImageType, InputImageType> FilterType;

  typename InputImageReaderType::Pointer imageReader = InputImageReaderType::New();
  imageReader->SetFileName(args.inputImage);

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput(imageReader->GetOutput());
  filter->SetForegroundValue(args.foregroundValue);
  filter->SetBackgroundValue(args.backgroundValue);

  typename OutputImageWriterType::Pointer imageWriter = OutputImageWriterType::New();
  imageWriter->SetFileName(args.outputImage);
  imageWriter->SetInput(filter->GetOutput());

  try
  {
    imageWriter->Update();
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "Failed: " << err << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * \brief Takes image1 and image2 and adds them together
 */
int main(int argc, char** argv)
{
  itk::NifTKImageIOFactory::Initialize();

  // To pass around command line args
  struct arguments args;
  args.foregroundValue = 1;
  args.backgroundValue = 0;
  

  // Parse command line args
  for(int i=1; i < argc; i++){
    if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0 || strcmp(argv[i], "-HELP")==0 || strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--h")==0){
      Usage(argv[0]);
      return -1;
    }
    else if(strcmp(argv[i], "-i") == 0){
      args.inputImage=argv[++i];
      std::cout << "Set -i=" << args.inputImage << std::endl;
    }
    else if(strcmp(argv[i], "-o") == 0){
      args.outputImage=argv[++i];
      std::cout << "Set -o=" << args.outputImage << std::endl;
    }
    else if(strcmp(argv[i], "-f") == 0){
      args.foregroundValue=atof(argv[++i]);
      std::cout << "Set -f=" << niftk::ConvertToString(args.foregroundValue) << std::endl;
    }
    else if(strcmp(argv[i], "-b") == 0){
      args.backgroundValue=atof(argv[++i]);
      std::cout << "Set -b=" << niftk::ConvertToString(args.backgroundValue) << std::endl;
    }
    else {
      std::cerr << argv[0] << ":\tParameter " << argv[i] << " unknown." << std::endl;
      return -1;
    }
  }

  // Validate command line args
  if (args.inputImage.length() == 0 || args.outputImage.length() == 0)
    {
      Usage(argv[0]);
      return EXIT_FAILURE;
    }

  int dims = itk::PeekAtImageDimension(args.inputImage);
  if (dims != 2 && dims != 3)
    {
      std::cout << "Unsupported image dimension" << std::endl;
      return EXIT_FAILURE;
    }

  int result;

  if (dims == 2)
    {
      result = DoMain<2, float>(args);
    }
  else
    {
      result = DoMain<3, float>(args);
    }
  return result;
}
