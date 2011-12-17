/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-09-20 20:35:56 +0100 (Tue, 20 Sep 2011) $
 Revision          : $Revision: 7340 $
 Last modified by  : $Author: ad $
 
 Original author   : leung@drc.ion.ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#include "itkLogHelper.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSTAPLEImageFilter.h"

/**
 * STAPLE: The STAPLE algorithm is described in
 * S. Warfield, K. Zou, W. Wells, "Validation of image segmentation and expert quality with an 
 * expectation-maximization algorithm" in MICCAI 2002: Fifth International Conference on 
 * Medical Image Computing and Computer-Assisted Intervention, Springer-Verlag, Heidelberg, Germany, 2002, pp. 298-306.
 */
int main(int argc, char** argv)
{
  const unsigned int Dimension = 3;
  typedef short PixelType;
  
  if (argc < 4)
  {
    niftk::itkLogHelper::PrintCommandLineHeader(std::cout);
    std::cout << std::endl;    
    std::cout << "Usage: " << argv[0]
        << " outputFilename foregroundValue confidenceWeight inputFilenames1 inputFilenames2 ..." << std::endl;
    return EXIT_FAILURE; 
  }

  char* outputFilename = argv[1]; 
  PixelType foregroundValue = atoi(argv[2]); 
  double confidenceWeight = atof(argv[3]); 
  std::vector<char*> inputFilenames; 
  for (int argIndex = 4; argIndex < argc; argIndex++)
  {
    inputFilenames.push_back(argv[argIndex]); 
  }

  typedef itk::Image< double, Dimension > OutputImageType;
  typedef itk::Image< PixelType, Dimension > InputImageType;
  typedef itk::ImageFileReader<InputImageType> ImageFileReaderType;

  typedef itk::STAPLEImageFilter<InputImageType, OutputImageType> StapleFilterType;
  StapleFilterType::Pointer stapler = StapleFilterType::New(); 
  stapler->SetConfidenceWeight(confidenceWeight);
  stapler->SetForegroundValue(foregroundValue);

  try
  {
    for (unsigned int inputFileIndex = 0; inputFileIndex < inputFilenames.size(); inputFileIndex++)
    {
      ImageFileReaderType::Pointer reader = ImageFileReaderType::New();
      
      reader->SetFileName(inputFilenames[inputFileIndex]);
      reader->Update();
      stapler->SetInput(inputFileIndex, reader->GetOutput());
    }
  }
  catch (itk::ExceptionObject &e)
  {
    std::cout << "Error while setting input files:" << e << std::endl;
    return EXIT_FAILURE;
  }
  
  try
  {
    itk::ImageFileWriter<OutputImageType>::Pointer writer = itk::ImageFileWriter<OutputImageType>::New();
        
    writer->SetFileName(outputFilename);
    writer->SetInput(stapler->GetOutput());
    writer->Update();
  }
  catch( itk::ExceptionObject &e )
  {
    std::cout << "Error while performing STAPLE:" << e << std::endl;
    return EXIT_FAILURE;
  }
  
  for (unsigned int inputFileIndex = 0; inputFileIndex < inputFilenames.size(); inputFileIndex++)
  {
    std::cout << "Image " << inputFilenames[inputFileIndex] << " : "
      << stapler->GetSpecificity(inputFileIndex) << "," << stapler->GetSensitivity(inputFileIndex) << std::endl;
  }

  return EXIT_SUCCESS; 
}




