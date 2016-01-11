/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "MorphologicalSegmentorPipeline.h"
#include <itkConversionUtils.h>
#include <itkMIDASHelper.h>


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::MorphologicalSegmentorPipeline()
{
  unsigned long int capacity = 2000000;
  
  // This is the main pipeline that will form the whole of the final output.
  m_ThresholdingFilter = ThresholdingFilterType::New();
  m_ThresholdingMaskFilter = MaskByRegionFilterType::New();
  m_ThresholdingConnectedComponentFilter = LargestConnectedComponentFilterType::New();
  m_ThresholdingConnectedComponentFilter->SetCapacity(capacity);
  m_ErosionFilter = ErosionFilterType::New();
  m_ErosionMaskFilter = MaskByRegionFilterType::New();
  m_ErosionConnectedComponentFilter = LargestConnectedComponentFilterType::New();
  m_ErosionConnectedComponentFilter->SetCapacity(capacity);
  m_DilationFilter = DilationFilterType::New();
  m_DilationMaskFilter = MaskByRegionFilterType::New();
  m_DilationConnectedComponentFilter = LargestConnectedComponentFilterType::New();
  m_DilationConnectedComponentFilter->SetCapacity(capacity);
  m_RethresholdingFilter = RethresholdingFilterType::New();

  this->SetForegroundValue((unsigned char)1);
  this->SetBackgroundValue((unsigned char)0);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::~MorphologicalSegmentorPipeline()
{
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::SetForegroundValue(unsigned char foregroundValue)
{
  m_ForegroundValue = foregroundValue;
  this->UpdateForegroundValues();
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::SetBackgroundValue(unsigned char backgroundValue)
{
  m_BackgroundValue = backgroundValue;
  this->UpdateBackgroundValues();
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension> 
::UpdateForegroundValues()
{
  m_ThresholdingFilter->SetInsideValue(m_ForegroundValue);
  m_ThresholdingConnectedComponentFilter->SetOutputForegroundValue(m_ForegroundValue);
  m_ErosionFilter->SetInValue(m_ForegroundValue);
  m_ErosionConnectedComponentFilter->SetOutputForegroundValue(m_ForegroundValue);
  m_DilationFilter->SetInValue(m_ForegroundValue);
  m_DilationConnectedComponentFilter->SetOutputForegroundValue(m_ForegroundValue);
  m_RethresholdingFilter->SetInValue(m_ForegroundValue);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension>  
::UpdateBackgroundValues()
{
  m_ThresholdingFilter->SetOutsideValue(m_BackgroundValue);
  m_ThresholdingMaskFilter->SetOutputBackgroundValue(m_BackgroundValue);
  m_ThresholdingConnectedComponentFilter->SetInputBackgroundValue(m_BackgroundValue);
  m_ThresholdingConnectedComponentFilter->SetOutputBackgroundValue(m_BackgroundValue);
  m_ErosionFilter->SetOutValue(m_BackgroundValue);
  m_ErosionMaskFilter->SetOutputBackgroundValue(m_BackgroundValue);
  m_ErosionConnectedComponentFilter->SetInputBackgroundValue(m_BackgroundValue);
  m_ErosionConnectedComponentFilter->SetOutputBackgroundValue(m_BackgroundValue);
  m_DilationFilter->SetOutValue(m_BackgroundValue);
  m_DilationMaskFilter->SetOutputBackgroundValue(m_BackgroundValue);
  m_DilationConnectedComponentFilter->SetInputBackgroundValue(m_BackgroundValue);
  m_DilationConnectedComponentFilter->SetOutputBackgroundValue(m_BackgroundValue);;
  m_RethresholdingFilter->SetOutValue(m_BackgroundValue);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::SetInputs(const GreyScaleImageType* referenceImage,
    const SegmentationImageType* erosionsAdditionsImage,
    const SegmentationImageType* erosionsSubtractionsImage,
    const SegmentationImageType* dilationsAdditionsImage,
    const SegmentationImageType* dilationsSubtractionsImage)
{
  m_ThresholdingFilter->SetInput(referenceImage);
  m_ErosionFilter->SetGreyScaleImageInput(referenceImage);
  m_DilationFilter->SetGreyScaleImageInput(referenceImage);
  m_RethresholdingFilter->SetGreyScaleImageInput(referenceImage);

  m_ErosionMaskFilter->SetInput(1, erosionsAdditionsImage);
  m_ErosionMaskFilter->SetInput(2, erosionsSubtractionsImage);
  m_DilationMaskFilter->SetInput(1, dilationsAdditionsImage);
  m_DilationFilter->SetConnectionBreakerImage(dilationsSubtractionsImage);
  m_DilationMaskFilter->SetInput(2, dilationsSubtractionsImage);

  m_ThresholdingMaskFilter->SetInput(m_ThresholdingFilter->GetOutput());
  m_ThresholdingConnectedComponentFilter->SetInput(m_ThresholdingMaskFilter->GetOutput());

  m_ErosionFilter->SetBinaryImageInput(m_ThresholdingConnectedComponentFilter->GetOutput());
  m_ErosionMaskFilter->SetInput(0, m_ErosionFilter->GetOutput());
  m_ErosionConnectedComponentFilter->SetInput(m_ErosionMaskFilter->GetOutput());

  m_DilationFilter->SetBinaryImageInput(m_ErosionConnectedComponentFilter->GetOutput());
  m_DilationMaskFilter->SetInput(0, m_DilationFilter->GetOutput());
  m_DilationConnectedComponentFilter->SetInput(m_DilationMaskFilter->GetOutput());

  m_RethresholdingFilter->SetBinaryImageInput(m_DilationConnectedComponentFilter->GetOutput());
  m_RethresholdingFilter->SetThresholdedImageInput(m_ThresholdingMaskFilter->GetOutput());
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::SetErosionSubtractionsInput(const SegmentationImageType* erosionsSubtractionsImage)
{
  m_ErosionMaskFilter->SetInput(2, erosionsSubtractionsImage);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::SetDilationSubtractionsInput(const SegmentationImageType* dilationsSubtractionsImage)
{
  m_DilationFilter->SetConnectionBreakerImage(dilationsSubtractionsImage);
  m_DilationMaskFilter->SetInput(2, dilationsSubtractionsImage);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::SetParams(const MorphologicalSegmentorPipelineParams& params)
{
  int startStage = params.m_StartStage;
  m_Stage = params.m_Stage;

  // Note, the ITK Set/Get Macro ensures that the Modified flag only gets set if the value set is actually different.

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // Start Trac 998, setting region of interest, on all Mask filters, to produce Axial-Cut-off effect.
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  
  typename SegmentationImageType::RegionType         regionOfInterest;
  typename SegmentationImageType::IndexType          regionOfInterestIndex;
  typename SegmentationImageType::SizeType           regionOfInterestSize;

  typename GreyScaleImageType::ConstPointer referenceImage = m_ThresholdingFilter->GetInput();
  
  // 1. Set region to full size of input image
  regionOfInterestIndex = referenceImage->GetLargestPossibleRegion().GetIndex();
  regionOfInterestSize = referenceImage->GetLargestPossibleRegion().GetSize();

  // 2. Get string describing orientation.
  typename itk::SpatialOrientationAdapter adaptor;
  typename itk::SpatialOrientation::ValidCoordinateOrientationFlags orientation;
  orientation = adaptor.FromDirectionCosines(referenceImage->GetDirection());
  std::string orientationString = itk::ConvertSpatialOrientationToString(orientation);

  // 3. Get Axis that represents superior/inferior
  int axialAxis = -1;
  itk::GetAxisFromITKImage<TPixel, VImageDimension>(referenceImage, itk::ORIENTATION_AXIAL, axialAxis);
  
  if (axialAxis != -1)
  {
    // 4. Calculate size of region of interest in that axis
    if (orientationString[axialAxis] == 'I')
    {
      regionOfInterestIndex[axialAxis] = params.m_AxialCutOffSlice;
      regionOfInterestSize[axialAxis] = regionOfInterestSize[axialAxis] - params.m_AxialCutOffSlice;
    }
    else
    {
      regionOfInterestIndex[axialAxis] = 0;
      regionOfInterestSize[axialAxis] = params.m_AxialCutOffSlice + 1;
    }
  }

  // 5. Set region on both filters
  regionOfInterest.SetIndex(regionOfInterestIndex);
  regionOfInterest.SetSize(regionOfInterestSize);

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // End Trac 998, setting region of interest, on Mask filters
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // Start Trac 1131, calculate a rough size to help LargestConnectedComponents allocate memory.
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  unsigned long int expectedSize = regionOfInterest.GetNumberOfPixels() / 8;
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // End Trac 1131.
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  
  if (startStage <= THRESHOLDING && m_Stage >= THRESHOLDING)
  {
    m_ThresholdingFilter->SetLowerThreshold((TPixel)params.m_LowerIntensityThreshold);
    m_ThresholdingFilter->SetUpperThreshold((TPixel)params.m_UpperIntensityThreshold);

    m_ThresholdingMaskFilter->SetRegion(regionOfInterest);
  }

  if (startStage <= EROSION && m_Stage >= EROSION)
  {
    m_ThresholdingConnectedComponentFilter->SetCapacity(expectedSize);

    m_ErosionFilter->SetRegion(regionOfInterest);
    m_ErosionFilter->SetUpperThreshold((TPixel)params.m_UpperErosionsThreshold);
    m_ErosionFilter->SetNumberOfIterations(params.m_NumberOfErosions);
    m_ErosionFilter->SetRegion(regionOfInterest);

    m_ErosionMaskFilter->SetRegion(regionOfInterest);

    m_ErosionConnectedComponentFilter->SetCapacity(expectedSize);
  }

  if (startStage <= DILATION && m_Stage >= DILATION)
  {
    m_DilationFilter->SetLowerThreshold((int)(params.m_LowerPercentageThresholdForDilations));
    m_DilationFilter->SetUpperThreshold((int)(params.m_UpperPercentageThresholdForDilations));
    m_DilationFilter->SetNumberOfIterations((int)(params.m_NumberOfDilations));
    m_DilationFilter->SetRegion(regionOfInterest);

    m_DilationMaskFilter->SetRegion(regionOfInterest);

    m_DilationConnectedComponentFilter->SetCapacity(expectedSize);
  }

  if (startStage <= RETHRESHOLDING && m_Stage >= RETHRESHOLDING)
  {
    m_RethresholdingFilter->SetDownSamplingFactor(params.m_BoxSize);
    m_RethresholdingFilter->SetLowPercentageThreshold((int)(params.m_LowerPercentageThresholdForDilations));
    m_RethresholdingFilter->SetHighPercentageThreshold((int)(params.m_UpperPercentageThresholdForDilations));
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::Update(const std::vector<bool>& editingFlags, const std::vector<int>& editingRegion)
{
  if (m_Stage == THRESHOLDING)
  {
    m_ThresholdingMaskFilter->UpdateLargestPossibleRegion();
  }
  else if (m_Stage == EROSION || m_Stage == DILATION)
  {
    // Simple cases first - no editing.
    if (m_Stage == EROSION && !editingFlags[0] && !editingFlags[1])
    {
      m_ErosionConnectedComponentFilter->UpdateLargestPossibleRegion();
    }
    else if (m_Stage == DILATION && !editingFlags[2] && !editingFlags[3])
    {
      m_DilationConnectedComponentFilter->UpdateLargestPossibleRegion();
    }
    else
    {
      // Else: We are doing live updates.
      // Note: We try and update as small a section of the pipeline as possible - as GUI has to be interactive.
      typename SegmentationImageType::ConstPointer inputImage =
          m_Stage == EROSION ? m_ErosionMaskFilter->GetInput(editingFlags[0] ? 1 : 2) : m_DilationMaskFilter->GetInput(editingFlags[2] ? 1 : 2);
      typename SegmentationImageType::Pointer outputImage =
          m_Stage == EROSION ? m_ErosionConnectedComponentFilter->GetOutput() : m_DilationConnectedComponentFilter->GetOutput();

      typename SegmentationImageType::RegionType editingROI;
      for (int i = 0; i < 3; ++i)
      {
        editingROI.SetIndex(i, editingRegion[i]);
        editingROI.SetSize(i, editingRegion[i + 3]);
      }

      itk::ImageRegionConstIterator<SegmentationImageType> editedRegionIt(inputImage, editingROI);
      itk::ImageRegionIterator<SegmentationImageType> outputIt(outputImage, editingROI);

      if (editingFlags[0] || editingFlags[2])
      {
        for (outputIt.GoToBegin(), editedRegionIt.GoToBegin(); !outputIt.IsAtEnd(); ++outputIt, ++editedRegionIt)
        {
          outputIt.Set(outputIt.Get() || editedRegionIt.Get() ? m_ForegroundValue : m_BackgroundValue);
        }
      }
      else
      {
        for (outputIt.GoToBegin(), editedRegionIt.GoToBegin(); !outputIt.IsAtEnd(); ++outputIt, ++editedRegionIt)
        {
          if (editedRegionIt.Get())
          {
            outputIt.Set(m_BackgroundValue);
          }
        }
      }
    }
  }
  else if (m_Stage == RETHRESHOLDING)
  {  
    m_RethresholdingFilter->UpdateLargestPossibleRegion();
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
typename MorphologicalSegmentorPipeline<TPixel, VImageDimension>::SegmentationImageType::Pointer
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::GetOutput()
{
  return this->GetOutput(m_Stage);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
typename MorphologicalSegmentorPipeline<TPixel, VImageDimension>::SegmentationImageType::Pointer
MorphologicalSegmentorPipeline<TPixel, VImageDimension>
::GetOutput(int stage)
{
  typename SegmentationImageType::Pointer result;

  if (stage == THRESHOLDING)
  {
    result = m_ThresholdingMaskFilter->GetOutput();
  }
  else if (stage == EROSION)
  {
    result = m_ErosionConnectedComponentFilter->GetOutput();
  }
  else if (stage == DILATION)
  {
    result = m_DilationConnectedComponentFilter->GetOutput();
  }
  else if (stage == RETHRESHOLDING)
  {
    result = m_RethresholdingFilter->GetOutput();
  }

  return result;
}
