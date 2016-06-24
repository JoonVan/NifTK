/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <math.h>
#include <float.h>

#include <niftkConversionUtils.h>
#include <niftkCommandLineParser.h>
#include <itkCommandLineHelper.h>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNifTKImageIOFactory.h>
#include <itkImage.h>

#include <itkOtsuThresholdImageFilter.h>
#include <itkShrinkImageFilter.h>
#include <itkN4BiasFieldCorrectionImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkExpandImageFilter.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkVectorIndexSelectionCastImageFilter.h>
#include <itkExpImageFilter.h>

#include <niftkN4BiasFieldCorrectionCLP.h>



// -------------------------------------------------------------------------
// CommandIterationUpdate
// -------------------------------------------------------------------------

template<class TFilter>
class CommandIterationUpdate : public itk::Command
{
public:
  typedef CommandIterationUpdate  Self;
  typedef itk::Command            Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {}

public:

  void Execute(itk::Object *caller, const itk::EventObject & event) override
    {
    Execute( (const itk::Object *) caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event) override
    {
    const TFilter * filter =
      dynamic_cast< const TFilter * >( object );

    if( typeid( event ) != typeid( itk::IterationEvent ) )
                                        { return; }
    if( filter->GetElapsedIterations() == 1 )
      {
      std::cout << "Current level = " << filter->GetCurrentLevel() + 1
                << std::endl;
      }
    std::cout << "  Iteration " << filter->GetElapsedIterations()
              << " (of "
              << filter->GetMaximumNumberOfIterations()[
      filter->GetCurrentLevel()]
              << ").  ";
    std::cout << " Current convergence value = "
              << filter->GetCurrentConvergenceMeasurement()
              << " (threshold = " << filter->GetConvergenceThreshold()
              << ")" << std::endl;
    }

};


// -------------------------------------------------------------------------
// Arguments
// -------------------------------------------------------------------------

class Arguments
{
public:
  float subsampling;
  float splineOrder;
  float nHistogramBins;
  float WeinerFilterNoise;
  float BiasFieldFullWidthAtHalfMaximum;
  float MaximumNumberOfIterations;
  float ConvergenceThreshold;
  float NumberOfFittingLevels;
  float NumberOfControlPoints;

  std::string fileInputImage;
  std::string fileInputMask;
  std::string fileOutputBiasField;
  std::string fileOutputImage;

  std::string fileOutputMask;
  std::string fileOutputSubsampledImage;
  std::string fileOutputSubsampledMask;

  Arguments() {

    subsampling = 4.;
    splineOrder = 3;
    nHistogramBins = 200;
    WeinerFilterNoise = 0.01;
    BiasFieldFullWidthAtHalfMaximum = 0.15;
    MaximumNumberOfIterations = 50;
    ConvergenceThreshold = 0.001;
    NumberOfFittingLevels = 4;
    NumberOfControlPoints = 0;
  }

  void Print(void) {
    std::cout << std::endl;

    std::cout << "ARG: Sub-sampling factor: " << subsampling << std::endl;
    std::cout << "ARG: Spline order: " << splineOrder << std::endl;
    std::cout << "ARG: No. of histogram bins: " << nHistogramBins << std::endl;
    std::cout << "ARG: Weinder filter noise level: " << WeinerFilterNoise << std::endl;
    std::cout << "ARG: Bias field FWHM: " << BiasFieldFullWidthAtHalfMaximum << std::endl;
    std::cout << "ARG: Max no. of iterations: " << MaximumNumberOfIterations << std::endl;
    std::cout << "ARG: Convergence threshold: " << ConvergenceThreshold << std::endl;
    std::cout << "ARG: No. of fitting levels: " << NumberOfFittingLevels << std::endl;
    std::cout << "ARG: No of control points: " << NumberOfControlPoints << std::endl;
    
    std::cout << "ARG: Input image file: " << fileInputImage << std::endl;
    std::cout << "ARG: Input mask file: " << fileInputMask << std::endl;
    std::cout << "ARG: Output bias field: " << fileOutputBiasField << std::endl;
    std::cout << "ARG: Output corrected image: " << fileOutputImage << std::endl;
    
    std::cout << "ARG: Output mask file: " << fileOutputMask << std::endl;
    std::cout << "ARG: Output subsampled image: " << fileOutputSubsampledImage << std::endl;
    std::cout << "ARG: Output sub-sampled mask: " << fileOutputSubsampledMask << std::endl;
    std::cout << std::endl;
  }
};


// --------------------------------------------------------------------------
// WriteImageToFile()
// --------------------------------------------------------------------------

template <class ImageType>
bool WriteImageToFile( std::string &fileOutput, const char *description,
                       typename ImageType::Pointer image )
{
  if ( fileOutput.length() ) {

    std::string fileModifiedOutput;

    typedef itk::ImageFileWriter< ImageType > FileWriterType;

    typename FileWriterType::Pointer writer = FileWriterType::New();

    writer->SetFileName( fileOutput );
    writer->SetInput( image );

    std::cout << "Writing " << description << " to file: "
	      << fileOutput << std::endl;
    writer->Update();

    return true;
  }
  else
    return false;
}


// -------------------------------------------------------------------------
// DoMain(Arguments args)
// -------------------------------------------------------------------------

template <int Dimension, class OutputPixelType>
int DoMain(Arguments &args)
{
  float progress = 0.;
  float iStep = 0.;
  float nSteps = 13;

  double factor[3];


  // Read the input image
  // ~~~~~~~~~~~~~~~~~~~~

  typedef float InputPixelType;
  typedef itk::Image<InputPixelType, Dimension> InputImageType;

  typedef itk::ImageFileReader< InputImageType > FileReaderType;

  typename FileReaderType::Pointer imageReader = FileReaderType::New();

  imageReader->SetFileName( args.fileInputImage );
  

  try
  { 
    std::cout << "Reading the input image: " << args.fileInputImage << std::endl;
    imageReader->Update();
  }
  catch (itk::ExceptionObject &ex)
  { 
    std::cout << ex << std::endl;
    return EXIT_FAILURE;
  }

  typename InputImageType::Pointer imOriginal = imageReader->GetOutput();
  imOriginal->SetRegions( imageReader->GetOutput()->GetLargestPossibleRegion() ) ;
  imOriginal->SetOrigin(imageReader->GetOutput()->GetOrigin());
  imOriginal->SetSpacing(imageReader->GetOutput()->GetSpacing());
  imOriginal->SetDirection(imageReader->GetOutput()->GetDirection());
  imOriginal->DisconnectPipeline();

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;

  // Create a mask by thresholding
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  typedef unsigned char MaskPixelType;
  typedef itk::Image<MaskPixelType, Dimension> MaskImageType;
  typedef itk::ImageFileReader< MaskImageType > MaskFileReaderType;
  typename MaskFileReaderType::Pointer maskReader = MaskFileReaderType::New();
  typedef itk::OtsuThresholdImageFilter< InputImageType,
                                         MaskImageType > OtsuThresholdImageFilterType;

  typename OtsuThresholdImageFilterType::Pointer thresholder = OtsuThresholdImageFilterType::New();

  if ( args.fileInputMask.length() == 0) {
      thresholder->SetInput( imOriginal );

      thresholder->SetInsideValue( 0 );
      thresholder->SetOutsideValue( 1 );

      thresholder->SetNumberOfHistogramBins( 200 );

      try
      {
        std::cout << "Thresholding to obtain image mask" << std::endl;
        thresholder->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        std::cerr << e << std::endl;
      }

      progress = ++iStep/nSteps;
      std::cout << "<filter-progress>" << std::endl
            << progress << std::endl
            << "</filter-progress>" << std::endl;

      // Write the mask image to a file?
      // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      if ( args.fileOutputMask.length() != 0 )
      {
        WriteImageToFile< MaskImageType >( args.fileOutputMask, "mask image",
                                           thresholder->GetOutput() );
      }
    }
    else {
      try
      {
        std::cout << "Reading input image mask" << std::endl;
        maskReader->SetFileName( args.fileInputMask );
        maskReader->Update();
      }
      catch (itk::ExceptionObject &e)
      {
        std::cerr << e << std::endl;
      }

      if ( args.fileOutputMask.length() != 0 )
      {
        WriteImageToFile< MaskImageType >( args.fileOutputMask, "mask image",
                                           maskReader->GetOutput() );
      }

    }
  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
        << progress << std::endl
        << "</filter-progress>" << std::endl;

  // Shrink the image and the mask
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  typedef itk::ShrinkImageFilter <InputImageType, InputImageType> ShrinkImageFilterType;
 
  typename ShrinkImageFilterType::Pointer shrinkFilter = ShrinkImageFilterType::New();

  shrinkFilter->SetInput( imOriginal );

  shrinkFilter->SetShrinkFactor(0, args.subsampling);
  shrinkFilter->SetShrinkFactor(1, args.subsampling);
  shrinkFilter->SetShrinkFactor(2, args.subsampling);

  try
  {
    std::cout << "Shrinking the image by a factor of " << args.subsampling << std::endl;
    shrinkFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;
 
  if ( args.fileOutputSubsampledImage.length() != 0 ) 
  {
    WriteImageToFile< InputImageType >( args.fileOutputSubsampledImage, 
                                        "subsampled input image",
                                        shrinkFilter->GetOutput() );
  }

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;

  typedef itk::ShrinkImageFilter <MaskImageType, MaskImageType> ShrinkMaskFilterType;
 
  typename ShrinkMaskFilterType::Pointer maskShrinkFilter = ShrinkMaskFilterType::New();

  if ( args.fileInputMask.length() == 0) {
    maskShrinkFilter->SetInput( thresholder->GetOutput() );
  }
  else {
    maskShrinkFilter->SetInput( maskReader->GetOutput() );
  }

  maskShrinkFilter->SetShrinkFactor(0, args.subsampling);
  maskShrinkFilter->SetShrinkFactor(1, args.subsampling);
  maskShrinkFilter->SetShrinkFactor(2, args.subsampling);

  try
  {
    std::cout << "Shrinking the mask by a factor of " << args.subsampling << std::endl;
    maskShrinkFilter->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;
 
  if ( args.fileOutputSubsampledMask.length() != 0 ) 
  {
    WriteImageToFile< MaskImageType >( args.fileOutputSubsampledMask,
                                       "subsampled mask image",
                                       maskShrinkFilter->GetOutput() );
  }
  
  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;


  // Compute the N4 Bias Field Correction
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  typedef itk::N4BiasFieldCorrectionImageFilter <InputImageType, 
                                                 MaskImageType,
                                                 InputImageType> N4BiasFieldCorrectionImageFilterType;
 
  typename N4BiasFieldCorrectionImageFilterType::Pointer 
    biasFieldFilter = N4BiasFieldCorrectionImageFilterType::New();

  biasFieldFilter->SetInput( shrinkFilter->GetOutput() );

  biasFieldFilter->SetMaskImage( maskShrinkFilter->GetOutput() );

  typedef CommandIterationUpdate< N4BiasFieldCorrectionImageFilterType > CommandType;
  typename CommandType::Pointer observer = CommandType::New();
  biasFieldFilter->AddObserver( itk::IterationEvent(), observer );

  biasFieldFilter->SetMaskLabel( 1 );

  biasFieldFilter->SetNumberOfHistogramBins( args.nHistogramBins );
  biasFieldFilter->SetSplineOrder( args.splineOrder );
  biasFieldFilter->SetWienerFilterNoise( args.WeinerFilterNoise );
  biasFieldFilter->SetBiasFieldFullWidthAtHalfMaximum( args.BiasFieldFullWidthAtHalfMaximum );
  biasFieldFilter->SetConvergenceThreshold( args.ConvergenceThreshold );

  // handle the number of iterations
  std::vector<unsigned int> numIters;
  numIters.resize( args.NumberOfFittingLevels, args.MaximumNumberOfIterations );

  typename N4BiasFieldCorrectionImageFilterType::VariableSizeArrayType
  maximumNumberOfIterations( numIters.size() );
  for( unsigned int d = 0; d < numIters.size(); d++ )
  {
    maximumNumberOfIterations[d] = numIters[d];
  }
  biasFieldFilter->SetMaximumNumberOfIterations( maximumNumberOfIterations );

  typename N4BiasFieldCorrectionImageFilterType::ArrayType numberOfFittingLevels;
  numberOfFittingLevels.Fill( numIters.size() );
  biasFieldFilter->SetNumberOfFittingLevels( numberOfFittingLevels );

  if ( args.NumberOfControlPoints )
  {
    typename N4BiasFieldCorrectionImageFilterType::ArrayType numberOfControlPoints;
    numberOfControlPoints.Fill( args.NumberOfControlPoints );
    biasFieldFilter->SetNumberOfControlPoints( numberOfControlPoints );
  }

  try
  {
    std::cout << "Computing the bias field" << std::endl;
    biasFieldFilter->UpdateLargestPossibleRegion();
    //biasFieldFilter->Print( std::cout );
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;


  // Reconstruction of the bias field
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Compute the log bias field

  typedef itk::BSplineControlPointImageFilter
    < typename N4BiasFieldCorrectionImageFilterType::BiasFieldControlPointLatticeType, 
      typename N4BiasFieldCorrectionImageFilterType::ScalarImageType > BSplinerType;

  typename BSplinerType::Pointer bspliner = BSplinerType::New();

  bspliner->SetInput( biasFieldFilter->GetLogBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( biasFieldFilter->GetSplineOrder() );
  bspliner->SetSize( imOriginal->GetLargestPossibleRegion().GetSize() );
  bspliner->SetOrigin( imOriginal->GetOrigin() );
  bspliner->SetDirection( imOriginal->GetDirection() );
  bspliner->SetSpacing( imOriginal->GetSpacing() );

  try
  {
    std::cout << "Computing the log bias field" << std::endl;
    bspliner->UpdateLargestPossibleRegion();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;

  // And then the exponential

  typedef itk::VectorIndexSelectionCastImageFilter
    < typename N4BiasFieldCorrectionImageFilterType::ScalarImageType, 
    InputImageType > CastImageFilterType;

  typename CastImageFilterType::Pointer caster = CastImageFilterType::New();

  caster->SetInput( bspliner->GetOutput() );
  caster->SetIndex(0);


  typedef itk::ExpImageFilter
    < InputImageType, 
      InputImageType > ExpImageFilterType;

  typename ExpImageFilterType::Pointer expFilter = ExpImageFilterType::New();

  expFilter->SetInput( caster->GetOutput() );

  try
  {
    std::cout << "Computing the exponential of the bias field" << std::endl;
    expFilter->UpdateLargestPossibleRegion();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;


  // Write the bias field image to a file?
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if ( args.fileOutputBiasField.length() != 0 ) 
  {
    WriteImageToFile< InputImageType >( args.fileOutputBiasField, "bias field image",
                                        expFilter->GetOutput() );
  }
  
  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;


  // Correct the original input image
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  typename InputImageType::Pointer imBiasField = expFilter->GetOutput();

  typename itk::ImageRegionIterator< InputImageType > 
    iterOriginal( imOriginal, imOriginal->GetLargestPossibleRegion() );

  typename itk::ImageRegionConstIterator< InputImageType > 
    iterBiasField( imBiasField, imBiasField->GetLargestPossibleRegion() );
        
  for ( iterBiasField.GoToBegin(), iterOriginal.GoToBegin();
        ! iterBiasField.IsAtEnd();
        ++iterBiasField, ++iterOriginal)
  {
    if ( iterBiasField.Get() )
    {
      iterOriginal.Set( iterOriginal.Get() / iterBiasField.Get() );
    }
  }    

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;
 

  // Write the bias field corrected image to a file?
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if ( args.fileOutputImage.length() != 0 ) 
  {
    WriteImageToFile< InputImageType >( args.fileOutputImage, "bias field corrected image",
                                        imOriginal );
  }

  progress = ++iStep/nSteps;
  std::cout << "<filter-progress>" << std::endl
	    << progress << std::endl
	    << "</filter-progress>" << std::endl;

  return EXIT_SUCCESS;
}


// -------------------------------------------------------------------------
// main( int argc, char *argv[] )
// -------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
  itk::NifTKImageIOFactory::Initialize();

  // To pass around command line args
  Arguments args;

  // Validate command line args
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~

  PARSE_ARGS;

  args.subsampling                     = subsampling;
  args.splineOrder                     = splineOrder;                       
  args.nHistogramBins                  = nHistogramBins;                    
  args.WeinerFilterNoise               = WeinerFilterNoise;                 
  args.BiasFieldFullWidthAtHalfMaximum = BiasFieldFullWidthAtHalfMaximum;   
  args.MaximumNumberOfIterations       = MaximumNumberOfIterations;         
  args.ConvergenceThreshold            = ConvergenceThreshold;              
  args.NumberOfFittingLevels           = NumberOfFittingLevels;             
  args.NumberOfControlPoints           = NumberOfControlPoints;                

  args.fileInputImage      = fileInputImage;
  args.fileInputMask       = fileInputMask;
  args.fileOutputBiasField = fileOutputBiasField;
  args.fileOutputImage     = fileOutputImage;
  
  args.fileOutputMask            = fileOutputMask;
  args.fileOutputSubsampledImage = fileOutputSubsampledImage;
  args.fileOutputSubsampledMask  = fileOutputSubsampledMask;

  args.Print();

  if ( args.fileInputImage.length() == 0 || args.fileOutputImage.length() == 0 )
  {
    commandLine.getOutput()->usage(commandLine);
    return EXIT_FAILURE;
  }

  int dims = itk::PeekAtImageDimensionFromSizeInVoxels(args.fileInputImage);
  if (dims != 3)
  {
    std::cout << "ERROR: Unsupported image dimension, image must be 3D" << std::endl;
    return EXIT_FAILURE;
  }
  
  int result;

  switch (itk::PeekAtComponentType(args.fileInputImage))
  {
  case itk::ImageIOBase::UCHAR:
    result = DoMain<3, unsigned char>(args);
    break;

  case itk::ImageIOBase::CHAR:
    result = DoMain<3, char>(args);
    break;

  case itk::ImageIOBase::USHORT:
    result = DoMain<3, unsigned short>(args);
    break;

  case itk::ImageIOBase::SHORT:
    result = DoMain<3, short>(args);
    break;

  case itk::ImageIOBase::UINT:
    result = DoMain<3, unsigned int>(args);
    break;

  case itk::ImageIOBase::INT:
    result = DoMain<3, int>(args);
    break;

  case itk::ImageIOBase::ULONG:
    result = DoMain<3, unsigned long>(args);
    break;

  case itk::ImageIOBase::LONG:
    result = DoMain<3, long>(args);
    break;

  case itk::ImageIOBase::FLOAT:
    result = DoMain<3, float>(args);
    break;

  case itk::ImageIOBase::DOUBLE:
    result = DoMain<3, double>(args);
    break;

  default:
    std::cerr << "ERROR: Unsupported pixel format" << std::endl;
    return EXIT_FAILURE;
  }
  return result;
}
