/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: $
 Revision          : $Revision: $
 Last modified by  : $Author: ad $

 Original author   : a.duttaroy@cs.ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
#include <iostream>
#include <memory>
#include <math.h>
#include "itkImage.h"
#include "itkMIDASHelper.h"
/**
 * Basic test for
 */
int itkLimitByRegionFunctionTest(int argc, char * argv[])
{

  const unsigned int Dimension = 2;
  typedef int PixelType;
  typedef itk::Image<PixelType, Dimension> ImageType;

  ImageType::Pointer inputImage = ImageType::New();
  typedef ImageType::IndexType    IndexType;
  typedef ImageType::SizeType     SizeType;
  typedef ImageType::RegionType   RegionType;

  //Create a 4x4 image
  SizeType imageSize;
  imageSize[0] = 4;
  imageSize[1] = 4;

  IndexType startIndex;
  startIndex[0] = 0;
  startIndex[1] = 0;

  RegionType imageRegion;
  imageRegion.SetIndex(startIndex);
  imageRegion.SetSize(imageSize);

  inputImage->SetRegions(imageRegion);
  inputImage->Allocate();
  inputImage->FillBuffer(1);

  imageSize[0] = 2;
  imageSize[1] = 2;
  startIndex[0] = 1;
  startIndex[1] = 1;

  RegionType limitingRegion;
  limitingRegion.SetIndex(startIndex);
  limitingRegion.SetSize(imageSize);

  itk::LimitMaskByRegion<ImageType>(inputImage.GetPointer(), limitingRegion, 0);

  int total = 0;
  IndexType outputIndex;

  for (int y = 0; y < 4; y++)
  {
    for (int x = 0; x < 4; x++)
    {
      outputIndex[0] = x;
      outputIndex[1] = y;

      std::cerr << "outputIndex=" << outputIndex << ", outputValue="<< inputImage->GetPixel(outputIndex) << std::endl;
      total += inputImage->GetPixel(outputIndex);
    }
  }

  if (total == 4)
  {
    return EXIT_SUCCESS;
  }
  else
  {
    std::cerr << "Expected 4, but got " << total << std::endl;
    return EXIT_FAILURE;
  }
}
