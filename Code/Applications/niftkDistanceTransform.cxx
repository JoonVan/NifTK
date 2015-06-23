/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <itkLogHelper.h>
#include <niftkConversionUtils.h>
#include <itkCommandLineHelper.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include <itkInvertIntensityBetweenMaxAndMinImageFilter.h>
#include <itkAddImageFilter.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkNegateImageFilter.h>

/*!
 * \file niftkDistanceTransform.cxx
 * \page niftkDistanceTransform
 * \section niftkDistanceTransformSummary Runs the ITK DanielssonDistanceMapImageFilter, specifically for binary images, outputting the distance transform.
 */
void Usage(char *exec)
  {
    niftk::itkLogHelper::PrintCommandLineHeader(std::cout);
    std::cout << "  " << std::endl;
    std::cout << "  Runs the ITK DanielssonDistanceMapImageFilter, specifically for binary images, outputting the distance transform." << std::endl;
    std::cout << "  Assumes your input image, has 1 object, with background = 0, and foreground = 1." << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "  " << exec << " -i inputFileName -o outputFileName [options]" << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "*** [mandatory] ***" << std::endl << std::endl;
    std::cout << "    -i <filename>        Input image " << std::endl;
    std::cout << "    -o <filename>        Output image" << std::endl << std::endl;
    std::cout << "*** [options]   ***" << std::endl << std::endl;
    std::cout << "    -invert              If specified, will invert the input image prior to calculating distances. " << std::endl;
    std::cout << "    -internal            If specified, will calculate distances internal to the object. " << std::endl;
    std::cout << "                         Usefull if you have 1 object, and want to simulate a level set." << std::endl;
  }

struct arguments
{
  std::string inputImage;
  std::string outputImage;
  bool internal;
  bool invert;
};

template <int Dimension, class PixelType>
int DoMain(arguments args)
{
  typedef typename itk::Image< PixelType, Dimension >     InputImageType;

  typedef typename itk::ImageFileReader< InputImageType > InputImageReaderType;
  typedef typename itk::ImageFileWriter< InputImageType > OutputImageWriterType;
  typedef typename itk::InvertIntensityBetweenMaxAndMinImageFilter<InputImageType> InvertFilterType;
  typedef typename itk::DanielssonDistanceMapImageFilter<InputImageType, InputImageType> DistanceFilterType;

  try
  {
    typename InputImageReaderType::Pointer imageReader = InputImageReaderType::New();
    typename DistanceFilterType::Pointer distanceFilter = DistanceFilterType::New();

    typename OutputImageWriterType::Pointer imageWriter = OutputImageWriterType::New();

    imageReader->SetFileName(args.inputImage);
    imageReader->Update();

    typename InputImageType::Pointer inImage = imageReader->GetOutput();
    inImage->DisconnectPipeline();

    imageWriter->SetFileName(args.outputImage);

    if ( args.invert )
    {
      typename InvertFilterType::Pointer invertInputImageFilter = InvertFilterType::New();

      invertInputImageFilter->SetInput( inImage );
      invertInputImageFilter->Update();
      inImage = invertInputImageFilter->GetOutput();
      inImage->DisconnectPipeline();
    }

    distanceFilter->SetInput( inImage );
    distanceFilter->SetSquaredDistance(false);
    distanceFilter->SetInputIsBinary(true);
    distanceFilter->SetUseImageSpacing(true);
    distanceFilter->Update();

    if (args.internal)
    {
      typedef typename itk::BinaryCrossStructuringElement<PixelType, Dimension> StructuringElementType;

      StructuringElementType element;
      element.SetRadius(1);
      element.CreateStructuringElement();

      typedef typename itk::BinaryErodeImageFilter<InputImageType, InputImageType, StructuringElementType> ErodeImageFilterType;
      typename ErodeImageFilterType::Pointer erodeFilter = ErodeImageFilterType::New();

      erodeFilter->SetInput( inImage );
      erodeFilter->SetKernel(element);
      erodeFilter->SetErodeValue(1);
      erodeFilter->SetBackgroundValue(0);
      erodeFilter->SetBoundaryToForeground(false);
      erodeFilter->Update();

      typename InvertFilterType::Pointer invertInputImageFilter = InvertFilterType::New();

      invertInputImageFilter->SetInput(erodeFilter->GetOutput());

      typename DistanceFilterType::Pointer insideDistanceFilter = DistanceFilterType::New();

      insideDistanceFilter->SetInput(invertInputImageFilter->GetOutput());
      insideDistanceFilter->SetSquaredDistance(false);
      insideDistanceFilter->SetInputIsBinary(true);
      insideDistanceFilter->SetUseImageSpacing(true);

      typedef typename itk::NegateImageFilter<InputImageType, InputImageType> NegateFilterType;
      typename NegateFilterType::Pointer negateFilter = NegateFilterType::New();

      negateFilter->SetInput(insideDistanceFilter->GetOutput());

      typedef typename itk::AddImageFilter<InputImageType, InputImageType> AddFilterType;
      typename AddFilterType::Pointer addFilter = AddFilterType::New();

      addFilter->SetInput(0, distanceFilter->GetOutput());
      addFilter->SetInput(1, negateFilter->GetOutput());

      imageWriter->SetInput(addFilter->GetOutput());
      imageWriter->Update();

    }
    else
    {
      imageWriter->SetInput(distanceFilter->GetOutput());
      imageWriter->Update();
    }

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
  // To pass around command line args
  struct arguments args;
  args.invert = false;
  args.internal = false;
  

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
    else if(strcmp(argv[i], "-invert") == 0){
      args.invert=true;
      std::cout << "Set -invert=" << niftk::ConvertToString(args.invert) << std::endl;
    }
    else if(strcmp(argv[i], "-internal") == 0){
      args.internal=true;
      std::cout << "Set -internal=" << niftk::ConvertToString(args.internal) << std::endl;
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
