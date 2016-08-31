/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

  =============================================================================*/

/*!
 * \file niftkAnonymiseDICOMMammograms.cxx 
 * \page niftkAnonymiseDICOMMammograms
 * \section niftkAnonymiseDICOMMammogramsSummary niftkAnonymiseDICOMMammograms
 * 
 * Search for DICOM mammograms in a directory and anonymise them by removing patient information from the DICOM header and/or applying a rectangular mask to remove the label.
 *
 */


#include <niftkFileHelper.h>
#include <niftkConversionUtils.h>
#include <itkCommandLineHelper.h>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNifTKImageIOFactory.h>
#include <itkImageRegionIterator.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkGDCMImageIO.h>
#include <itkImageLinearIteratorWithIndex.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkInvertIntensityBetweenMaxAndMinImageFilter.h>
#include <itkCreatePositiveMammogram.h>
#include <itkFlipImageFilter.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <vector>

#include <niftkAnonymiseDICOMMammogramsCLP.h>


namespace fs = boost::filesystem;

typedef itk::MetaDataDictionary DictionaryType;
typedef itk::MetaDataObject< std::string > MetaDataStringType;


struct arguments
{
  std::string dcmDirectoryIn;
  std::string dcmDirectoryOut;
  std::string strAdd2Suffix;  

  bool flgOverwrite;
  bool flgAnonymiseDICOMHeader;
  bool flgAnonymiseImageLabel;
  bool flgRescaleIntensitiesToMaxRange;
  bool flgInvert;
  bool flgFlipHorizontally;
  bool flgFlipVertically;
  bool flgVerbose;

  float labelWidth;
  float labelHeight;

  std::string labelPosition;
  std::string labelSide;
  
  bool flgDontAnonPatientsName;
  std::string strPatientsName;

  bool flgDontAnonPatientsBirthDate;
  std::string strPatientsBirthDate;

  bool flgDontAnonOtherPatientNames;
  std::string strOtherPatientNames;

  bool flgDontAnonPatientsBirthName;
  std::string strPatientsBirthName;

  bool flgDontAnonPatientsAddress;
  std::string strPatientsAddress;

  bool flgDontAnonPatientsMothersBirthName;
  std::string strPatientsMothersBirthName;

  bool flgDontAnonPatientsTelephoneNumbers;
  std::string strPatientsTelephoneNumbers;


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
// AnonymiseTag()
// -------------------------------------------------------------------------

void AnonymiseTag( bool flgDontAnonymise, 
		   DictionaryType &dictionary,
		   std::string tagID,
		   std::string newTagValue )
{
  if ( flgDontAnonymise )
    return;

  // Search for the tag
  
  DictionaryType::ConstIterator tagItr = dictionary.Find( tagID );
  DictionaryType::ConstIterator end = dictionary.End();
   
  if ( tagItr != end )
  {
    MetaDataStringType::ConstPointer entryvalue = 
      dynamic_cast<const MetaDataStringType *>( tagItr->second.GetPointer() );
    
    if ( entryvalue )
    {
      std::string tagValue = entryvalue->GetMetaDataObjectValue();
      
      std::cout << "Anonymising tag (" << tagID <<  ") "
		<< " from: " << tagValue 
		<< " to: " << newTagValue << std::endl;
      
      itk::EncapsulateMetaData<std::string>( dictionary, tagID, newTagValue );
    }
  }

};


// -------------------------------------------------------------------------
// main()
// -------------------------------------------------------------------------

template <class InputPixelType>
int DoMain(arguments args, InputPixelType min, InputPixelType max)
{
  enum BreastSideType { 
    UNKNOWN_BREAST_SIDE,
    LEFT_BREAST_SIDE,
    RIGHT_BREAST_SIDE,
  };

  BreastSideType breastSide = UNKNOWN_BREAST_SIDE;


  // Iterate through each file and anonymise it
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  std::string fileInputFullPath;
  std::string fileInputRelativePath;
  std::string fileOutputRelativePath;
  std::string fileOutputFullPath;
  std::string dirOutputFullPath;
    
  const unsigned int   InputDimension = 2;

  typedef itk::Image< InputPixelType, InputDimension > InputImageType;
  typedef itk::ImageFileReader< InputImageType > ReaderType;
  typedef itk::ImageFileWriter< InputImageType > WriterType;
  typedef itk::ImageRegionIterator< InputImageType > IteratorType;  


  typename ReaderType::Pointer reader = ReaderType::New();
  typename InputImageType::Pointer image;

  typedef itk::GDCMImageIO           ImageIOType;
  typename ImageIOType::Pointer gdcmImageIO = ImageIOType::New();

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


  DictionaryType dictionary = reader->GetOutput()->GetMetaDataDictionary();
  
  if ( args.flgVerbose )
  {
    PrintDictionary( dictionary );
  }

  // Rescale the image intensities

  if ( args.flgRescaleIntensitiesToMaxRange )
  {
    itksys_ios::ostringstream value;

    typedef typename itk::RescaleIntensityImageFilter<InputImageType, 
						      InputImageType> RescaleFilterType;
    
    typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();

    rescaleFilter->SetInput( reader->GetOutput() );

    rescaleFilter->SetOutputMinimum( min );
    rescaleFilter->SetOutputMaximum( max );  
  
    std::cout << "Scaling image intensity range from: " 
	      << min << " to " << max << std::endl;

    rescaleFilter->Update();

    image = rescaleFilter->GetOutput();
    image->DisconnectPipeline();

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
  }
  else {
    image = reader->GetOutput();
    image->DisconnectPipeline();
  }


  // Check that the modality DICOM tag is 'MG'

  std::string tagModalityID = "0008|0060";
  std::string tagModalityValue;

  DictionaryType::ConstIterator tagItr = dictionary.Find( tagModalityID );
  DictionaryType::ConstIterator end = dictionary.End();
   
  if( tagItr != end )
  {
    MetaDataStringType::ConstPointer entryvalue = 
      dynamic_cast<const MetaDataStringType *>( tagItr->second.GetPointer() );

    if ( entryvalue )
    {
      tagModalityValue = entryvalue->GetMetaDataObjectValue();
      std::cout << "Modality Name (" << tagModalityID <<  ") "
		<< " is: " << tagModalityValue << std::endl;
    }
  }

  if ( ( tagModalityValue == std::string( "CR" ) ) || // Computed Radiography
       ( tagModalityValue == std::string( "MG" ) ) )  // Mammography
  {
    std::cout << "Image is definitely mammography - anonymising"
	      << std::endl;
  }
  else if ( tagModalityValue == std::string( "RG" ) )  // Radiography?
  {
    std::cout << "Image could be mammography - anonymising"
	      << std::endl;
  }
  else if ( ( tagModalityValue == std::string( "CT" ) ) || //  Computed Tomography
	    ( tagModalityValue == std::string( "DX" ) ) || //  Digital Radiography
	    ( tagModalityValue == std::string( "ECG" ) ) || //  Electrocardiography
	    ( tagModalityValue == std::string( "EPS" ) ) || // Cardiac Electrophysiology
	    ( tagModalityValue == std::string( "ES" ) ) || //  Endoscopy
	    ( tagModalityValue == std::string( "GM" ) ) || //  General Microscopy
	    ( tagModalityValue == std::string( "HD" ) ) || //  Hemodynamic Waveform
	    ( tagModalityValue == std::string( "IO" ) ) || //  Intra-oral Radiography
	    ( tagModalityValue == std::string( "IVUS" ) ) || //  Intravascular Ultrasound
	    ( tagModalityValue == std::string( "MR" ) ) || //  Magnetic Resonance
	    ( tagModalityValue == std::string( "NM" ) ) || //  Nuclear Medicine
	    ( tagModalityValue == std::string( "OP" ) ) || //  Ophthalmic Photography
	    ( tagModalityValue == std::string( "PT" ) ) || //  Positron emission tomography
	    ( tagModalityValue == std::string( "PX" ) ) || //  Panoramic X-Ray
	    ( tagModalityValue == std::string( "RF" ) ) || //  Radiofluoroscopy
	    ( tagModalityValue == std::string( "RTIMAGE" ) ) || //  Radiotherapy Image
	    ( tagModalityValue == std::string( "SM" ) ) || //  Slide Microscopy
	    ( tagModalityValue == std::string( "US" ) ) || //  Ultrasound
	    ( tagModalityValue == std::string( "XA" ) ) || //  X-Ray Angiography
	    ( tagModalityValue == std::string( "XC" ) ) ) //  External-camera Photography
  {
    std::cout << "Skipping image - does not appear to be a mammogram" << std::endl << std::endl;
    return EXIT_SUCCESS;
  }
  else
  {
    std::cout << "WARNING: Unsure if this ia a mammogram but anonymising anyway" 
	      << std::endl;
  }


  // Create a positive version of the mammogram?

  itk::CreatePositiveMammogram< InputImageType >( image, dictionary, args.flgInvert );


  // Anonymise the image label

  if ( args.flgAnonymiseImageLabel )
  {

    typename InputImageType::RegionType region;
    typename InputImageType::SizeType   size;
    typename InputImageType::SizeType   scanSize;
    typename InputImageType::IndexType  start;
    typename InputImageType::IndexType  idx;

    region = image->GetLargestPossibleRegion();      

    size = region.GetSize();


    if ( args.labelSide == std::string( "Automatic" ) )
    {

      // Determine if this is a left or right breast by calculating the CoM

      start[0] = size[0]/10;
      start[1] = 0;

      scanSize[0] = size[0]*8/10;
      scanSize[1] = size[1];

      region.SetSize(  scanSize  );
      region.SetIndex( start );

      std::cout << "Image size: " << size << std::endl;
      std::cout << "Region: " << region << std::endl;

      unsigned int iRow = 0;
      unsigned int nRows = 5;
      unsigned int rowSpacing = size[1]/( nRows + 1 );

      float xMoment = 0.;
      float xMomentSum = 0.;
      float intensitySum = 0.;

      typedef typename itk::ImageLinearIteratorWithIndex< InputImageType > LineIteratorType;

      LineIteratorType itLinear( image, region );

      itLinear.SetDirection( 0 );

      while ( ! itLinear.IsAtEnd() )
      {
	// Skip initial set of rows

	iRow = 0;
	while ( ( ! itLinear.IsAtEnd() ) && ( iRow < rowSpacing ) )
	{
	  iRow++;
	  itLinear.NextLine();
	}

	// Add next row to moment calculation

	while ( ! itLinear.IsAtEndOfLine() )
	{
	  idx = itLinear.GetIndex();

	  intensitySum += itLinear.Get();

	  xMoment = idx[0]*itLinear.Get();
	  xMomentSum += xMoment;

	  ++itLinear;
	}
      }

      xMoment = xMomentSum/intensitySum;

      std::cout << "Center of mass in x: " << xMoment << std::endl;


      if ( xMoment > static_cast<float>(size[0])/2. )
      {
	breastSide = RIGHT_BREAST_SIDE;
	std::cout << "RIGHT breast (label on left-hand side)" << std::endl;
      }
      else 
      {
	breastSide = LEFT_BREAST_SIDE;
	std::cout << "LEFT breast (label on right-hand side)" << std::endl;
      }
    }
    
    else if ( args.labelSide == std::string( "Right" ) )
    {
      breastSide = LEFT_BREAST_SIDE;
      std::cout << "Label on RIGHT-hand side (left breast)" << std::endl;
    }

    else if ( args.labelSide == std::string( "Left" ) )
    {
      breastSide = RIGHT_BREAST_SIDE;
      std::cout << "Label on left-hand side (right breast)" << std::endl;
    }


    // Set the label region to zero

    start[0] = size[0];
    start[1] = size[1];

    size[0] = static_cast<unsigned int>( static_cast<float>(size[0]) * args.labelWidth/100. );
    size[1] = static_cast<unsigned int>( static_cast<float>(size[1]) * args.labelHeight/100. );

    if ( args.labelPosition == std::string( "Upper" ) )
    {
      start[1] = 0;
    }
    else 
    {
      start[1] -= size[1];
    }

    if ( breastSide == LEFT_BREAST_SIDE )
    {
      start[0] -= size[0];
    }
    else 
    {
      start[0] = 0;
    }

    region.SetSize( size );
    region.SetIndex( start );

    std::cout << "Removing label from region: " << region << std::endl;

    IteratorType itLabel( image, region );
  
    for ( itLabel.GoToBegin(); 
	  ! itLabel.IsAtEnd() ; 
	  ++itLabel )
    {
      itLabel.Set( 0 );
    }
  }


  // Flip the image?

  if ( args.flgFlipHorizontally || args.flgFlipVertically )
  {
    typename InputImageType::PointType inOrigin = image->GetOrigin();

    typedef itk::FlipImageFilter<InputImageType> FlipImageFilterType;
 
    typename FlipImageFilterType::Pointer flipFilter = FlipImageFilterType::New();

    itk::FixedArray<bool, InputDimension> flipAxes;

    flipAxes[0] = args.flgFlipHorizontally;
    flipAxes[1] = args.flgFlipVertically;
  
    flipFilter->SetInput( image );
    flipFilter->SetFlipAxes( flipAxes );

    flipFilter->Update();

    typename InputImageType::Pointer imFlipped = flipFilter->GetOutput();
    
    imFlipped->DisconnectPipeline();
    imFlipped->SetOrigin( inOrigin );

    image = static_cast< InputImageType* >( imFlipped );
  }


  // Anonymise the DICOM header?

  if ( args.flgAnonymiseDICOMHeader )
  {
    AnonymiseTag( args.flgDontAnonPatientsName,  	    dictionary, "0010|0010", "Anonymous"    ); // Patient's Name                               
    AnonymiseTag( args.flgDontAnonPatientsBirthDate,	    dictionary, "0010|0030", "00000000"     ); // Patient's Birth Date                        
    AnonymiseTag( args.flgDontAnonOtherPatientNames, 	    dictionary, "0010|1001", "None"         ); // Other Patient Names                         
    AnonymiseTag( args.flgDontAnonPatientsBirthName, 	    dictionary, "0010|1005", "Anonymous"    ); // Patient's Birth Name                        
    AnonymiseTag( args.flgDontAnonPatientsAddress, 	    dictionary, "0010|1040", "None"         ); // Patient's Address                           
    AnonymiseTag( args.flgDontAnonPatientsMothersBirthName, dictionary, "0010|1060", "Anonymous"    ); // Patient's Mother's Birth Name               
    AnonymiseTag( args.flgDontAnonPatientsTelephoneNumbers, dictionary, "0010|2154", "None"         ); // Patient's Telephone Numbers                 
  }
      

  // Create the output image filename

  fileInputFullPath = args.iterFilename;

  fileInputRelativePath = fileInputFullPath.substr( args.dcmDirectoryIn.length() );
     
  fileOutputRelativePath = niftk::AddStringToImageFileSuffix( fileInputRelativePath,
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

    writer->SetFileName( fileOutputFullPath );

    image->DisconnectPipeline();
    writer->SetInput( image );

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

  if ( ! ( flgAnonymiseDICOMHeader || flgAnonymiseImageLabel ) )
  {
    std::cerr << "WARNING: Neither 'anonHeader' or 'anonLabel' specified" << std::endl;
  }

  if ( dcmDirectoryOut.length() == 0 )
  {
    dcmDirectoryOut = dcmDirectoryIn;
  }

  args.dcmDirectoryIn  = dcmDirectoryIn;                     
  args.dcmDirectoryOut = dcmDirectoryOut;                    

  args.strAdd2Suffix = strAdd2Suffix;                      
				   	                                                 
  args.flgOverwrite            = flgOverwrite;                       
  args.flgAnonymiseDICOMHeader = flgAnonymiseDICOMHeader;            
  args.flgAnonymiseImageLabel  = flgAnonymiseImageLabel;    
  args.flgVerbose              = flgVerbose;    

  args.flgRescaleIntensitiesToMaxRange = flgRescaleIntensitiesToMaxRange;
  args.flgInvert = flgInvert;
  args.flgFlipHorizontally = flgFlipHorizontally;
  args.flgFlipVertically   = flgFlipVertically;
				   	                                                 
  args.labelWidth  = labelWidth;                         
  args.labelHeight = labelHeight;                        
					                                                 
  args.labelPosition = labelPosition;                      
  args.labelSide     = labelSide;                          

  args.flgDontAnonPatientsName = flgDontAnonPatientsName;            
  args.strPatientsName         = strPatientsName;                    
					                                                 
  args.flgDontAnonPatientsBirthDate = flgDontAnonPatientsBirthDate;       
  args.strPatientsBirthDate	    = strPatientsBirthDate;               
					                                                 
  args.flgDontAnonOtherPatientNames = flgDontAnonOtherPatientNames;       
  args.strOtherPatientNames	    = strOtherPatientNames;               
					                                                 
  args.flgDontAnonPatientsBirthName = flgDontAnonPatientsBirthName;       
  args.strPatientsBirthName	    = strPatientsBirthName;               
					                                                 
  args.flgDontAnonPatientsAddress = flgDontAnonPatientsAddress;         
  args.strPatientsAddress	  = strPatientsAddress;                 
					                                                 
  args.flgDontAnonPatientsMothersBirthName = flgDontAnonPatientsMothersBirthName;
  args.strPatientsMothersBirthName         = strPatientsMothersBirthName;        
					                                                 
  args.flgDontAnonPatientsTelephoneNumbers = flgDontAnonPatientsTelephoneNumbers;
  args.strPatientsTelephoneNumbers         = strPatientsTelephoneNumbers;        


  std::cout << std::endl << "Examining directory: " 
	    << dcmDirectoryIn << std::endl << std::endl;


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
 
 

