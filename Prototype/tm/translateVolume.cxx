#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
//#include "itkCenteredRigid2DTransform.h"
#include "itkAffineTransform.h"
#include "itkResampleImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"


int main( int argc, char * argv[] )
{
 if( argc < 6 ) 
 { 
   std::cerr << "Usage: " << std::endl;
   std::cerr << argv[0] << "  inputImageFile  outputImageFile translationInMM-X translationInMM-Y translationInMM-Z ";
   std::cerr <<  std::endl;
   return EXIT_FAILURE;
 }
 // imput and output decl  
 const int dimension = 3;
 typedef    short    InputPixelType; //float
 typedef    short    OutputPixelType; //double
  
 typedef itk::Image< InputPixelType,  dimension >   InputImageType;
 typedef itk::Image< OutputPixelType, dimension  >   OutputImageType; 
 
 // reader and writer for the input and output images
 typedef itk::ImageFileReader< InputImageType >  ReaderType;
 typedef itk::ImageFileWriter< OutputImageType >  WriterType;

 ReaderType::Pointer reader = ReaderType::New();
 WriterType::Pointer writer = WriterType::New();
 reader->SetFileName( argv[1] );
 writer->SetFileName( argv[2] );
 
 // defs of transorm and filter
 typedef itk::ResampleImageFilter< InputImageType, OutputImageType > FilterType;
 FilterType::Pointer filter = FilterType::New();

 typedef itk::AffineTransform< double, dimension > TransformType;
 TransformType::Pointer transform = TransformType::New();

 typedef itk::LinearInterpolateImageFunction< InputImageType, double > InterpolatorType;

 InterpolatorType::Pointer interpolator = InterpolatorType::New();
 filter->SetInterpolator( interpolator );
 
 // set the filter options
 filter->SetDefaultPixelValue( 0 ); // value of pixles mapped outside the image
 
 reader->Update(); // update to take the values of reader
 const InputImageType::SpacingType & spacing = reader->GetOutput()->GetSpacing(); 
 const InputImageType::PointType & origin = reader->GetOutput()->GetOrigin();

 InputImageType::SizeType size = reader->GetOutput()->GetLargestPossibleRegion().GetSize();

 std::cerr << "Spacing: " << spacing[0] << " "<< spacing[1] << " "<< spacing[2] << std::endl;
 std::cerr << "Size: " << size[0] << " "<< size[1] << " "<< size[2] << std::endl;
 std::cerr << "Origin: " << origin[0] << " "<< origin[1] << " "<< origin[2] << std::endl;
 
 filter->SetOutputSpacing( spacing );
 filter->SetOutputOrigin( origin );

 // what is the origin?
 std::cerr << "Origin: " << origin[0] << " "<< origin[1] << " "<< origin[2] << std::endl;

 filter->SetSize( size );

 // set the new size
 InputImageType::SizeType newSize;
 newSize[0] = size[0] ;//+ 30; //501;//160;
 newSize[1] = size[1]; //501;//80;
 newSize[2] = size[2] ;//+ 30; //1;//180;
 filter->SetSize( newSize );
 std::cerr << "New Size: " << newSize[0] << " "<< newSize[1] << " "<< newSize[2] << std::endl;
 
 // transform build
 TransformType::OutputVectorType translation;
  
 translation[0] = atof( argv[3] );//0;//-( ( ((float)newSize[0]-1)/2 ) - ( (size[0]-1)/2 ) )*spacing[0];
 translation[1] = atof( argv[4] );//80;//-( ( ((float)newSize[1]-1)/2 ) - ( size[1] )  )*spacing[1];
 translation[2] = atof( argv[5] );//0;//-4*spacing[2];
 transform->Translate( translation );

 filter->SetTransform( transform );
 

 filter->SetInput( reader->GetOutput() );
 writer->SetInput( filter->GetOutput() );
 //writer->Update();

 try
 {
	writer->Update();
 }
 catch( itk::ExceptionObject & excep )
 {
	std::cerr << "Exception caught !" << std::endl;
	std::cerr << excep << std::endl;
 }
 
 
}
