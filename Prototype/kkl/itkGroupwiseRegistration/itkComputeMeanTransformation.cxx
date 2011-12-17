/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.
 
 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2010-10-29 13:30:36 +0100 (Fri, 29 Oct 2010) $
 Revision          : $Revision: 4089 $
 Last modified by  : $Author: kkl $

 Original author   : leung@drc.ion.ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#include "ConversionUtils.h"
#include "itkImage.h"
#include "itkImageRegistrationFactory.h"
#include "itkImageRegistrationFilter.h"
#include "itkTransformFileWriter.h" 

/**
 * \brief Compute mean transformaitons. 
 */
int main(int argc, char** argv)
{
  const unsigned int Dimension = 3;
  typedef short PixelType;
  typedef itk::Image< PixelType, Dimension >  InputImageType; 
  typedef itk::ImageRegistrationFactory<InputImageType, Dimension, double> FactoryType;
  typedef itk::EulerAffineTransform<double, Dimension, Dimension> AffineTransformType; 
  typedef itk::MatrixLinearCombinationFunctions<AffineTransformType::FullAffineMatrixType::InternalMatrixType> MatrixLinearCombinationFunctionsType; 
  typedef itk::TransformFileWriter TransformFileWriterType;
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(1); 
  
  int startingArgIndex = 3; 
  const char* outputName = argv[1]; 
  double tolerance = atof(argv[2]);   // suggest 1e-7. 
  double numberOfTransformations = static_cast<double>(argc-startingArgIndex)/2; 
  std::cout << "numberOfTransformations=" << numberOfTransformations << std::endl; 
  
  try
  {
    FactoryType::Pointer factory = FactoryType::New();
    AffineTransformType::Pointer averageTransform = AffineTransformType::New(); 
    AffineTransformType::FullAffineMatrixType averageMatrix; 
    std::cout << "averageMatrix=" << averageMatrix << std::endl; 
    
    for (int i = 0; i < numberOfTransformations; i++)
    {
      std::cout << "----------------------------------------" << std::endl; 
      std::cout << "Reading tranform: " << i << " " << argv[startingArgIndex+2*i] << std::endl; 
      AffineTransformType* currentTransform = dynamic_cast<AffineTransformType*>(factory->CreateTransform(argv[startingArgIndex+2*i]).GetPointer());
      double weight = atof(argv[startingArgIndex+2*i+1]); 
      std::cout << "weight=" << weight << std::endl; 
      AffineTransformType::FullAffineMatrixType currentMatrix = currentTransform->GetFullAffineMatrix();  
      currentMatrix = MatrixLinearCombinationFunctionsType::ComputeMatrixLogarithm(currentMatrix.GetVnlMatrix(), tolerance); 
      currentMatrix *= weight; 
      currentMatrix /= numberOfTransformations; 
      averageMatrix += currentMatrix; 
      std::cout << "averageMatrix=" << averageMatrix << std::endl; 
      std::cout << "----------------------------------------" << std::endl; 
    }
      
    averageMatrix = MatrixLinearCombinationFunctionsType::ComputeMatrixExponential(averageMatrix.GetVnlMatrix()); 
    averageTransform->SetFullAffineMatrix(averageMatrix); 
    averageTransform->SetParametersFromTransform(averageTransform->GetFullAffineTransform());     
    std::cout << "averageMatrix=" << averageMatrix << std::endl; 
    
    TransformFileWriterType::Pointer transformFileWriter = TransformFileWriterType::New();
    transformFileWriter->SetInput(averageTransform);
    transformFileWriter->SetFileName(outputName); 
    transformFileWriter->Update(); 
  }    
  catch (itk::ExceptionObject& exceptionObject)
  {
    std::cout << "Failed:" << exceptionObject << std::endl;
    return 2; 
  }
  
  return 0; 
}


