/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.
 
 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-11-24 17:44:42 +0000 (Thu, 24 Nov 2011) $
 Revision          : $Revision: 7864 $
 Last modified by  : $Author: kkl $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#include "itkLogHelper.h"
#include "ConversionUtils.h"
#include "itkCommandLineHelper.h"
#include "itkEulerAffineTransform.h"

void Usage(char *exec)
  {
    niftk::itkLogHelper::PrintCommandLineHeader(std::cout);
    std::cout << "  " << std::endl;
    std::cout << "  Inverts an affine transform" << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "  " << exec << " -i inputFileName -o outputFileName [options]" << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "*** [mandatory] ***" << std::endl << std::endl;
    std::cout << "    -i    <filename>        Input matrix, a plain text file, 4 rows, 4 columns." << std::endl;
    std::cout << "    -o    <filename>        Output matrix, in same format as input." << std::endl << std::endl;      
    std::cout << "*** [options]   ***" << std::endl << std::endl;   
  }

struct arguments
{
  std::string inputImage;
  std::string outputImage;  
};

/**
 * \brief Calculates inverse of an affine transform.
 */
int main(int argc, char** argv)
{
  // To pass around command line args
  struct arguments args;
  

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

  // Short and sweet.... well... not as short and sweet as Matlab!
  
  typedef itk::EulerAffineTransform<double, 3, 3> MatrixType;
  
  MatrixType::Pointer inputMatrix = MatrixType::New();
  inputMatrix->LoadFullAffineMatrix(args.inputImage);
  
  MatrixType::Pointer outputMatrix = MatrixType::New();
  inputMatrix->GetInverse(outputMatrix);
  outputMatrix->SaveFullAffineMatrix(args.outputImage);
  
}
