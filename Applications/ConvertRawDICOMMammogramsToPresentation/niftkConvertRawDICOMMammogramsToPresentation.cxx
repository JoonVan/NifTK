/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

  =============================================================================*/

/*!
 * \file niftkConvertRawDICOMMammogramsToPresentation.cxx 
 * \page niftkConvertRawDICOMMammogramsToPresentation
 * \section niftkConvertRawDICOMMammogramsToPresentationSummary niftkConvertRawDICOMMammogramsToPresentation
 * 
 * Search for raw "For Processing" DICOM mammograms in a directory and convert them to "For Presentation" versions by calculating the logarithm of their intensities and then the inverse.
 *
 */


#include <niftkFileHelper.h>
#include <niftkConversionUtils.h>
#include <itkCommandLineHelper.h>

#include <itkCreatePositiveMammogram.h>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNifTKImageIOFactory.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkGDCMImageIO.h>
#include <itkCastImageFilter.h>
#include <itkMammogramFatSubtractionImageFilter.h>
#include <itkMammogramMaskSegmentationImageFilter.h>

#include <boost/filesystem/path.hpp>

#include <vector>

#include <niftkConvertRawDICOMMammogramsToPresentationCLP.h>


namespace fs = boost::filesystem;

typedef itk::MetaDataDictionary DictionaryType;
typedef itk::MetaDataObject< std::string > MetaDataStringType;



struct arguments
{
  std::string dcmDirectoryIn;
  std::string dcmDirectoryOut;
  std::string strAdd2Suffix;  

  bool flgOverwrite;
  bool flgRescaleIntensitiesToMaxRange;
  bool flgVerbose;
  bool flgFatSubtract;

  std::string iterFilename;
};


// -------------------------------------------------------------------------
// PrintDictionary()
// -------------------------------------------------------------------------

void PrintDictionary( DictionaryType &dictionary )
{
  DictionaryType::ConstIterator tagItr = dictionary.Begin();
  DictionaryType::ConstIterator end = dictionary.End();
   
  while ( tagItr != end )
  {
    MetaDataStringType::ConstPointer entryvalue = 
      dynamic_cast<const MetaDataStringType *>( tagItr->second.GetPointer() );
    
    if ( entryvalue )
    {
      std::string tagkey = tagItr->first;
      std::string tagID;
      bool found =  itk::GDCMImageIO::GetLabelFromTag( tagkey, tagID );

      std::string tagValue = entryvalue->GetMetaDataObjectValue();
      
      std::cout << tagkey << " " << tagID <<  ": " << tagValue << std::endl;
    }

    ++tagItr;
  }
};


// -------------------------------------------------------------------------
// AddPresentationFileSuffix
// -------------------------------------------------------------------------

std::string AddPresentationFileSuffix( std::string fileName, std::string strAdd2Suffix )
{
  std::string suffix;
  std::string newSuffix;

  if ( ( fileName.length() >= 4 ) && 
       ( fileName.substr( fileName.length() - 4 ) == std::string( ".dcm" ) ) )
  {
    suffix = std::string( ".dcm" );
  }

  else if ( ( fileName.length() >= 4 ) && 
            ( fileName.substr( fileName.length() - 4 ) == std::string( ".DCM" ) ) )
  {
    suffix = std::string( ".DCM" );
  }

  else if ( ( fileName.length() >= 6 ) && 
            ( fileName.substr( fileName.length() - 6 ) == std::string( ".dicom" ) ) )
  {
    suffix = std::string( ".dicom" );
  }

  else if ( ( fileName.length() >= 6 ) && 
            ( fileName.substr( fileName.length() - 6 ) == std::string( ".DICOM" ) ) )
  {
    suffix = std::string( ".DICOM" );
  }

  else if ( ( fileName.length() >= 4 ) && 
            ( fileName.substr( fileName.length() - 4 ) == std::string( ".IMA" ) ) )
  {
    suffix = std::string( ".IMA" );
  }

  std::cout << "Suffix: '" << suffix << "'" << std::endl;

  newSuffix = strAdd2Suffix + suffix;

  if ( ( fileName.length() >= newSuffix.length() ) && 
       ( fileName.substr( fileName.length() - newSuffix.length() ) != newSuffix ) )
  {
    return fileName.substr( 0, fileName.length() - suffix.length() ) + newSuffix;
  }
  else
  {
    return fileName;
  }
};


// -------------------------------------------------------------------------
// main()
// -------------------------------------------------------------------------

template <class OutputPixelType>
int DoMain(arguments args, OutputPixelType min, OutputPixelType max)
{
  float progress = 0.;
  float iFile = 0.;
  float nFiles;
 

  // Get the list of files in the directory
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  std::vector< std::string > fileNames;
  niftk::GetRecursiveFilesInDirectory( args.dcmDirectoryIn, fileNames );

  nFiles = fileNames.size();


  // Iterate through each file and convert it
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  std::string fileInputFullPath;
  std::string fileInputRelativePath;
  std::string fileOutputRelativePath;
  std::string fileOutputFullPath;
  std::string dirOutputFullPath;
    
  std::vector< std::string >::iterator iterFileNames;       

  typedef float InternalPixelType;

  const unsigned int   InputDimension = 2;

  typedef itk::Image< InternalPixelType, InputDimension > InternalImageType; 
  typedef itk::Image< OutputPixelType, InputDimension > OutputImageType;

  typedef itk::RescaleIntensityImageFilter< InternalImageType, InternalImageType > RescalerType;
  typedef itk::CastImageFilter< InternalImageType, OutputImageType > CastingFilterType;
 
  typedef itk::ImageFileReader< InternalImageType > ReaderType;
  typedef itk::ImageFileWriter< OutputImageType > WriterType;


  ReaderType::Pointer reader = ReaderType::New();
  InternalImageType::Pointer image;

  typedef itk::GDCMImageIO           ImageIOType;
  ImageIOType::Pointer gdcmImageIO = ImageIOType::New();

  progress = iFile/nFiles;
  std::cout << "<filter-progress>" << std::endl
            << progress << std::endl
            << "</filter-progress>" << std::endl;

  // Read the image

  reader->SetImageIO( gdcmImageIO );
  reader->SetFileName( args.iterFilename );
    
  try
  {
    reader->UpdateLargestPossibleRegion();
  }

  catch (itk::ExceptionObject &ex)
  {
    std::cout << "Skipping file (not DICOM?): " << args.iterFilename << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "File: " << args.iterFilename << std::endl;

  image = reader->GetOutput();
  image->DisconnectPipeline();

  DictionaryType dictionary = image->GetMetaDataDictionary();
  

  // Convert a raw DICOM mammogram to a presentation version by log inverting it

  if ( ! itk::ConvertMammogramFromRawToPresentation< InternalImageType >( image, 
                                                                          dictionary ) )
  {
    return EXIT_FAILURE;
  }



  // Fat subtract the image?

  if ( args.flgFatSubtract )
  {
    // First compute the mask

    typedef unsigned char MaskPixelType;
    typedef itk::Image< MaskPixelType, InputDimension > MaskImageType;   

    typedef itk::MammogramMaskSegmentationImageFilter<InternalImageType, MaskImageType> 
      MammogramMaskSegmentationImageFilterType;

    typename MammogramMaskSegmentationImageFilterType::Pointer 
      maskFilter = MammogramMaskSegmentationImageFilterType::New();

    maskFilter->SetInput( image );

    maskFilter->SetVerbose( args.flgVerbose );

    maskFilter->SetIncludeBorderRegion( true );

    try {
      maskFilter->Update();
    }
    catch( itk::ExceptionObject & err ) 
    { 
      std::cerr << "ERROR: Failed to segment image" << std::endl
                << err << std::endl; 
      return EXIT_FAILURE;
    }                

    typename MaskImageType::Pointer mask = maskFilter->GetOutput();

    mask->DisconnectPipeline();
    
    // Then calculate the fat subtracted image

    typedef itk::MammogramFatSubtractionImageFilter<InternalImageType> 
      MammogramFatSubtractionImageFilterType;

    typename MammogramFatSubtractionImageFilterType::Pointer 
      fatFilter = MammogramFatSubtractionImageFilterType::New();

    fatFilter->SetInput( image );  

    fatFilter->SetVerbose( args.flgVerbose );

    fatFilter->SetComputeFatEstimationFit( true );

    fatFilter->SetMask( mask );
  
    try
    {
      fatFilter->Update(); 
    }
    catch( itk::ExceptionObject & err ) 
    { 
      std::cerr << "ERROR: Failed to calculate the fat subtraction: " << err << std::endl; 
      return EXIT_FAILURE;
    }                
    
    image = fatFilter->GetOutput( 0 );
    image->DisconnectPipeline();
  }


  // Rescale the image to the maximum range

  if ( args.flgRescaleIntensitiesToMaxRange )
  {
    itksys_ios::ostringstream value;

    typename RescalerType::Pointer intensityRescaler = RescalerType::New();
  
    intensityRescaler->SetOutputMinimum( min );
    intensityRescaler->SetOutputMaximum( max );

    // Set the pixel intensity relationship sign to linear
    value.str("");
    value << "LIN";
    itk::EncapsulateMetaData<std::string>(dictionary,"0028|1040", value.str());

    // Set the pixel intensity relationship sign to one
    value.str("");
    value << 1;
    itk::EncapsulateMetaData<std::string>(dictionary,"0028|1041", value.str());

    // Set the new window centre tag value
    value.str("");
    value << max / 2;
    itk::EncapsulateMetaData<std::string>(dictionary,"0028|1050", value.str());

    // Set the new window width tag value
    value.str("");
    value << max;
    itk::EncapsulateMetaData<std::string>(dictionary,"0028|1051", value.str());

    // Set the rescale intercept and slope to zero and one 
    value.str("");
    value << 0;
    itk::EncapsulateMetaData<std::string>(dictionary, "0028|1052", value.str());
    value.str("");
    value << 1;
    itk::EncapsulateMetaData<std::string>(dictionary, "0028|1053", value.str());

    intensityRescaler->UpdateLargestPossibleRegion();

    image = intensityRescaler->GetOutput();
    image->DisconnectPipeline();
  }


  // Cast to the output type

  typename CastingFilterType::Pointer caster = CastingFilterType::New();

  caster->SetInput( image );

  caster->UpdateLargestPossibleRegion();


  // Create the output image filename

  fileInputFullPath = args.iterFilename;

  fileInputRelativePath = fileInputFullPath.substr( args.dcmDirectoryIn.length() );
     
  fileOutputRelativePath = AddPresentationFileSuffix( fileInputRelativePath,
                                                      args.strAdd2Suffix );
    
  fileOutputFullPath = niftk::ConcatenatePath( args.dcmDirectoryOut, 
                                               fileOutputRelativePath );

  dirOutputFullPath = fs::path( fileOutputFullPath ).branch_path().string();
    
  if ( ! niftk::DirectoryExists( dirOutputFullPath ) )
  {
    niftk::CreateDirAndParents( dirOutputFullPath );
  }
      
  std::cout << "Input relative filename: " << fileInputRelativePath << std::endl
            << "Output relative filename: " << fileOutputRelativePath << std::endl
            << "Output directory: " << dirOutputFullPath << std::endl;


  // Write the image to the output file

  if ( niftk::FileExists( fileOutputFullPath ) && ( ! args.flgOverwrite ) )
  {
    std::cerr << std::endl << "ERROR: File " << fileOutputFullPath << " exists"
              << std::endl << "       and can't be overwritten. Consider option: 'overwrite'."
              << std::endl << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
  
    if ( args.flgVerbose )
    {
      PrintDictionary( dictionary );
    }

    typename WriterType::Pointer writer = WriterType::New();

    typename OutputImageType::Pointer outImage = caster->GetOutput();

    writer->SetFileName( fileOutputFullPath );

    outImage->DisconnectPipeline();
    writer->SetInput( outImage );

    gdcmImageIO->SetMetaDataDictionary( dictionary );
    gdcmImageIO->KeepOriginalUIDOn( );
    writer->SetImageIO( gdcmImageIO );

    writer->UseInputMetaDataDictionaryOff();

    try
    {
      std::cout << "Writing image to file: " << fileOutputFullPath << std::endl;
      writer->Update();
    }
    catch (itk::ExceptionObject & e)
    {
      std::cerr << "ERROR: Failed to write image: " << std::endl << e << std::endl;
      return EXIT_FAILURE;
    }
  }

  std::cout << std::endl;


  return EXIT_SUCCESS;
}



// -------------------------------------------------------------------------
// main()
// -------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
  itk::NifTKImageIOFactory::Initialize();

  float progress = 0.;
  float iFile = 0.;
  float nFiles;

  struct arguments args;

  // Validate command line args
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~

  PARSE_ARGS;

  if ( dcmDirectoryIn.length() == 0 )
  {
    commandLine.getOutput()->usage(commandLine);
    std::cerr << "ERROR: The input directory must be specified" << std::endl;
    return EXIT_FAILURE;
  }

  if ( dcmDirectoryOut.length() == 0 )
  {
    dcmDirectoryOut = dcmDirectoryIn;
  }

  args.dcmDirectoryIn  = dcmDirectoryIn;                     
  args.dcmDirectoryOut = dcmDirectoryOut;                    

  args.strAdd2Suffix = strAdd2Suffix;                      
				   	                                                 
  args.flgOverwrite   = flgOverwrite;                       
  args.flgVerbose     = flgVerbose;    
  args.flgFatSubtract = flgFatSubtract;

  args.flgRescaleIntensitiesToMaxRange = flgRescaleIntensitiesToMaxRange;


  std::cout << std::endl << "Examining directory: " 
	    << args.dcmDirectoryIn << std::endl << std::endl;


  // Get the list of files in the directory
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  std::vector< std::string > fileNames;
  std::vector< std::string >::iterator iterFileNames;       

  niftk::GetRecursiveFilesInDirectory( dcmDirectoryIn, fileNames );

  nFiles = fileNames.size();

  for ( iterFileNames = fileNames.begin(); 
	iterFileNames < fileNames.end(); 
	++iterFileNames, iFile += 1. )
  {
    args.iterFilename = *iterFileNames;
    
    std::cout << "File: " << args.iterFilename << std::endl;

    progress = iFile/nFiles;
    std::cout << "<filter-progress>" << std::endl
	      << progress << std::endl
	      << "</filter-progress>" << std::endl;

  
    itk::ImageIOBase::Pointer imageIO;
    imageIO = itk::ImageIOFactory::CreateImageIO(args.iterFilename.c_str(), 
						 itk::ImageIOFactory::ReadMode);

    if ( ( ! imageIO ) || ( ! imageIO->CanReadFile( args.iterFilename.c_str() ) ) )
    {
      std::cerr << "WARNING: Unrecognised image type, skipping file: " 
		<< args.iterFilename << std::endl;
      continue;
    }


    int result;

    switch (itk::PeekAtComponentType(args.iterFilename))
    {
    case itk::ImageIOBase::UCHAR:
      result = DoMain<unsigned char>( args,
                                      itk::NumericTraits<unsigned char>::ZeroValue(),
                                      itk::NumericTraits<unsigned char>::max() );  
      break;
    
    case itk::ImageIOBase::CHAR:
      result = DoMain<char>( args,
                             itk::NumericTraits<char>::ZeroValue(),
                             itk::NumericTraits<char>::max() );  
      break;

    case itk::ImageIOBase::USHORT:
      result = DoMain<unsigned short>( args,
                                       itk::NumericTraits<unsigned short>::ZeroValue(),
                                       static_cast<unsigned short>( 32767 ) );
      break;

    case itk::ImageIOBase::SHORT:
      result = DoMain<short>( args,
                              itk::NumericTraits<short>::ZeroValue(),
                              static_cast<short>( 32767 ) );  
      break;

    case itk::ImageIOBase::UINT:
      result = DoMain<unsigned int>( args,
                                     itk::NumericTraits<unsigned int>::ZeroValue(),
                                     static_cast<unsigned int>( 32767 ) );  
      break;

    case itk::ImageIOBase::INT:
      result = DoMain<int>( args,
                            itk::NumericTraits<int>::ZeroValue(),
                            static_cast<int>( 32767 ) );  
      break;

    case itk::ImageIOBase::ULONG:
      result = DoMain<unsigned long>( args,
                                      itk::NumericTraits<unsigned long>::ZeroValue(),
                                      static_cast<unsigned long>( 32767 ) );  
      break;

    case itk::ImageIOBase::LONG:
      result = DoMain<long>( args,
                             itk::NumericTraits<long>::ZeroValue(),
                             static_cast<long>( 32767 ) );  
      break;

    case itk::ImageIOBase::FLOAT:
      result = DoMain<float>( args,
                              itk::NumericTraits<float>::ZeroValue(),
                              static_cast<float>( 32767 ) );  
      break;

    case itk::ImageIOBase::DOUBLE:
      result = DoMain<double>( args,
                               itk::NumericTraits<double>::ZeroValue(),
                               static_cast<double>( 32767 ) );  
      break;

    default:
      std::cerr << "WARNING: Unrecognised pixel type, skipping file: " 
		<< args.iterFilename << std::endl;
    }

    std::cout << std::endl;
  }

  return EXIT_SUCCESS;
}
 
 

