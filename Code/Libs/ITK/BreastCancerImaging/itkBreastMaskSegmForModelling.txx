/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "itkBreastMaskSegmForModelling.h"


namespace itk
{

// --------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
BreastMaskSegmForModelling< ImageDimension, InputPixelType >
::BreastMaskSegmForModelling()
{

};


// --------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
BreastMaskSegmForModelling< ImageDimension, InputPixelType >
::~BreastMaskSegmForModelling()
{

};


// --------------------------------------------------------------------------
// Execute()
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmForModelling< ImageDimension, InputPixelType >
::Execute( void )
{
  unsigned long iPointPec = 0;

  // Initialise the segmentation
  this->Initialise();
  this->SmoothTheInputImages();
  this->GreyScaleClosing();

  // Calculate the Maximum Image
  this->CalculateTheMaximumImage();

  // Segment the backgound using the maximum image histogram
  if ( this->bgndThresholdProb )
  {
    this->SegmentBackground();
  }
  else
  {
    this->SegmentForegroundFromBackground();
  }

  // Find the nipple and mid-sternum landmarks
  this->FindBreastLandmarks();

  // Compute a 2D map of the height of the patient's anterior skin
  // surface and use it to remove the arms
  this->ComputeElevationOfAnteriorSurface();

  // Segment the Pectoral Muscle
  typename InternalImageType::SizeType 
    maxSize = this->imStructural->GetLargestPossibleRegion().GetSize();

  RealType rYHeightOffset = static_cast< RealType >( maxSize[1] );

  typename PointSetType::Pointer pecPointSet = 
    this->SegmentThePectoralMuscle( rYHeightOffset, iPointPec, true );

  MaskThePectoralMuscleOnly( rYHeightOffset, pecPointSet );

  // Discard anything not within a fitted surface (switch -cropfit)
  if ( this->flgCropWithFittedSurface )
    this->MaskWithBSplineBreastSurface( rYHeightOffset );

  // OR: for prone-supine scheme: clip at a distance of 40mm 
  //     posterior to the mid sternum point
  else
    this->MaskAtDistancePosteriorToMidSternum();

  // Finally smooth the mask and threshold to round corners etc.
  this->SmoothMask();

}


// --------------------------------------------------------------------------
// Mask the pectoral muscle using a B-Spline surface
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmForModelling< ImageDimension, InputPixelType >
::MaskThePectoralMuscleOnly( RealType rYHeightOffset, 
			     typename PointSetType::Pointer &pecPointSet )
{


  // Fit the B-Spline surface to the pectoral surface
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  typename InternalImageType::Pointer imFittedPectoralis;

  // We require smaller kernel support for the prone-supine case 

  imFittedPectoralis = 
    this->MaskImageFromBSplineFittedSurface( pecPointSet, 
					     this->imStructural->GetLargestPossibleRegion(), 
					     this->imStructural->GetOrigin(), 
					     this->imStructural->GetSpacing(), 
					     this->imStructural->GetDirection(), 
					     rYHeightOffset,
					     3, this->pecControlPointSpacing, 3, true );

  // Write the fitted surface to file

  this->WriteBinaryImageToUCharFileOrVTKSurfaceFile( this->fileOutputPectoralSurfaceMask, 
			  "fitted pectoral surface with offset", 
			  imFittedPectoralis, this->flgLeft, this->flgRight );

  // Write the chest surface points to a file?

  // Discard anything within the pectoral mask (i.e. below the surface fit)
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  IteratorType itSeg    = IteratorType( this->imSegmented,        
					this->imStructural->GetLargestPossibleRegion() );

  IteratorType itFitPec = IteratorType( imFittedPectoralis, 
					this->imStructural->GetLargestPossibleRegion() );

  if ( this->flgVerbose ) 
    std::cout << "Discarding segmentation posterior to pectoralis mask. " 
	      << std::endl;
  
  for ( itSeg.GoToBegin(), itFitPec.GoToBegin(); 
        ( ! itSeg.IsAtEnd() ) && ( ! itFitPec.IsAtEnd() ) ; 
        ++itSeg, ++itFitPec )
  {
    if ( itSeg.Get() )
      if ( itFitPec.Get() )
        itSeg.Set( 0 );
  }

  this->imPectoralVoxels = 0;
}
 
// --------------------------------------------------------------------------
// Mask at a distance posterior to the mid-sterum point
// --------------------------------------------------------------------------

template <const unsigned int ImageDimension, class InputPixelType>
void
BreastMaskSegmForModelling< ImageDimension, InputPixelType >
::MaskAtDistancePosteriorToMidSternum( void )
{
  
  std::cout << "Cropping segmented region " 
            << this->cropDistPosteriorToMidSternum 
            << "mm posterior to mid sternum location."
            << std::cout;

  typename InternalImageType::RegionType region;
  typename InternalImageType::SizeType size;
  typename InternalImageType::IndexType start;

  IteratorType itSeg = IteratorType( this->imSegmented,        
				     this->imStructural->GetLargestPossibleRegion() );

  //InternalImageType::SizeType sizeChestSurfaceRegion;
  region = this->imSegmented->GetLargestPossibleRegion();
  
  const typename InternalImageType::SpacingType& sp = this->imSegmented->GetSpacing();

  start    = region.GetIndex();
  start[0] = 0;
  start[1] = static_cast<typename InternalImageType::IndexValueType>(this->idxMidSternum[1] + ( this->cropDistPosteriorToMidSternum / sp[1]));
  start[2] = 0;
  
  size    = region.GetSize();
  size[1] = size[1] - start[1];

  region.SetSize( size );
  region.SetIndex( start );

  itSeg = IteratorType( this->imSegmented, region );

  for ( itSeg.GoToBegin() ; ( ! itSeg.IsAtEnd() ) ; ++itSeg )
  {
    itSeg.Set(0);
  }
 
}


} // namespace itk
