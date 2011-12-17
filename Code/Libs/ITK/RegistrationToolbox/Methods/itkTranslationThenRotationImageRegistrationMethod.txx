/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.
 
 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-09-20 20:57:34 +0100 (Tue, 20 Sep 2011) $
 Revision          : $Revision: 7341 $
 Last modified by  : $Author: ad $
 
 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details. 

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#ifndef _itkTranslationThenRotationImageRegistrationMethod_txx
#define _itkTranslationThenRotationRegistrationMethod_txx

#include "itkLogHelper.h"
#include "itkTranslationThenRotationImageRegistrationMethod.h"

namespace itk
{
/*
 * Constructor
 */
template < typename TInputImageType >
TranslationThenRotationImageRegistrationMethod<TInputImageType>
::TranslationThenRotationImageRegistrationMethod()
{
  niftkitkDebugMacro(<<"Constructed:TranslationThenRotationImageRegistrationMethod");
}

/*
 * The optimize bit that we can now override.
 */
template < typename TInputImageType >
void
TranslationThenRotationImageRegistrationMethod<TInputImageType>
::DoMultiStageRegistration(OptimizerPointer optimizer, TransformPointer transform)  throw (ExceptionObject)
{
  niftkitkDebugMacro(<<"TranslationThenRotationImageRegistrationMethod::DoRegistration::Start");
  
  int i = 0;
  int currentNumberOfDof;
  
  double maxStepSize = optimizer->GetMaximumStepLength();
  double minStepSize = optimizer->GetMinimumStepLength();
  double currentStepSize = maxStepSize;
  double reductionFactor = this->GetLoopStepSizeReductionFactor();
  double tolerance = this->GetLoopTolerance();
  int    numberOfIterations = this->GetMaxNumberOfLoops();
  
  double translationMetricValue = 0;
  double rotationMetricValue = 0;

  ParametersType params;
  
  ScalesType scales;
  scales.Fill(1);

  // We do translation and rotations without brain mask.
  this->SetUseFixedMask(false);
  this->SetUseMovingMask(false);
  this->Initialize();
  
  i = 0;  
  do
    {

      do
        {
          
          transform->SetFullAffine();
          params = transform->GetParameters();    
          
          niftkitkDebugMacro(<<"Start of loop:" << i << ", params:" << params << ", currentStepSize:" <<  currentStepSize);
      
          transform->SetJustTranslation();
          currentNumberOfDof = transform->GetNumberOfDOF();
          
          niftkitkDebugMacro(<<"Before translation, parameters are:" << transform->GetParameters() << ", dof:" << currentNumberOfDof << std::endl);

          scales.SetSize(currentNumberOfDof);
          scales.Fill(1);
          
          optimizer->SetInitialPosition(transform->GetParameters());      
          optimizer->SetScales(scales);
          optimizer->SetMaximumStepLength(currentStepSize);
          optimizer->SetMinimumStepLength(currentStepSize);
          optimizer->StartOptimization();

          transform->SetParameters(optimizer->GetCurrentPosition());
          translationMetricValue = optimizer->GetValue();
          
          transform->SetFullAffine();
          params = transform->GetParameters();            
          niftkitkDebugMacro(<<"After translation:" << i << ", params:" << params << ", value:" << translationMetricValue);
      
          // Do rotations
          transform->SetJustRotation();
          currentNumberOfDof = transform->GetNumberOfDOF();
          niftkitkDebugMacro(<<"Before rotation, parameters are:" << transform->GetParameters()  << ", dof:" << currentNumberOfDof << std::endl);
      
          scales.SetSize(currentNumberOfDof);
          scales.Fill(1);
          
          optimizer->SetInitialPosition(transform->GetParameters());
          optimizer->SetScales(scales);
          optimizer->SetMaximumStepLength(currentStepSize/2.0);
          optimizer->SetMinimumStepLength(currentStepSize/2.0);          
          optimizer->StartOptimization();

          transform->SetParameters(optimizer->GetCurrentPosition());
          rotationMetricValue = optimizer->GetValue();
          
          transform->SetFullAffine();
          params = transform->GetParameters();   
          niftkitkDebugMacro(<<"After rotation:" << i << ", params:" << params << ", value:" << rotationMetricValue);
      
          i++;  
        }
      while (fabs(translationMetricValue - rotationMetricValue) > tolerance && i < numberOfIterations);
      
      currentStepSize *= reductionFactor;
    }
  while (currentStepSize > minStepSize);
  
  niftkitkDebugMacro(<<"TranslationThenRotationImageRegistrationMethod::DoRegistration::Finish");
}

} // end namespace itk


#endif
