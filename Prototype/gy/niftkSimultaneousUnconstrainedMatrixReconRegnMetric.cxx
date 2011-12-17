/*=============================================================================

NifTK: An image processing toolkit jointly developed by the
Dementia Research Centre, and the Centre For Medical Image Computing
at University College London.

See:        http://dementia.ion.ucl.ac.uk/
http://cmic.cs.ucl.ac.uk/
http://www.ucl.ac.uk/

Last Changed      : $Date: 2010-06-04 15:11:19 +0100 (Fri, 04 Jun 2010) $
Revision          : $Revision: 3349 $
Last modified by  : $Author: jhh, gy $

Original author   : j.hipwell@ucl.ac.uk

Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

============================================================================*/

#include "itkLogHelper.h"
#include "ConversionUtils.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkEulerAffineTransform.h"
#include "itkTransformFactory.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"

#include "itkTransformFactory.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"

#include "itkGE5000_TomosynthesisGeometry.h"
#include "itkGE6000_TomosynthesisGeometry.h"
#include "itkIsocentricConeBeamRotationGeometry.h"

#include "itkForwardAndBackwardProjectionMatrix.h"
#include "itkEulerAffineTransformMatrixAndItsVariations.h"

#include "itkSimultaneousUnconstrainedMatrixReconRegnMetric.h"


void Usage(char *exec)
{

  niftk::itkLogHelper::PrintCommandLineHeader(std::cout);
  std::cout << "  " << std::endl
    << "  Program of testing the metric (cost function and its derivative) using matrix form manipulations" << std::endl << std::endl

    << "  " << exec 
    << " -s3D Input3DimageSize -sz OutputSize -im Input3Dimage -o Output2Dimage "
    << std::endl << "  " << std::endl

    << "*** [mandatory] ***" << std::endl << std::endl
    << "    -s3D   <int> <int> <int>        	Input 3D image volume size " << std::endl
    << "    -sz   <int> <int>       					The size of the 2D projection image " << std::endl
    << "    -im   <filename>        					Input 3D image volume " << std::endl
    << "    -o    <filename>        					Output 2D projection image" << std::endl

    << "*** [options]   ***" << std::endl << std::endl
    << "    -r3D  <float> <float> <float>  		The resolution of the reconstructed volume [1mm x 1mm x 1mm]" << std::endl
    << "    -o3D  <float> <float> <float>  		The origin of the reconstructed volume [0mm x 0mm x 0mm]" << std::endl << std::endl
    << "    -res  <float> <float>   					The resolution of the 2D projection image [1mm x 1mm]" << std::endl
    << "    -o2D  <float> <float>   					The origin of the 2D projection image [0mm x 0mm]" << std::endl << std::endl

    << "  Use the following three options to specify an isocentric cone beam rotation" << std::endl
    << "    -1stAngle <double>     						The angle of the first projection in the sequence [-89]" << std::endl
    << "    -AngRange <double>     						The full angular range of the sequence [180]" << std::endl
    << "    -FocalLength <double>  						The focal length of the projection [660]" << std::endl << std::endl

    << "    -GE5000                						Use the 'old' GE-5000, 11 projection geometry" << std::endl
    << "    -GE6000                						Use the 'new' GE-6000, 15 projection geometry" << std::endl << std::endl;

}


/**
 * \brief Project a 3D image volume into 3D.
 */
int main(int argc, char** argv)
{
  typedef double 																																								TScalarType;
  typedef float 																																								IntensityType;
  typedef vnl_sparse_matrix<TScalarType>           																							SparseMatrixType;
  typedef vnl_vector<TScalarType>                    																						VectorType;

  typedef itk::EulerAffineTransformMatrixAndItsVariations< TScalarType > 												AffineTransformerType;
  typedef AffineTransformerType::EulerAffineTransformType 																			EulerAffineTransformType;

  typedef itk::ForwardAndBackwardProjectionMatrix< TScalarType, IntensityType > 								MatrixProjectorType;
	typedef itk::SimultaneousUnconstrainedMatrixReconRegnMetric < TScalarType, IntensityType > 		SimultaneousUnconMetricType;

  std::string fileInputImage3D;
  typedef MatrixProjectorType::InputImageType 				InputImageType;
  typedef InputImageType::Pointer      								InputImagePointer;
  typedef InputImageType::ConstPointer 								InputImageConstPointer;
  typedef InputImageType::RegionType   								InputImageRegionType;
  typedef InputImageType::PixelType    								InputImagePixelType;
  typedef InputImageType::SizeType     								InputImageSizeType;
  typedef InputImageType::SpacingType  								InputImageSpacingType;
  typedef InputImageType::PointType   								InputImagePointType;
  typedef InputImageType::IndexType   								InputImageIndexType;
  typedef itk::ImageFileReader< InputImageType >  		InputImageReaderType;

  std::string fileOutputImage2D;
  typedef MatrixProjectorType::OutputImageType 				OutputImageType;
  typedef OutputImageType::Pointer     								OutputImagePointer;
  typedef OutputImageType::ConstPointer 							OutputImageConstPointer;
  typedef OutputImageType::RegionType  								OutputImageRegionType;
  typedef OutputImageType::PixelType   								OutputImagePixelType;
  typedef OutputImageType::SizeType    								OutputImageSizeType;
  typedef OutputImageType::SpacingType 								OutputImageSpacingType;
  typedef OutputImageType::PointType   								OutputImagePointType;
  typedef OutputImageType::IndexType   								OutputImageIndexType;
  typedef itk::ImageFileWriter< OutputImageType > 		OutputImageWriterType;

  typedef itk::ProjectionGeometry< IntensityType > 		ProjectionGeometryType;

  bool flgGE_5000 = true;						// Use the GE 5000 11 projection geometry
  bool flgGE_6000 = false;					// Use the GE 6000 15 projection geometry

  double firstAngle = 	0;         	// The angle of the first projection in the sequence
  double angularRange = 0;       		// The full angular range of the sequence
  double focalLength = 	0;        	// The focal length of the projection

  double thetaX = 0;		 						// An additional rotation in 'x'
  double thetaY = 0;		 						// An additional rotation in 'y'
  double thetaZ = 0;		 						// An additional rotation in 'z'

  // Initialise the affine parameters
	EulerAffineTransformType::ParametersType parameters;

	// parameters 0-2: translation; 3-5: rotation; 6-8 scaling; 9-11: skew
  parameters.SetSize(12); 					

  parameters.Fill(0.);

  parameters[0] = -2;
  parameters[1] = 2;
  parameters[2] = 3;

  parameters[4] = 12;
  parameters[5] = 8;

  parameters[6] = 1.;								// Scale factor along the 'x' axis
  parameters[7] = 1.;								// Scale factor along the 'y' axis
  parameters[8] = 1.;								// Scale factor along the 'z' axis

	
	// Initial guess of the affine parameters
	EulerAffineTransformType::ParametersType parametersInitialGuess;

	// parameters 0-2: translation; 3-5: rotation; 6-8 scaling; 9-11: skew
  parametersInitialGuess.SetSize(12); 					

  parametersInitialGuess.Fill(0.);

  parametersInitialGuess[0] = 1;
  parametersInitialGuess[1] = 1;
  parametersInitialGuess[2] = 1;

  parametersInitialGuess[4] = 1;
  parametersInitialGuess[5] = 1;

  parametersInitialGuess[6] = 1.;								// Scale factor along the 'x' axis
  parametersInitialGuess[7] = 1.;								// Scale factor along the 'y' axis
  parametersInitialGuess[8] = 1.;								// Scale factor along the 'z' axis


  // The dimensions in pixels of the 3D image
  MatrixProjectorType::VolumeSizeType 	nPixels3D;
  // The resolution in mm of the 3D image
  InputImageSpacingType  								spacing3D;
  // The origin in mm of the 3D image
  InputImagePointType										origin3D;

  // The dimensions in pixels of the 2D image
  OutputImageSizeType 									nPixels2D;
  // The resolution in mm of the 2D image
  OutputImageSpacingType 								spacing2D;
  // The origin in mm of the 2D image
  OutputImagePointType 									origin2D;

  spacing3D[0] = 1.;
  spacing3D[1] = 1.;
  spacing3D[2] = 1.;

  origin3D[0] = 0.;
  origin3D[1] = 0.;
  origin3D[2] = 0.;

  spacing2D[0] = 1.;
  spacing2D[1] = 1.;

  origin2D[0] = 0.;
  origin2D[1] = 0.;

  // Parse command line args
  // ~~~~~~~~~~~~~~~~~~~~~~~

  for(int i=1; i < argc; i++){
    if(strcmp(argv[i], "-help")==0 || strcmp(argv[i], "-Help")==0 || strcmp(argv[i], "-HELP")==0 
        || strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--h")==0){
      Usage(argv[0]);
      return -1;
    }
    else if(strcmp(argv[i], "-s3D") == 0) {
      nPixels3D[0] = atoi(argv[++i]);
      nPixels3D[1] = atoi(argv[++i]);
      nPixels3D[2] = atoi(argv[++i]);
      std::cout << "Set -s3D="
          << niftk::ConvertToString((int) nPixels3D[0]) << " "
          << niftk::ConvertToString((int) nPixels3D[1]) << " "
          << niftk::ConvertToString((int) nPixels3D[2]);
    }
    else if(strcmp(argv[i], "-sz") == 0) {
      nPixels2D[0] = atoi(argv[++i]);
      nPixels2D[1] = atoi(argv[++i]);
      std::cout << "Set -sz="
          << niftk::ConvertToString((int) nPixels2D[0]) << " "
          << niftk::ConvertToString((int) nPixels2D[1]);
    }
    else if(strcmp(argv[i], "-im") == 0) {
      fileInputImage3D = argv[++i];
      std::cout << "Set -im=" << fileInputImage3D;
    }
    else if(strcmp(argv[i], "-o") == 0) {
      fileOutputImage2D = argv[++i];
      std::cout << "Set -o=" << fileOutputImage2D;
    }
    else if(strcmp(argv[i], "-r3D") == 0) {
      spacing3D[0] = atof(argv[++i]);
      spacing3D[1] = atof(argv[++i]);
      spacing3D[2] = atof(argv[++i]);
      cout << "Reconstruction resolution: "
        << spacing3D[0] << " x " << spacing3D[1] << " x " << spacing3D[2] << " mm" << endl;
    }
    else if(strcmp(argv[i], "-o3D") == 0) {
      origin3D[0] = atof(argv[++i]);
      origin3D[1] = atof(argv[++i]);
      origin3D[2] = atof(argv[++i]);
      cout << "Reconstruction origin: "
        << origin3D[0] << " x " << origin3D[1] << " x " << origin3D[2] << " mm" << endl;
    }
    else if(strcmp(argv[i], "-res") == 0) {
      spacing2D[0] = atof(argv[++i]);
      spacing2D[1] = atof(argv[++i]);
      std::cout << "Set -res="
          << niftk::ConvertToString(spacing2D[0]) << " "
          << niftk::ConvertToString(spacing2D[1]);
    }
    else if(strcmp(argv[i], "-o2D") == 0) {
      origin2D[0] = atof(argv[++i]);
      origin2D[1] = atof(argv[++i]);
      std::cout << "Set -o2D="
    		  << niftk::ConvertToString(origin2D[0]) << " "
    		  << niftk::ConvertToString(origin2D[1]);
    }

    // Reconstruction geometry command line options

    else if(strcmp(argv[i], "-1stAngle") == 0) {
      firstAngle = (unsigned int) atof(argv[++i]);
      flgGE_5000 = false;
      std::cout << "Set -1stAngle=" << niftk::ConvertToString(firstAngle);
    }
    else if(strcmp(argv[i], "-AngRange") == 0) {
      angularRange = (unsigned int) atof(argv[++i]);
      flgGE_5000 = false;
      std::cout << "Set -AngRange=" << niftk::ConvertToString(angularRange);
    }
    else if(strcmp(argv[i], "-FocalLength") == 0) {
      focalLength = (unsigned int) atof(argv[++i]);
      flgGE_5000 = false;
      std::cout << "Set -FocalLength=" << niftk::ConvertToString(focalLength);
    }
    else if(strcmp(argv[i], "-GE5000") == 0) {
      flgGE_5000 = true;
      flgGE_6000 = false;
      std::cout << "Set -GE5000";
    }

    else if(strcmp(argv[i], "-GE6000") == 0) {
      flgGE_6000 = true;
      flgGE_5000 = false;
      std::cout << "Set -GE6000";
    }
    else if(strcmp(argv[i], "-thetaX") == 0) {
      thetaX = atof(argv[++i]);
      std::cout << "Set -thetaX";
    }
    else if(strcmp(argv[i], "-thetaY") == 0) {
      thetaY = atof(argv[++i]);
      std::cout << "Set -thetaY";
    }
    else if(strcmp(argv[i], "-thetaZ") == 0) {
      thetaZ = atof(argv[++i]);
      std::cout << "Set -thetaZ";
    }
    else {
      std::cerr << argv[0] << ":\tParameter " << argv[i] << " unknown." << std::endl;
      return -1;
    }            
  }


  // Validate command line args
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~

  if ( nPixels3D[0] == 0 || nPixels3D[1] == 0 || nPixels3D[2] == 0 || 
      fileInputImage3D.length() == 0 || fileOutputImage2D.length() == 0 || 
      nPixels2D[0] == 0 || nPixels2D[1] == 0 ) {
    Usage(argv[0]);
    return EXIT_FAILURE;
  }     

  if ( flgGE_5000 && flgGE_6000 ) {
    std::cerr << "Command line options '-GE5000' and '-GE6000' are exclusive.";

    Usage(argv[0]);
    return EXIT_FAILURE;
  }

  if ( (flgGE_5000 || flgGE_6000) && (firstAngle || angularRange || focalLength) ) {
    std::cerr << "Command line options '-GE5000' or '-GE6000' "
        "and '-1stAngle' or '-AngRange' or '-FocalLength' are exclusive.";

    Usage(argv[0]);
    return EXIT_FAILURE;
  }


  // Load the input image volume
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~

  InputImageReaderType::Pointer inputImageReader  = InputImageReaderType::New();

  inputImageReader->SetFileName( fileInputImage3D );

  try { 
    std::cout << "Reading input 3D volume: " <<  fileInputImage3D;
    inputImageReader->Update();
    std::cout << "Done";
  } 
  catch( itk::ExceptionObject & err ) { 
    std::cerr << "ERROR: Failed to load input image: " << err << std::endl; 
    return EXIT_FAILURE;
  }

  InputImageConstPointer inImage = inputImageReader->GetOutput();


  // Create an initial guess as the 3D volume estimation
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  InputImagePointer inImageEstimation = InputImageType::New();

  InputImageIndexType inputStart;
  inputStart[0] = 0; // first index on X
  inputStart[1] = 0; // first index on Y
  inputStart[2] = 0; // first index on Z

  InputImageRegionType inputRegion;
  inputRegion.SetSize( nPixels3D );
  inputRegion.SetIndex( inputStart );

  inImageEstimation->SetRegions( inputRegion );
  inImageEstimation->SetOrigin( origin3D );
  inImageEstimation->SetSpacing( spacing3D );
  inImageEstimation->Allocate();


  // Create an initial 2D projection image
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  OutputImagePointer outImage = OutputImageType::New();

  OutputImageIndexType outputStart;
  outputStart[0] = 0; // first index on X
  outputStart[1] = 0; // first index on Y

  OutputImageRegionType outputRegion;
  outputRegion.SetSize( nPixels2D );
  outputRegion.SetIndex( outputStart );

  outImage->SetRegions( outputRegion );
  outImage->SetOrigin( origin2D );
  outImage->SetSpacing( spacing2D );
  outImage->Allocate();


  // Create the matrix projector
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~

  MatrixProjectorType::Pointer matrixProjector = MatrixProjectorType::New();

  // Set the number of projections
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  unsigned int projectionNumber = 11;


  // Create the tomosynthesis geometry
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  ProjectionGeometryType::Pointer geometry; 

  // Create the GE-5000 11 projection geometry 

  if (flgGE_5000) {

    projectionNumber = 11;

    if (projectionNumber != 11) {
      std::cerr << "ERROR: Number of projections in input volume (" << projectionNumber << ") must equal 11 for GE-5000 geometry" << endl;
      return EXIT_FAILURE;
    }         

    typedef itk::GE5000_TomosynthesisGeometry< IntensityType > GE5000_TomosynthesisGeometryType;
    geometry = GE5000_TomosynthesisGeometryType::New();

    geometry->SetProjectionSize(nPixels2D);		
    geometry->SetProjectionSpacing(spacing2D);

    geometry->SetVolumeSize(nPixels3D);
    geometry->SetVolumeSpacing(spacing3D);

  }

  // Create the GE-6000 15 projection geometry 

  else if (flgGE_6000) {

    projectionNumber = 15;

    if (projectionNumber != 15) {
      std::cerr << "ERROR: Number of projections in input volume (" << projectionNumber << ") must equal 15 for GE-6000 geometry" << endl;
      return EXIT_FAILURE;
    }

    typedef itk::GE6000_TomosynthesisGeometry< IntensityType > GE6000_TomosynthesisGeometryType;
    geometry = GE6000_TomosynthesisGeometryType::New();

    geometry->SetProjectionSize(nPixels2D);		
    geometry->SetProjectionSpacing(spacing2D);

    geometry->SetVolumeSize(nPixels3D);
    geometry->SetVolumeSpacing(spacing3D);

  }

  // Create an isocentric cone bean rotation geometry

  else {

    if (! firstAngle) firstAngle = -89.;
    if (! angularRange) angularRange = 180.;
    if (! focalLength) focalLength = 660.;

    projectionNumber = 180;

    typedef itk::IsocentricConeBeamRotationGeometry< IntensityType > IsocentricConeBeamRotationGeometryType;

    IsocentricConeBeamRotationGeometryType::Pointer isoGeometry = IsocentricConeBeamRotationGeometryType::New();

    isoGeometry->SetNumberOfProjections(projectionNumber);
    isoGeometry->SetFirstAngle(firstAngle);
    isoGeometry->SetAngularRange(angularRange);
    isoGeometry->SetFocalLength(focalLength);

    geometry = isoGeometry;

    geometry->SetProjectionSize(nPixels2D);		
    geometry->SetProjectionSpacing(spacing2D);

    geometry->SetVolumeSize(nPixels3D);
    geometry->SetVolumeSpacing(spacing3D);

  }

  if (thetaX) geometry->SetRotationInX(thetaX);
  if (thetaY) geometry->SetRotationInY(thetaY);
  if (thetaZ) geometry->SetRotationInZ(thetaZ);

  matrixProjector->SetProjectionGeometry( geometry );


  // Create the initial matrix
  // ~~~~~~~~~~~~~~~~~~~~~~~~~
  unsigned int totalSize3D = nPixels3D[0]*nPixels3D[1]*nPixels3D[2];
  unsigned int totalSize2D = nPixels2D[0]*nPixels2D[1];
  unsigned int totalSizeAllProjs = projectionNumber*totalSize2D;
  static SparseMatrixType forwardProjectionMatrix(totalSizeAllProjs, totalSize3D);
  static SparseMatrixType backwardProjectionMatrix(totalSize3D, totalSizeAllProjs);

  // Obtain the forward and backward projection matrix
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  matrixProjector->GetForwardProjectionSparseMatrix(forwardProjectionMatrix, inImage, outImage, nPixels3D, nPixels2D, projectionNumber);

  // Create the affine transformer
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  AffineTransformerType::Pointer affineTransformer = AffineTransformerType::New();
  // AffineTransformerType::EulerAffineTransformType::Pointer affineTransform = EulerAffineTransformType::New();
  // affineTransformer->SetAffineTransform(affineTransform);

  static SparseMatrixType affineMatrix(totalSize3D, totalSize3D);
  static SparseMatrixType affineMatrixTranspose(totalSize3D, totalSize3D);

  // Obtain the affine transformation matrix and its transpose
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  affineTransformer->GetAffineTransformationSparseMatrix(affineMatrix, nPixels3D, parameters);


  /*
  // Print out the non-zero entries of the forward projection matrix
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  forwardProjectionMatrix.reset();
  std::ofstream sparseforwardProjectionMatrixFile("sparseForwardProjectionMatrix.txt");
  sparseforwardProjectionMatrixFile << std::endl << "The non-zero entries of the forward projection matrix are: " << std::endl;

  unsigned int rowIndex = 0;
  unsigned int colIndex = 0;

  while ( forwardProjectionMatrix.next() )
  {
  rowIndex = forwardProjectionMatrix.getrow();
  colIndex = forwardProjectionMatrix.getcolumn();

  if ( (rowIndex < forwardProjectionMatrix.rows()) && (colIndex < forwardProjectionMatrix.cols()) )	
  sparseforwardProjectionMatrixFile << std::endl << "Row " << rowIndex << " and column " << colIndex 
  << " is: " << forwardProjectionMatrix.value() << std::endl;
  }

   */


  /*
  // Print out the non-zero entries of the affine transformation matrix
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  affineMatrix.reset();
  std::ofstream sparseAffineMatrixFile("sparseAffineMatrix.txt");
  sparseAffineMatrixFile << std::endl << "The non-zero entries of the affine matrix are: " << std::endl;

  unsigned int rowIndex = 0;
  unsigned int colIndex = 0;

  while ( affineMatrix.next() )
  {
  rowIndex = affineMatrix.getrow();
  colIndex = affineMatrix.getcolumn();

  if ( (rowIndex < affineMatrix.rows()) && (colIndex < affineMatrix.cols()) )	
  sparseAffineMatrixFile << std::endl << "Row " << rowIndex << " and column " << colIndex << " is: " << affineMatrix.value() << std::endl;
  } 
   */


  // Covert the input image into the vnl vector form
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  typedef itk::ImageRegionConstIteratorWithIndex<InputImageType> ConstIteratorType;
  ConstIteratorType inputIterator( inImage, inImage->GetLargestPossibleRegion() );	

  VectorType inputImageVector(totalSize3D);

  unsigned int voxel3D = 0;
  InputImagePixelType voxelValue;
  for ( inputIterator.GoToBegin(); !inputIterator.IsAtEnd(); ++inputIterator)
  {
    voxelValue = inputIterator.Get();
    inputImageVector.put(voxel3D, (double) voxelValue);

    voxel3D++;	 
  }

  // std::ofstream inputImageVectorFile("inputImageVector.txt");
  // inputImageVectorFile << inputImageVector << " ";

  // Calculate the matrix/vector multiplication in order to get the forward projection of the original volume to simulated (y_1)
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  assert (!inputImageVector.is_zero());
  VectorType forwardProjectedVectorOne(totalSizeAllProjs);
  forwardProjectedVectorOne.fill(0.);

  // forwardProjectionMatrix.mult(inputImageVector, forwardProjectedVector);
  matrixProjector->CalculteMatrixVectorMultiplication(forwardProjectionMatrix, inputImageVector, forwardProjectedVectorOne);

  std::ofstream forwardProjectedVectorOneFile("forwardProjectedVectorOneFile.txt", std::ios::out | std::ios::app | std::ios::binary) ;
  forwardProjectedVectorOneFile << forwardProjectedVectorOne << " ";


  // Calculate the matrix/vector multiplication in order to get the affine transformation
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  assert (!inputImageVector.is_zero());
  VectorType affineTransformedVector(totalSize3D);
  affineTransformedVector.fill(0.);

  // affineMatrix.mult(inputImageVector, affineTransformedVector);
  affineTransformer->CalculteMatrixVectorMultiplication(affineMatrix, inputImageVector, affineTransformedVector);

  std::ofstream affineTransformedVectorFile("affineTransformedVectorFile.txt", std::ios::out | std::ios::app | std::ios::binary);
  affineTransformedVectorFile << affineTransformedVector << " ";


  // Calculate the matrix/vector multiplication in order to get the forward projection of the transformed volume to simulated (y_2)
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  assert (!affineTransformedVector.is_zero());
  VectorType forwardProjectedVectorTwo(totalSizeAllProjs);
  forwardProjectedVectorTwo.fill(0.);

  // forwardProjectionMatrix.mult(inputImageVector, forwardProjectedVector);
  matrixProjector->CalculteMatrixVectorMultiplication(forwardProjectionMatrix, affineTransformedVector, forwardProjectedVectorTwo);

  std::ofstream forwardProjectedVectorTwoFile("forwardProjectedVectorTwoFile.txt", std::ios::out | std::ios::app | std::ios::binary) ;
  forwardProjectedVectorTwoFile << forwardProjectedVectorTwo << " ";


  // Test the metric class
	// ~~~~~~~~~~~~~~~~~~~~~

	SimultaneousUnconMetricType::Pointer simultaneousUnconMetric = SimultaneousUnconMetricType::New();

	
	// Set the 3D reconstruction estimate input volume (x)
  simultaneousUnconMetric->SetInputVolume( inImageEstimation );

	// Set the inputs of (y_1) and (y_2)
	assert ( (!forwardProjectedVectorOne.is_zero()) && (!forwardProjectedVectorTwo.is_zero()) );
	simultaneousUnconMetric->SetInputTwoProjectionVectors( forwardProjectedVectorOne, forwardProjectedVectorTwo );

  // Set the temporary projection image
  simultaneousUnconMetric->SetInputTempProjections( outImage );

	// Set the number of the transformation parameters
  simultaneousUnconMetric->SetParameterNumber( parameters.Size() );

	// Set the total number of the voxels of the volume
  simultaneousUnconMetric->SetTotalVoxel( totalSize3D );

	// Set the total number of the pixels of the projection
  simultaneousUnconMetric->SetTotalPixel( totalSize2D );

	// Set the total number of the pixels of the projection
  simultaneousUnconMetric->SetTotalProjectionNumber( projectionNumber );

	//Set the total number of the pixels of the projection
  simultaneousUnconMetric->SetTotalProjectionSize( nPixels2D );

	// Set the geometry
	simultaneousUnconMetric->SetProjectionGeometry( geometry );


	// Return success after finish
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::cout << "Done";
	return EXIT_SUCCESS;   

}

