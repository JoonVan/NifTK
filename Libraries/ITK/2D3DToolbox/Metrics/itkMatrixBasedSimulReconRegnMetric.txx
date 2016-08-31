/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __itkMatrixBasedSimulReconRegnMetric_txx
#define __itkMatrixBasedSimulReconRegnMetric_txx

#include "itkMatrixBasedSimulReconRegnMetric.h"
#include <itkImageRegionIterator.h>


namespace itk
{

/* -----------------------------------------------------------------------
   Constructor
   ----------------------------------------------------------------------- */

template <class IntensityType>
MatrixBasedSimulReconRegnMetric<IntensityType>
::MatrixBasedSimulReconRegnMetric()
{
  // suffixOutputCurrentEstimate = "nii";

  // Create the matrix projector
  m_MatrixProjector = MatrixProjectorType::New();

  // Allocate the affine transformer
	m_AffineTransformer = AffineTransformerType::New();
}


/* -----------------------------------------------------------------------
   PrintSelf
   ----------------------------------------------------------------------- */

template <class IntensityType>
void
MatrixBasedSimulReconRegnMetric<IntensityType>
::PrintSelf(std::ostream& os, Indent indent) const
{
	Superclass::PrintSelf(os,indent);
	os << indent << "CreateForwardBackwardProjectionMatrix: " << std::endl;
}


/* -----------------------------------------------------------------------
   GetNumberOfParameters()
   ----------------------------------------------------------------------- */

template< class IntensityType>
unsigned int 
MatrixBasedSimulReconRegnMetric<IntensityType>
::GetNumberOfParameters( void ) const
{
	assert ( (m_EstimatedVolumeVector.size()!=0) && (m_TransformationParameterVector.size()!=0) );
  unsigned int nParameters = m_EstimatedVolumeVector.size() + m_TransformationParameterVector.size();
  return nParameters;
}


/* -----------------------------------------------------------------------
   GetValue() - Get the value of the similarity metric
   ----------------------------------------------------------------------- */

template< class IntensityType>
typename MatrixBasedSimulReconRegnMetric<IntensityType>::MeasureType
MatrixBasedSimulReconRegnMetric<IntensityType>
::GetValue( const ParametersType &parameters ) const
{

    niftkitkDebugMacro(<< "The optimisation parameter size is: " << parameters.Size());

	// Get the volume estimate and the transformation parameters as a vector respeectively
	VectorType parametersVector = parameters;
  VectorType m_EstimatedVolumeVector = parametersVector.extract(m_totalSize3D, 0);
	VectorType m_TransformationParameterVector = parameters.extract(m_paraNumber, m_totalSize3D);

	std::ofstream estimateVolumeIniFile("estimateVolumeIniFile.txt", std::ios::out | std::ios::app | std::ios::binary);
  estimateVolumeIniFile << m_EstimatedVolumeVector << " " << std::endl;

	std::ofstream MTransformationParameterVectorFile("MTransformationParameterVectorFile.txt", std::ios::out | std::ios::app | std::ios::binary);
  MTransformationParameterVectorFile << m_TransformationParameterVector << " " << std::endl;

    niftkitkDebugMacro(<< "The size of the volume vector is: " << m_EstimatedVolumeVector.size());
    niftkitkDebugMacro(<< "The size of the transformation vector is: " << m_TransformationParameterVector.size());
  
  // Change the updated input volume vector into 3D image
  InputVolumeIndexType inIndex;

  ImageRegionIterator<InputVolumeType> inVolumeIterator;
  inVolumeIterator = ImageRegionIterator<InputVolumeType>(m_inVolume, m_inVolume->GetLargestPossibleRegion());

  unsigned long int voxelNumber = 0;
  for ( inVolumeIterator.GoToBegin(); !inVolumeIterator.IsAtEnd(); ++inVolumeIterator)
  {

    // Determine the coordinate of the input volume
    inIndex = inVolumeIterator.GetIndex();
    m_inVolume->SetPixel(inIndex, m_EstimatedVolumeVector[voxelNumber]);

    voxelNumber++;

  }

	// Allocate the matrix projector
	MatrixProjectorPointerType	m_MatrixProjector;
  if ( m_MatrixProjector.IsNull() )
    m_MatrixProjector = MatrixProjectorType::New();

	InputVolumeSizeType inVolumeSize 		= m_InVolumeSize;
	InputProjectionSizeType inProjSize 	= m_InProjectionSize;

	// Create the corresponding forward/backward projection matrix
  const unsigned long int totalSizeAllProjs = m_ProjectionNumber*m_totalSize2D;
  static SparseMatrixType forwardProjectionMatrix(totalSizeAllProjs, m_totalSize3D);

	// Set the projection geometry
	m_MatrixProjector->SetProjectionGeometry( m_Geometry );

  m_MatrixProjector->GetForwardProjectionSparseMatrix(forwardProjectionMatrix, m_inVolume, m_inProjTemp, 
       inVolumeSize, inProjSize, m_ProjectionNumber);

  // Calculate the matrix/vector multiplication in order to get the forward projection (Ax)
  VectorType forwardProjectedVectorOne(m_totalSize3D);
  forwardProjectedVectorOne.fill(0.);

  m_MatrixProjector->CalculteMatrixVectorMultiplication(forwardProjectionMatrix, m_EstimatedVolumeVector, forwardProjectedVectorOne);

	// Create the corresponding transformation matrix
	static SparseMatrixType affineMatrix(m_totalSize3D, m_totalSize3D);

	EulerAffineTransformType::ParametersType tempEulerAffineParameters(m_paraNumber);
	tempEulerAffineParameters.Fill(0.);

/*
	tempEulerAffineParameters.SetElement(0, -5.);
	tempEulerAffineParameters.SetElement(2, 10.);

	tempEulerAffineParameters.SetElement(4, 30.0);

	tempEulerAffineParameters.SetElement(6, 1.0);
	tempEulerAffineParameters.SetElement(7, 1.0);
	tempEulerAffineParameters.SetElement(8, 1.0);
*/

	for (unsigned int iPara = 0; iPara < m_paraNumber; iPara++)
		 tempEulerAffineParameters[iPara] = m_TransformationParameterVector[iPara];

	std::ofstream tempEulerAffineParametersFile("tempEulerAffineParametersFile.txt", std::ios::out | std::ios::app | std::ios::binary);
  tempEulerAffineParametersFile << tempEulerAffineParameters << " " << std::endl;

	m_AffineTransformer->GetAffineTransformationSparseMatrix(affineMatrix, inVolumeSize, tempEulerAffineParameters);

	// Calculate the matrix/vector multiplication in order to get the affine transformation (Rx)
	VectorType affineTransformedVector(m_totalSize3D);
	affineTransformedVector.fill(0.);

	m_AffineTransformer->CalculteMatrixVectorMultiplication(affineMatrix, m_EstimatedVolumeVector, affineTransformedVector);

	// Calculate the matrix/vector multiplication in order to get the forward projection (ARx)
	assert (!affineTransformedVector.is_zero());
	VectorType forwardProjectedVectorTwo(m_totalSize3D);
	forwardProjectedVectorTwo.fill(0.);
			
	m_MatrixProjector->CalculteMatrixVectorMultiplication(forwardProjectionMatrix, affineTransformedVector, forwardProjectedVectorTwo);

	// Initialise the current measure
  MeasureType currentMeasure;
  currentMeasure = 0.;

	// Calculate (Ax - y_1) and (ARx - y_2)
	VectorType	m_inProjOneSub(m_totalSize3D);
	VectorType	m_inProjTwoSub(m_totalSize3D);
	m_inProjOneSub.fill(0.);
	m_inProjTwoSub.fill(0.);
			
	assert( !(this->m_inProjOne.is_zero()) && !(this->m_inProjTwo.is_zero()) );
	m_inProjOneSub = forwardProjectedVectorOne - this->m_inProjOne;
	m_inProjTwoSub = forwardProjectedVectorTwo - this->m_inProjTwo;
			
	std::ofstream projOneVectorFile("projOneVectorFile.txt", std::ios::out | std::ios::app | std::ios::binary);
	projOneVectorFile << m_inProjOneSub << " ";

	std::ofstream projTwoVectorFile("projTwoVectorFile.txt", std::ios::out | std::ios::app | std::ios::binary);
	projTwoVectorFile << m_inProjTwoSub << " ";

	std::ofstream estimateVolumeFile("estimateVolumeFile.txt", std::ios::out | std::ios::app | std::ios::binary);
	estimateVolumeFile << m_EstimatedVolumeVector << " ";

			
	// Calculate (||Ax - y_1||^2 + ||ARx - y_2||^2), which is the cost function value
	currentMeasure = m_inProjOneSub.squared_magnitude() + m_inProjTwoSub.squared_magnitude();
	std::cerr << "Current cost function value is: " << currentMeasure << std::endl;

	std::ofstream costFunctionValueFile("costFunctionValueFile.txt", std::ios::out | std::ios::app | std::ios::binary);
  costFunctionValueFile << currentMeasure << std::endl;

  return currentMeasure;

}


/* -----------------------------------------------------------------------
   GetDerivative() - Get the derivative of the similarity metric
   ----------------------------------------------------------------------- */

template< class IntensityType>
void
MatrixBasedSimulReconRegnMetric<IntensityType>
::GetDerivative( const ParametersType &parameters, 
                 DerivativeType &derivative ) const
{

	niftkitkDebugMacro(<< "The optimisation derivative size is: " << derivative.Size());

	// Get the volume estimate and the transformation parameters as a vector respeectively
	VectorType parametersVector = parameters;
  VectorType m_EstimatedVolumeVector = parametersVector.extract(m_totalSize3D, 0);
	VectorType m_TransformationParameterVector = parameters.extract(m_paraNumber, m_totalSize3D);

  // Change the updated input volume vector into 3D image
  InputVolumeIndexType inIndex;

  ImageRegionIterator<InputVolumeType> inVolumeIterator;
  inVolumeIterator = ImageRegionIterator<InputVolumeType>(m_inVolume, m_inVolume->GetLargestPossibleRegion());

  unsigned long int voxelNumber = 0;
  for ( inVolumeIterator.GoToBegin(); !inVolumeIterator.IsAtEnd(); ++inVolumeIterator)
  {

    // Determine the coordinate of the input volume
    inIndex = inVolumeIterator.GetIndex();
    m_inVolume->SetPixel(inIndex, m_EstimatedVolumeVector[voxelNumber]);

    voxelNumber++;

  }

	// Allocate the matrix projector
	MatrixProjectorPointerType	m_MatrixProjector;
  if ( m_MatrixProjector.IsNull() )
    m_MatrixProjector = MatrixProjectorType::New();

	InputVolumeSizeType inVolumeSize 		= m_InVolumeSize;
	InputProjectionSizeType inProjSize 	= m_InProjectionSize;

	// Create the corresponding forward/backward projection matrix
  const unsigned long int totalSizeAllProjs = m_ProjectionNumber*m_totalSize2D;
  static SparseMatrixType forwardProjectionMatrix(totalSizeAllProjs, m_totalSize3D);
  static SparseMatrixType backwardProjectionMatrix(m_totalSize3D, totalSizeAllProjs);

	// Set the projection geometry
	m_MatrixProjector->SetProjectionGeometry( m_Geometry );

  m_MatrixProjector->GetForwardProjectionSparseMatrix(forwardProjectionMatrix, m_inVolume, m_inProjTemp, 
       inVolumeSize, inProjSize, m_ProjectionNumber);
  m_MatrixProjector->GetBackwardProjectionSparseMatrix(forwardProjectionMatrix, backwardProjectionMatrix, 
       inVolumeSize, inProjSize, m_ProjectionNumber);

  // Calculate the matrix/vector multiplication in order to get the forward projection (Ax)
  VectorType forwardProjectedVectorOne(m_totalSize3D);
  forwardProjectedVectorOne.fill(0.);

  m_MatrixProjector->CalculteMatrixVectorMultiplication(forwardProjectionMatrix, m_EstimatedVolumeVector, forwardProjectedVectorOne);

	// Create the corresponding transformation matrix
	static SparseMatrixType affineMatrix(m_totalSize3D, m_totalSize3D);
	static SparseMatrixType affineMatrixTranspose(m_totalSize3D, m_totalSize3D);

	EulerAffineTransformType::ParametersType tempEulerAffineParameters(m_paraNumber);
	tempEulerAffineParameters.Fill(0.);

/*
	tempEulerAffineParameters.SetElement(0, -5.);
	tempEulerAffineParameters.SetElement(2, 10.);

	tempEulerAffineParameters.SetElement(4, 30.0);

	tempEulerAffineParameters.SetElement(6, 1.0);
	tempEulerAffineParameters.SetElement(7, 1.0);
	tempEulerAffineParameters.SetElement(8, 1.0);
*/

	for (unsigned int iPara = 0; iPara < m_paraNumber; iPara++)
		 tempEulerAffineParameters[iPara] = m_TransformationParameterVector[iPara];

	m_AffineTransformer->GetAffineTransformationSparseMatrix(affineMatrix, inVolumeSize, tempEulerAffineParameters);
  m_AffineTransformer->GetAffineTransformationSparseMatrixT(affineMatrix, affineMatrixTranspose, inVolumeSize);

	// Calculate the matrix/vector multiplication in order to get the affine transformation (Rx)
	VectorType affineTransformedVector(m_totalSize3D);
	affineTransformedVector.fill(0.);

	m_AffineTransformer->CalculteMatrixVectorMultiplication(affineMatrix, m_EstimatedVolumeVector, affineTransformedVector);

	// Calculate the matrix/vector multiplication in order to get the forward projection (ARx)
	assert (!affineTransformedVector.is_zero());
	VectorType forwardProjectedVectorTwo(m_totalSize3D);
	forwardProjectedVectorTwo.fill(0.);
			
	m_MatrixProjector->CalculteMatrixVectorMultiplication(forwardProjectionMatrix, affineTransformedVector, forwardProjectedVectorTwo);

	// Calculate (Ax - y_1) and (ARx - y_2)
	VectorType	m_inProjOneSub(m_totalSize3D);
	VectorType	m_inProjTwoSub(m_totalSize3D);
	m_inProjOneSub.fill(0.);
	m_inProjTwoSub.fill(0.);

	assert( !(this->m_inProjOne.is_zero()) && !(this->m_inProjTwo.is_zero()) );
	m_inProjOneSub = forwardProjectedVectorOne - this->m_inProjOne;
	m_inProjTwoSub = forwardProjectedVectorTwo - this->m_inProjTwo;

	// The cost function value f = f1 + f2, and we just need f2 here
	double f2 = m_inProjTwoSub.squared_magnitude();
			
	// Process the backprojection (A^T (Ax - y_1)) and (A^T (ARx - y_2))
	// assert (!m_inProjOne.is_zero() && !m_inProjTwo.is_zero());
	VectorType	inBackProjOne(m_totalSize3D); 
	VectorType	inBackProjTwo(m_totalSize3D);
	inBackProjOne.fill(0.);
	inBackProjTwo.fill(0.);

	m_MatrixProjector->CalculteMatrixVectorMultiplication(backwardProjectionMatrix, m_inProjOneSub, inBackProjOne);
	m_MatrixProjector->CalculteMatrixVectorMultiplication(backwardProjectionMatrix, m_inProjTwoSub, inBackProjTwo);

	// Obtain the transpose of affine transformation matrix with the backprojection set two (R^T A^T (ARx - y_2))
	// assert (!inBackProjOne.is_zero() && !inBackProjTwo.is_zero());
	VectorType	inAffineTransposeBackProjTwo(m_totalSize3D);
	inAffineTransposeBackProjTwo.fill(0.);
			
	m_AffineTransformer->CalculteMatrixVectorMultiplication(affineMatrixTranspose, inBackProjTwo, inAffineTransposeBackProjTwo);

	// Create a derivative vector to store the values of the derivative
	VectorType  derivativeParameters(m_totalSize3D + m_paraNumber);
	derivativeParameters.fill(0.);

	// Update the derivative with respect to voxel values x by using (A^T (Ax - y_1) + R^T A^T (ARx - y_2))
	for ( unsigned int voxelIndex = 0; voxelIndex < m_totalSize3D; voxelIndex++ )
  {

		derivativeParameters[voxelIndex] = inBackProjOne[voxelIndex] + inAffineTransposeBackProjTwo[voxelIndex];

	}

	std::ofstream inBackProjTwoFile("inBackProjTwoFile.txt", std::ios::out | std::ios::app | std::ios::binary);
  inBackProjTwoFile << inBackProjTwo << std::endl;

	std::ofstream inAffineTransposeBackProjTwoFile("inAffineTransposeBackProjTwoFile.txt", std::ios::out | std::ios::app | std::ios::binary);
  inAffineTransposeBackProjTwoFile << inAffineTransposeBackProjTwo << std::endl;

  // Create the FDM and use it directly
	VectorType gradientImage(m_totalSize3D);
	VectorType gradientImageTemp(m_totalSize3D);
	VectorType updatedAffineTransformedImage(m_totalSize3D);
	VectorType forwardProjUpdatedAffineTransformedImage(m_totalSize3D);
	VectorType gradientImageAjoint(m_totalSize3D);
	VectorType gradientImageTempAjoint(m_totalSize3D);
	VectorType vectorAdjointOutput(m_totalSize3D);
	VectorType vectorFowardGradientImage(m_totalSize3D);
	VectorType vectorDifferenceGradientImage(m_totalSize3D);
	VectorType vectorBackwardGradientImage(m_totalSize3D);
  gradientImage.fill(0.);
  gradientImageTemp.fill(0.);
  updatedAffineTransformedImage.fill(0.);
	forwardProjUpdatedAffineTransformedImage.fill(0.);
  gradientImageAjoint.fill(0.);
  gradientImageTempAjoint.fill(0.);
  vectorAdjointOutput.fill(0.);
  vectorFowardGradientImage.fill(0.);
  vectorDifferenceGradientImage.fill(0.);
  vectorBackwardGradientImage.fill(0.);

	VectorType f2PlusSqrt(m_totalSize3D);
  f2PlusSqrt.fill(0.);
	double f2Plus;

  static SparseMatrixType affineMatrixQ(m_totalSize3D, m_totalSize3D);
  static SparseMatrixType affineMatrixQPlus(m_totalSize3D, m_totalSize3D);
  static SparseMatrixType affineMatrixQT(m_totalSize3D, m_totalSize3D);

  EulerAffineTransformType::ParametersType parametersTempPlus(12);
	parametersTempPlus.Fill(0.);

	std::ofstream derivativeParametersOutputFile("derivativeParametersOutputFile.txt", std::ios::out | std::ios::app | std::ios::binary);
	for (unsigned int iPara = 0; iPara < 12; iPara++)
	{
		// Get p + (epsilon x e)::e_k=[0 0 ...1 0 0]; 
		parametersTempPlus = tempEulerAffineParameters;
		parametersTempPlus[iPara] += m_epsilonVal;

		// Get R'(p)
		m_AffineTransformer->GetAffineTransformationSparseMatrix(affineMatrixQPlus, inVolumeSize, parametersTempPlus);
		affineMatrixQPlus.subtract(affineMatrix, affineMatrixQ);

		// Get R'(x,p)
		m_AffineTransformer->CalculteMatrixVectorMultiplication(affineMatrixQ, m_EstimatedVolumeVector, updatedAffineTransformedImage);

		// Get forward projection for R'(p), which is AR'(x,p)
		m_MatrixProjector->CalculteMatrixVectorMultiplication(forwardProjectionMatrix, updatedAffineTransformedImage, forwardProjUpdatedAffineTransformedImage);

		// Get ||AR'(x,p) - y_2||^2
		f2PlusSqrt = forwardProjUpdatedAffineTransformedImage - this->m_inProjTwo;
		f2Plus = f2PlusSqrt.squared_magnitude();

		// Get the finite difference
		derivativeParameters[m_totalSize3D+iPara] = (f2Plus - f2)/m_epsilonVal;

		niftkitkDebugMacro(<< "The derivative of the parameter " << iPara+1 << " is: " << derivativeParameters[m_totalSize3D+iPara] << std::endl << std::endl);
  	derivativeParametersOutputFile << derivativeParameters[m_totalSize3D+iPara] << std::endl;


#if 0
		// Get R'(p)^T
		m_AffineTransformer->GetAffineTransformationSparseMatrixT(affineMatrixQ, affineMatrixQT, inVolumeSize);

		// Get R'(x,p)
		m_AffineTransformer->CalculteMatrixVectorMultiplication(affineMatrixQT, m_EstimatedVolumeVector, gradientImageTemp);
		// gradientImage = gradientImageTemp / m_epsilonVal;
		gradientImage = gradientImageTemp;

		// Get forward projection for R'(x,p), which is AR'(x,p)
		m_MatrixProjector->CalculteMatrixVectorMultiplication(forwardProjectionMatrix, gradientImage, vectorFowardGradientImage);

		// Forward projection of the derivative image (AR'(x,p))^T dot product with difference (AR(x,p)-y_2): m_inProjTwoSub
		// which is (AR'(x,p))^T (AR(x,p)-y_2)
		derivativeParameters[m_totalSize3D+iPara] = dot_product( vectorFowardGradientImage, m_inProjTwoSub );
		// derivativeParameters[m_totalSize3D+iPara] = dot_product( vectorFowardGradientImage, m_inProjTwoSub ) / m_lambdaVal;
		niftkitkDebugMacro(<< "The derivative of the parameter " << iPara+1 << " is: " << derivativeParameters[m_totalSize3D+iPara] << std::endl << std::endl);

  	derivativeParametersOutputFile << derivativeParameters[m_totalSize3D+iPara] << std::endl;
#endif

	}

	std::ofstream derivativeParametersFile("derivativeParametersFile.txt", std::ios::out | std::ios::app | std::ios::binary);
  derivativeParametersFile << derivativeParameters << std::endl;

	derivative = derivativeParameters;
	
}


/* -----------------------------------------------------------------------
   GetValueAndDerivative() - Get both the value and derivative of the metric
   ----------------------------------------------------------------------- */

template< class IntensityType>
void
MatrixBasedSimulReconRegnMetric<IntensityType>
::GetValueAndDerivative(const ParametersType &parameters, 
                        MeasureType &Value, DerivativeType &Derivative) const
{
  niftkitkDebugMacro(<< "MatrixBasedSimulReconRegnMetric<IntensityType>::GetValueAndDerivative()");

  // Compute the similarity

  Value = this->GetValue( parameters );

  // Compute the derivative
  
  this->GetDerivative( parameters, Derivative );
}

} // end namespace itk


#endif
