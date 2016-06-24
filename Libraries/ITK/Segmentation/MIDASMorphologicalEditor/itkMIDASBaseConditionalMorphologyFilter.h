/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef itkMIDASBaseConditionalMorphologyFilter_h
#define itkMIDASBaseConditionalMorphologyFilter_h

#include <itkImageToImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkConstNeighborhoodIterator.h>

namespace itk
{

/**
 * \class MIDASBaseConditionalMorphologyFilter
 * \brief Base class for MIDASConditionalErosionFilter and MIDASConditionalDilationFilter.
 * \ingroup midas_morph_editor
 */

  template <class TInputImage1, class TInputImage2, class TOutputImage>
  class ITK_EXPORT MIDASBaseConditionalMorphologyFilter : public ImageToImageFilter<TInputImage1, TOutputImage>
  {
  public:
    /** Standard class typedefs */
    typedef MIDASBaseConditionalMorphologyFilter           Self;
    typedef ImageToImageFilter<TInputImage1, TOutputImage> SuperClass;
    typedef SmartPointer<Self>                             Pointer;
    typedef SmartPointer<const Self>                       ConstPointer;

    /** Run-time type information (and related methods) */
    itkTypeMacro(MIDASBaseConditionalMorphologyFilter, ImageToImageFilter);

    /** Typedef to describe the type of pixel for the first image, which should be the binary mask image. */
    typedef typename TInputImage1::PixelType PixelType1;

    /** Typedef to describe the type of pixel for the second image, which should be a grey scale image. */
    typedef typename TInputImage2::PixelType PixelType2;

    /** Some additional typedefs */
    typedef TInputImage1                              InputMaskImageType;
    typedef typename InputMaskImageType::Pointer      InputMaskImagePointer;
    typedef typename InputMaskImageType::SizeType     InputMaskImageSizeType;
    typedef typename InputMaskImageType::RegionType   InputMaskImageRegionType;
    typedef typename InputMaskImageType::IndexType    InputMaskImageIndexType;

    typedef TInputImage2                              InputMainImageType;
    typedef typename InputMainImageType::Pointer      InputMainImagePointer;
    typedef typename InputMainImageType::SizeType     InputMainImageSizeType;
    typedef typename InputMainImageType::RegionType   InputMainImageRegionType;

    typedef TOutputImage                              OutputImageType;
    typedef typename OutputImageType::Pointer         OutputImagePointer;
    typedef typename OutputImageType::RegionType      OutputImageRegionType;
    typedef typename OutputImageType::SizeType        OutputImageSizeType;
    typedef typename OutputImageType::IndexType       OutputImageIndexType;
    typedef typename itk::ConstNeighborhoodIterator<OutputImageType>::RadiusType  OutputImageRadiusType;

    /** So we can copy the mask image. */
    typedef typename itk::ImageDuplicator<OutputImageType> MaskImageDuplicatorType;
    typedef typename MaskImageDuplicatorType::Pointer MaskImageDuplicatorPointer;

    /** Set/Get methods to set the region to keep. */
    void SetRegion(InputMaskImageRegionType region) { m_Region = region; m_UserSetRegion = true; this->Modified(); }
    InputMaskImageRegionType GetRegion() const { return m_Region; }

    /** Set/Get methods to set the number of iterations, which in subclasses could be erosions or dilations. Default 0. */
    itkSetMacro(NumberOfIterations, unsigned int);
    itkGetConstMacro(NumberOfIterations, unsigned int);

    /** Set/Get methods to set the output value for inside the region. Default 1. */
    itkSetMacro(InValue, PixelType1);
    itkGetConstMacro(InValue, PixelType1);

    /** Set/Get methods to set the output value for inside the region. Default 0. */
    itkSetMacro(OutValue, PixelType1);
    itkGetConstMacro(OutValue, PixelType1);

    /** Set the first input, which is the binary mask, that will be eroded/dilated. */
    void SetBinaryImageInput(const InputMaskImageType* image);

    /** Set the second input, for the grey scale image. */
    void SetGreyScaleImageInput(const InputMainImageType* image);

  protected:
    MIDASBaseConditionalMorphologyFilter();
    virtual ~MIDASBaseConditionalMorphologyFilter() {};
    void PrintSelf(std::ostream& os, Indent indent) const;

    /** The main method to implement the erosion in this single-threaded class */
    virtual void GenerateData();

    /** GenerateData() is implemented in this class for both sub-classess and calls DoFilter, (TemplateMethod pattern).  */
    virtual void DoFilter(InputMainImageType* inGrey, OutputImageType* inMask, OutputImageType *out) = 0;

    /** This method called after some initial sanity checks, but before the main filtering process runs. */
    virtual void BeforeFilter() {};

    /** This method called right at the end of the filter. */
    virtual void AfterFilter() {};

    /** This method called at the start of each iteration of filtering. */
    virtual void BeforeIteration() {};

    /** This method called at the end of each iteration of filtering. */
    virtual void AfterIteration() {};

    void CopyImageData(OutputImageType* in, OutputImageType *out);
    bool IsOnBoundaryOfImage(const OutputImageIndexType &voxelIndex, const OutputImageSizeType &size);
    bool IsOnBoundaryOfRegion(const OutputImageIndexType &voxelIndex, const OutputImageRegionType& region);

    InputMaskImageRegionType m_Region;
    bool                     m_UserSetRegion;

  private:
    MIDASBaseConditionalMorphologyFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented
    void DoOneIterationOfFilter(InputMainImageType* inGrey, OutputImageType* inMask, OutputImageType *out);

    PixelType1               m_InValue;
    PixelType1               m_OutValue;
    unsigned int             m_NumberOfIterations;

    // This is a member variable, so we don't repeatedly create/destroy the memory if the main filter is called repeatedly.
    MaskImageDuplicatorPointer m_TempImage;
  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMIDASBaseConditionalMorphologyFilter.txx"
#endif

#endif
