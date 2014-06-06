/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef itkBreastMaskSegmentationFromMRI_h
#define itkBreastMaskSegmentationFromMRI_h


#include <math.h>
#include <float.h>
#include <iomanip>

#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageSliceIteratorWithIndex.h>
#include <itkImageLinearIteratorWithIndex.h>
#include <itkBasicImageFeaturesImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkMaskImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkIdentityTransform.h>
#include <itkImageDuplicator.h>
#include <itkScalarImageToHistogramGenerator.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkRayleighFunction.h>
#include <itkExponentialDecayFunction.h>
#include <itkScalarImageToHistogramGenerator.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkCurvatureFlowImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkBSplineScatteredDataPointSetToImageFilter.h>
#include <itkPointSet.h>
#include <itkRegionGrowSurfacePoints.h>
#include <itkSmoothingRecursiveGaussianImageFilter.h>
#include <itkLewisGriffinRecursiveGaussianImageFilter.h>
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <itkSigmoidImageFilter.h>
#include <itkFastMarchingImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkBasicImageFeaturesImageFilter.h>
#include <itkSliceBySliceImageFilterPatched.h>
#include <itkAffineTransform.h>
#include <itkSetBoundaryVoxelsToValueFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkMaximumImageFilter.h>
#include <itkImageAdaptor.h>

#include <vtkMarchingCubes.h> 
#include <vtkPolyDataWriter.h> 
#include <vtkSmartPointer.h>
#include <vtkWindowedSincPolyDataFilter.h> 

#include <vnl/vnl_vector.h>
#include <vnl/vnl_double_3.h>
#include <vnl/algo/vnl_levenberg_marquardt.h>

#include <boost/filesystem.hpp>

namespace itk
{

/** \class BreastMaskSegmentationFromMRI
 * \brief Base class for breast mask MRI segmentation methods.
 *
 * This class defines the common methods used by variations of the
 * breast mask segmentation.
 *
 */
template <const unsigned int ImageDimension, class InputPixelType>
class ITK_EXPORT BreastMaskSegmentationFromMRI : public Object 
{
public:
    
  typedef BreastMaskSegmentationFromMRI Self;
  typedef Object                        Superclass;
  typedef SmartPointer<Self>            Pointer;
  typedef SmartPointer<const Self>      ConstPointer;
  
  itkTypeMacro(BreastMaskSegmentationFromMRI, Object);
    
  itkStaticConstMacro( SliceDimension, unsigned int, 2 );
  itkStaticConstMacro( ParametricDimension, unsigned int, 2 );
  itkStaticConstMacro( DataDimension, unsigned int, 1 );
    
  typedef float RealType;
    
  typedef itk::Image<InputPixelType, ImageDimension> InternalImageType;
  typedef itk::Image<InputPixelType, SliceDimension> AxialImageType;

  typedef itk::Vector<RealType,     DataDimension>        VectorType;
  typedef itk::Image<VectorType,    ParametricDimension>  VectorImageType;
  typedef itk::PointSet<VectorType, ParametricDimension>  PointSetType;

  typedef typename PointSetType::PointsContainer          PointsContainer;
  typedef typename PointsContainer::Iterator              PointsIterator;
  typedef typename PointSetType::PointDataContainer       PointDataContainer;
  typedef typename PointDataContainer::Iterator           PointDataIterator;

  typedef itk::ImageRegionIterator< InternalImageType > IteratorType;  
  typedef itk::ImageRegionIteratorWithIndex<InternalImageType> IteratorWithIndexType;
  typedef itk::ImageSliceIteratorWithIndex< InternalImageType > SliceIteratorType;
  typedef itk::ImageLinearIteratorWithIndex< InternalImageType > LineIteratorType;
    

  typedef itk::ImageDuplicator< InternalImageType > DuplicatorType;

  typedef itk::Image<InputPixelType, SliceDimension> InputSliceType;
  typedef itk::BasicImageFeaturesImageFilter< InputSliceType, InputSliceType > BasicImageFeaturesFilterType;

  typedef itk::SliceBySliceImageFilter< InternalImageType, InternalImageType > SliceBySliceImageFilterType;

  typedef itk::RegionGrowSurfacePoints< InternalImageType, InternalImageType > ConnectedSurfaceVoxelFilterType;

  typedef itk::CurvatureAnisotropicDiffusionImageFilter< InternalImageType,
							 InternalImageType > SmoothingFilterType;
    

  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< InternalImageType,
							      InternalImageType > GradientFilterType;

  typedef itk::SigmoidImageFilter<InternalImageType,
				  InternalImageType > SigmoidFilterType;
    
  typedef  itk::FastMarchingImageFilter< InternalImageType,
					 InternalImageType > FastMarchingFilterType;

  typedef itk::BinaryThresholdImageFilter< InternalImageType, 
					   InternalImageType > ThresholdingFilterType;

  typedef itk::LewisGriffinRecursiveGaussianImageFilter < InternalImageType, 
							  InternalImageType > DerivativeFilterType;
  
  typedef typename DerivativeFilterType::Pointer  DerivativeFilterPointer;

  typedef itk::MaximumImageFilter <InternalImageType, InternalImageType>   MaxImageFilterType;
 
  typedef itk::ConnectedThresholdImageFilter< InternalImageType, 
                                              InternalImageType > ConnectedFilterType;

  /// Breast side
  typedef enum {
    BOTH_BREASTS,
    LEFT_BREAST,
    RIGHT_BREAST
  } enumBreastSideType;
  

  void SetVerbose( bool flag ) { flgVerbose = flag; }
  void SetSmooth( bool flag ) { flgSmooth = flag; }

  void SetLeftBreast( bool flag ) { flgLeft = flag; }
  void SetRightBreast( bool flag ) { flgRight = flag; }
  
  void SetExtInitialPect( bool flag ) { flgExtInitialPect = flag; }
  
  void SetRegionGrowX( int coord ) { regGrowXcoord = coord; flgRegGrowXcoord = true; }
  void SetRegionGrowY( int coord ) { regGrowYcoord = coord; flgRegGrowYcoord = true; }
  void SetRegionGrowZ( int coord ) { regGrowZcoord = coord; flgRegGrowZcoord = true; }

  void SetBackgroundThreshold( float threshold ) { bgndThresholdProb = threshold; }
  void SetFinalSegmThreshold( float threshold ) { finalSegmThreshold = threshold; }

  void SetSigmaInMM( float sigma ) { sigmaInMM = sigma; }

  void SetMarchingK1( float k1 ) { fMarchingK1 = k1; }
  void SetMarchingK2( float k2 ) { fMarchingK2 = k2; }
  void SetMarchingTime( float t ) { fMarchingTime = t; }

  void SetCropDistancePosteriorToMidSternum( float fDistIn ) { this->cropDistPosteriorToMidSternum = fDistIn; }
  
  void SetOutputBIFS( std::string fn ) { fileOutputBIFs = fn; }
  void SetSigmaBIF( float sig ){ sigmaBIF = sig; }

  void SetOutputSmoothedStructural( std::string fn ) { fileOutputSmoothedStructural = fn; }
  void SetOutputSmoothedFatSat( std::string fn ) { fileOutputSmoothedFatSat = fn; }
  void SetOutputClosedStructural( std::string fn ) { fileOutputClosedStructural = fn; }
  void SetOutputHistogram( std::string fn ) { fileOutputCombinedHistogram = fn; }
  void SetOutputFit( std::string fn ) { fileOutputRayleigh = fn; }
  void SetOutputCDF( std::string fn ) { fileOutputFreqLessBgndCDF = fn; }
  void SetOutputImageMax( std::string fn ) { fileOutputMaxImage = fn; }
  void SetOutputBackground( std::string fn ) { fileOutputBackground = fn; }
  void SetOutputChestPoints( std::string fn ) { fileOutputChestPoints = fn; }
  void SetOutputPectoralMask( std::string fn ) { fileOutputPectoral = fn; }
  void SetOutputPecSurfaceMask( std::string fn ) { fileOutputPectoralSurfaceMask = fn; }

  void SetOutputGradientMagImage( std::string fn ) { fileOutputGradientMagImage = fn; }
  void SetOutputSpeedImage( std::string fn ) { fileOutputSpeedImage = fn; }
  void SetOutputFastMarchingImage( std::string fn ) { fileOutputFastMarchingImage = fn; }
  
  void SetOutputPectoralSurf( std::string fn ) { fileOutputPectoralSurfaceVoxels = fn; }
  
  void SetCropFit( bool flag ) { flgCropWithFittedSurface = flag; }
  void SetOutputBreastFittedSurfMask( std::string fn ) { fileOutputFittedBreastMask = fn; }

  void SetOutputVTKSurface( std::string fn ) { fileOutputVTKSurface = fn; }

  void SetOutputSkinElevationMap( std::string fn ) { fileOutputSkinElevationMap = fn; }


  void SetStructuralImage( typename InternalImageType::Pointer image ) { imStructural = image; }

  void SetFatSatImage( typename InternalImageType::Pointer image ) { imFatSat = image; }

  void SetBIFImage( typename InternalImageType::Pointer image ) { imBIFs = image; }


  /// Execute the segmentation - must be implemented in derived class
  virtual void Execute( void ) = 0;

  /// Write the segmented image to a file
  void WriteSegmentationToAFile( std::string fileOutput ) {
    WriteBinaryImageToUCharFile( fileOutput, "final segmented image", 
				 imSegmented, flgLeft, flgRight );
  };


  // --------------------------------------------------------------------------
  // Sort pairs in descending order, thus largest elements first
  // --------------------------------------------------------------------------

  template<class T>
    struct larger_second
    : std::binary_function<T,T,bool>
  {
    inline bool operator()(const T &lhs, const T &rhs)
      {
	return lhs.second > rhs.second;
      }
  };


  // --------------------------------------------------------------------------
  // Pixel accessor class to include ascending and descending dark lines
  // from BIF image into region Growing
  // --------------------------------------------------------------------------

  class BIFIntensityAccessor
  {
  public:
    typedef InputPixelType InternalType;
    typedef InputPixelType ExternalType;
    
    static ExternalType Get( const InternalType &in )
      {
	if ( in == 15.0f || in == 16.0f || in == 18.0f ) return 15.0f;
	else return in;
      }
  };


protected:
  
  bool flgVerbose;
  bool flgXML;
  bool flgSmooth;
  bool flgLeft;
  bool flgRight;
  bool flgExtInitialPect;

  bool flgRegGrowXcoord;
  bool flgRegGrowYcoord;
  bool flgRegGrowZcoord;

  bool flgCropWithFittedSurface;

  unsigned int i;

  int regGrowXcoord;
  int regGrowYcoord;
  int regGrowZcoord;

  float maxIntensity;
  float minIntensity;

  float bgndThresholdProb;

  float finalSegmThreshold;

  float sigmaInMM;

  float fMarchingK1;
  float fMarchingK2;
  float fMarchingTime;

  float sigmaBIF;

  float cropDistPosteriorToMidSternum;

  std::string fileOutputBIFs;

  std::string fileOutputSmoothedStructural;
  std::string fileOutputSmoothedFatSat;
  std::string fileOutputClosedStructural;
  std::string fileOutputCombinedHistogram;
  std::string fileOutputRayleigh;
  std::string fileOutputFreqLessBgndCDF;
  std::string fileOutputMaxImage;
  std::string fileOutputBackground;
  std::string fileOutputPectoralSurfaceMask;
  std::string fileOutputChestPoints;
  std::string fileOutputPectoral;

  std::string fileOutputGradientMagImage;
  std::string fileOutputSpeedImage;
  std::string fileOutputFastMarchingImage;

  std::string fileOutputPectoralSurfaceVoxels;

  std::string fileOutputFittedBreastMask;

  std::string fileOutputVTKSurface;

  std::string fileOutputSkinElevationMap;

  typename InternalImageType::Pointer imStructural;
  typename InternalImageType::Pointer imFatSat;
  typename InternalImageType::Pointer imBIFs;

  typename InternalImageType::Pointer imSpeedFuncInputImage; // This will be a copy of the structural image after smoothing
  typename InternalImageType::Pointer imMax;
  typename InternalImageType::Pointer imPectoralVoxels;
  typename InternalImageType::Pointer imPectoralSurfaceVoxels;
  typename InternalImageType::Pointer imChestSurfaceVoxels;
  typename InternalImageType::Pointer imSegmented;

  typename InternalImageType::Pointer imTmp;

  typename AxialImageType::Pointer imSkinElevationMap;

  /// The left side of the image
  typename InternalImageType::RegionType m_LeftLateralRegion;
  typename InternalImageType::RegionType m_RightLateralRegion;


  /// Breast Landmarks

  typename InternalImageType::IndexType idxMidSternum;

  typename InternalImageType::IndexType idxLeftBreastMidPoint;
  typename InternalImageType::IndexType idxRightBreastMidPoint;

  typename InternalImageType::IndexType idxNippleRight;
  typename InternalImageType::IndexType idxNippleLeft;

  typename InternalImageType::IndexType idxLeftPosterior;
  typename InternalImageType::IndexType idxRightPosterior;


  /// Constructor
  BreastMaskSegmentationFromMRI();

  /// Destructor
  virtual ~BreastMaskSegmentationFromMRI();

  /// Initialise the segmentor
  virtual void Initialise( void );

  /// Create the BIF Image
  virtual void CreateBIFs( void );
  
  /// Smooth the structural and FatSat images
  void SmoothTheInputImages( void );

  /// Scan an image in a particular direction and replace voxels with scanned maximum intensity
  typename InternalImageType::Pointer ScanLineMaxima( typename InternalImageType::Pointer image,
                                                      typename InternalImageType::RegionType region,
                                                      unsigned int direction, bool flgForward );
  /// Scan an image in a particular direction and replace voxels with closed intensities
  typename InternalImageType::Pointer GreyScaleCloseImage( typename InternalImageType::Pointer image,
                                                           typename InternalImageType::RegionType region,
                                                           unsigned int direction,
                                                           const std::string label );
  /// Scan an image in all directions and replace voxels with closed intensities
  typename InternalImageType::Pointer GreyScaleCloseImage( typename InternalImageType::Pointer image,
                                                           typename InternalImageType::RegionType region,
                                                           const char *strSide );

  /// Scan rows and columns in the image to fill in the center of the breast
  void GreyScaleClosing( void );

  /// Calculate the maximum image
  void CalculateTheMaximumImage( void );

  /// Ensure the maximum image contains only positive intensities
  void EnsureMaxImageHasOnlyPositiveIntensities( void );

  /// Smooth the image to increase separation of the background
  void SmoothMaxImageToIncreaseSeparationOfTheBackground( void );

  /// Segment the backgound using the maximum image histogram
  void SegmentBackground( void );

  /// Compute a 2D map of the height of the patient's anterior skin surface
  void ComputeElevationOfAnteriorSurface( void );

  /// Find the nipple and mid-sternum landmarks
  void FindBreastLandmarks( void );

  /// Segment the Pectoral Muscle
  typename PointSetType::Pointer SegmentThePectoralMuscle( RealType rYHeightOffset,
							   unsigned long &iPointPec );

  /// Discard anything not within a B-Spline fitted to the breast skin surface
  void MaskWithBSplineBreastSurface( void );
  /// Mask with a sphere centered on each breast
  void MaskBreastWithSphere( void );

  /// Smooth the mask and threshold to round corners etc.
  void SmoothMask( void );

  /// Extract the largest object
  void ExtractLargestObject( enumBreastSideType breastSide );


  double DistanceBetweenVoxels( typename InternalImageType::IndexType p, 
				typename InternalImageType::IndexType q );
  
  std::string ModifySuffix( std::string filename, 
			    std::string strInsertBeforeSuffix );
  
  std::string GetBreastSide( std::string &fileOutput, 
			     enumBreastSideType breastSide );
  
  typename InternalImageType::Pointer GetBreastSide( typename InternalImageType::Pointer inImage, 
						     enumBreastSideType breastSide );
  
  bool WriteImageToFile( std::string &fileOutput, 
			 const char *description,
			 typename InternalImageType::Pointer image, 
			 enumBreastSideType breastSide );
  
  bool WriteImageToFile( std::string &fileOutput,
			 const char *description,
			 typename InternalImageType::Pointer image, 
			 bool flgLeft=false, bool flgRight=false );
  
  bool WriteBinaryImageToUCharFile( std::string &fileOutput, 
				    const char *description,
				    typename InternalImageType::Pointer image, 
				    enumBreastSideType breastSide );
  
  bool WriteBinaryImageToUCharFile( std::string &fileOutput,
				    const char *description,
				    typename InternalImageType::Pointer image, 
				    bool flgLeft, bool flgRight );

  /// Based on the file extension it will be decided wheter a vtk surface is written or an image is saved. 
  bool WriteBinaryImageToUCharFileOrVTKSurfaceFile( std::string &fileOutput,
				    const char *description,
				    typename InternalImageType::Pointer image, 
				    bool flgLeft, bool flgRight );
  
  void WriteHistogramToFile( std::string fileOutput,
			     vnl_vector< double > &xHistIntensity, 
			     vnl_vector< double > &yHistFrequency, 
			     unsigned int nBins );
  
  void polyDataInfo(vtkPolyData *polyData);
  
  void WriteImageToVTKSurfaceFile(typename InternalImageType::Pointer image, 
				  std::string &fileOutput, 
				  enumBreastSideType breastSide,
				  bool flgVerbose, 
				  float finalSegmThreshold ); 
  
  typename InternalImageType::Pointer 
    MaskImageFromBSplineFittedSurface( const typename PointSetType::Pointer            &pointSet, 
				       const typename InternalImageType::RegionType    &region,
				       const typename InternalImageType::PointType     &origin, 
				       const typename InternalImageType::SpacingType   &spacing,
				       const typename InternalImageType::DirectionType &direction,
				       const RealType rOffset, 
				       const int splineOrder, 
				       const int numOfControlPoints,
				       const int numOfLevels,
				       bool correctSurfaceOffest );


private:

  BreastMaskSegmentationFromMRI(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBreastMaskSegmentationFromMRI.txx"
#endif

#endif




 
