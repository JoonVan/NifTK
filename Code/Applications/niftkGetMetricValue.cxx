/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <itkLogHelper.h>
#include <itkImage.h>
#include <itkImageRegionConstIterator.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkImageFileReader.h>

/* ********************************************************************** */

typedef itk::Image<float,2> ImageType;
typedef itk::ImageFileReader<ImageType> ImageReaderType;
typedef itk::ImageRegionConstIterator<ImageType> IteratorType;
typedef itk::RescaleIntensityImageFilter<ImageType,ImageType> RescaleFilter;

/*!
 * \file niftkGetMetricValue.cxx
 * \page niftkGetMetricValue
 * \section niftkGetMetricValueSummary This program returns a metric value (NMI, MI, JE, SSD) between two input files
 */
void Usage(char *exec)
{
  niftk::itkLogHelper::PrintCommandLineHeader(std::cout);
  std::cout<<std::endl;
	std::cout<<"This program returns a metric value between two input files"<<std::endl;
	std::cout<< "Usage:\t"<< exec << " <inputFileName> <inputFileName> [metric] [bin number]"<<std::endl;
	std::cout<< "Metric:\t"<<"0:NMI | 1:MI | 2:JE | 3:SSD"<<std::endl;
	return;
}

int main(int argc, char **argv)
{
	if(argc<3){
		Usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	int metric = 0;
	int bin = 64;
	if(argc>4) metric=atoi(argv[3]);
	if(argc==5) bin=atoi(argv[4]);
	
	// Read first image
	ImageReaderType::Pointer firstReader = ImageReaderType::New();
	firstReader->SetFileName(argv[1]);
	try{
		firstReader->Update();
	}
	catch(itk::ExceptionObject  &err){
		std::cerr<<"Exception caught when reading the input image: "<< argv[1] <<std::endl;
		std::cerr<<"Error: "<<err<<std::endl;
		return EXIT_FAILURE;
	}
	// Read second image
	ImageReaderType::Pointer secondReader = ImageReaderType::New();
	secondReader->SetFileName(argv[2]);
	try{
		secondReader->Update();
	}
	catch(itk::ExceptionObject  &err){
		std::cerr<<"Exception caught when reading the input image: "<< argv[2] <<std::endl;
		std::cerr<<"Error: "<<err<<std::endl;
		return EXIT_FAILURE;
	}
	
	RescaleFilter::Pointer firstRescale = RescaleFilter::New();
	firstRescale->SetInput(firstReader->GetOutput());
	firstRescale->SetOutputMinimum(0.0f);
	firstRescale->SetOutputMaximum(bin-1);
	try{
		firstRescale->Update();
	}
	catch(itk::ExceptionObject &err){
		std::cerr<<"Reg:\tException caught when rescaling the image "<<argv[1]<<std::endl;
		std::cerr<<"Reg:\tError: "<<err<<std::endl;
		return EXIT_FAILURE;
	}	
	RescaleFilter::Pointer secondRescale = RescaleFilter::New();
	secondRescale->SetInput(secondReader->GetOutput());
	secondRescale->SetOutputMinimum(0.0f);
	secondRescale->SetOutputMaximum(bin-1);
	try{
		secondRescale->Update();
	}
	catch(itk::ExceptionObject &err){
		std::cerr<<"Reg:\tException caught when rescaling the image "<<argv[2]<<std::endl;
		std::cerr<<"Reg:\tError: "<<err<<std::endl;
		return EXIT_FAILURE;
	}
	
	IteratorType first(firstRescale->GetOutput(), firstRescale->GetOutput()->GetLargestPossibleRegion());
	IteratorType second(secondRescale->GetOutput(), secondRescale->GetOutput()->GetLargestPossibleRegion());
	
	if(metric<3){
		double *histo = (double*)calloc(bin*bin,sizeof(double));

		double voxelNumber=0;
		for(first.GoToBegin(),second.GoToBegin(); !first.IsAtEnd(); ++first,++second){
			histo[(int)first.Get()*bin + (int)second.Get()]++;
			voxelNumber++;
		}
		for(int i=0; i<bin*bin; i++) histo[i] /= voxelNumber;

		double fEntropy = 0.0;
		double sEntropy = 0.0;
		double jEntropy = 0.0;

		for(int t=0; t<bin; t++){
			double sum=0.0;
			int coord=t*bin;
			for(int r=0; r<bin; r++){
				sum += histo[coord++];
			}
			double logValue=0.0;
			if(sum)	logValue = log(sum);
			fEntropy -= sum*logValue;
		}
		for(int r=0; r<bin; r++){
			double sum=0.0;
			int coord=r;
			for(int t=0; t<bin; t++){
				sum += histo[coord];
				coord += bin;
			}
			double logValue=0.0;
			if(sum)	logValue = log(sum);
			sEntropy -= sum*logValue;
		}

		for(int tr=0; tr<bin*bin; tr++){
			double jointValue = histo[tr];
			double jointLog = 0.0;
			if(jointValue)	jointLog = log(jointValue);
			jEntropy -= jointValue*jointLog;
		}
		delete[] histo;

		if(metric==0) std::cout << (fEntropy+sEntropy)/jEntropy << std::endl;
		if(metric==1) std::cout << fEntropy+sEntropy-jEntropy << std::endl;
		if(metric==2) std::cout << jEntropy << std::endl;
	}
	
	if(metric==3){
		double ssd(0);
		for(first.GoToBegin(),second.GoToBegin(); !first.IsAtEnd(); ++first,++second){
			ssd += fabs(first.Get()-second.Get());
		}
		std::cout << ssd << std::endl;
	}

	return EXIT_SUCCESS;
}
