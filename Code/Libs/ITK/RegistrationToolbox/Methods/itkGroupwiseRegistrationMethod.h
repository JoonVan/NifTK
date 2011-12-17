/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.
 
 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: $
 Revision          : $Revision: $
 Last modified by  : $Author:  $

 Original author   : j.hipwell@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef __itkGroupwiseRegistrationMethod_h
#define __itkGroupwiseRegistrationMethod_h

#include "itkProcessObject.h"
#include "itkImage.h"
#include "itkMeanVoxelwiseIntensityOfMultipleImages.h"
#include "itkImageRegistrationFilter.h"

namespace itk
{

/** \class GroupwiseRegistrationMethod
 * \brief A class to perform a generic group-wise registration.
 *
 * The inputs to the method are the 'n' images to be registered. Also
 * required are a std::vector of 'n' registration filters which have
 * been initialised, and an initial mean image to form the first
 * target for all the registrations.
 *
 * The output from the method is the mean image generated by averaging
 * the 'n' registered and transformed input images.
 */
template <typename TImageType, 
          unsigned int Dimension, 
          class TScalarType, 
          typename TDeformationScalar >
class ITK_EXPORT GroupwiseRegistrationMethod : public ImageSource< TImageType >
{
public:
  /** Standard class typedefs. */
  typedef GroupwiseRegistrationMethod  Self;
  typedef ProcessObject                Superclass;
  typedef SmartPointer<Self>           Pointer;
  typedef SmartPointer<const Self>     ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(GroupwiseRegistrationMethod, ProcessObject);

  /** Image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int, TImageType::ImageDimension);

  // Some convenient typedefs.
  typedef TImageType                       ImageType;
  typedef typename ImageType::Pointer      ImagePointer;
  typedef typename ImageType::ConstPointer ImageConstPointer;
  typedef typename ImageType::RegionType   ImageRegionType; 
  typedef typename ImageType::PixelType    ImagePixelType; 
  typedef typename ImageType::SpacingType  ImageSpacingType;
  typedef typename ImageType::IndexType    ImageIndexType;
  typedef typename ImageType::PointType    ImagePointType;

  typedef typename ImageRegionType::SizeType  ImageSizeType;

  /** Set/Get the image input of this process object.  */
  virtual void SetInput( const ImageType *image);
  virtual void SetInput( unsigned int, const TImageType * image);

  const ImageType * GetInput(void);
  const ImageType * GetInput(unsigned int idx);

  /// The image registration filter type
  typedef itk::ImageRegistrationFilter<ImageType, ImageType, 
                                       Dimension, TScalarType, TDeformationScalar> ImageRegistrationFilterType;
  
  typedef typename ImageRegistrationFilterType::Pointer ImageRegistrationFilterPointerType;
  
  typedef typename itk::MeanVoxelwiseIntensityOfMultipleImages<ImageType, 
                                                               ImageType> MeanVoxelwiseIntensityOfMultipleImagesType;

  itkSetObjectMacro( SumImagesFilter, MeanVoxelwiseIntensityOfMultipleImagesType );
  itkGetObjectMacro( SumImagesFilter, MeanVoxelwiseIntensityOfMultipleImagesType );

  itkSetMacro( NumberOfIterations, unsigned int );
  itkGetMacro( NumberOfIterations, unsigned int );

  /** Set the image registration filters */
  void SetRegistrationFilters( std::vector< ImageRegistrationFilterPointerType > &regnFilters ) {
    m_RegistrationFilters = regnFilters;
    this->Modified();
  }

  /** Initialise by setting the interconnects between the components. */
  virtual void Initialise() throw (ExceptionObject);
    
protected:
  GroupwiseRegistrationMethod();
  virtual ~GroupwiseRegistrationMethod() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  void ComputeInitialSumOfInputImages();

  virtual void GenerateInputRequestedRegion();
  void GenerateOutputInformation();
  
  /** Method that initiates the optimization process. This method should not be
   * called directly by the users. Instead, this method is intended to be
   * invoked internally by the StartRegistration() which is in turn invoked by
   * the Update() method. */
  void StartOptimization(void);

  /** Method invoked by the pipeline in order to trigger the computation of 
   * the registration. */
  void  GenerateData ();


private:
  GroupwiseRegistrationMethod(const Self&); // purposely not implemented
  void operator=(const Self&);	          // purposely not implemented

  /// Flag indicating that the initial sum of input images has been computed
  bool m_FlagInitialSumComputed;

  /// Flag indicating that the method has been initialised
  bool m_FlagInitialised;

  /// The number of iterations to perform
  unsigned int m_NumberOfIterations;

  ImageRegionType  m_OutRegion;
  ImageSizeType    m_OutSize;
  ImageSpacingType m_OutSpacing;
  ImagePointType   m_OutOrigin;

  /// The filter used to sum the input images
  typename MeanVoxelwiseIntensityOfMultipleImagesType::Pointer m_SumImagesFilter;

  /// The array of image registration filters
  std::vector< ImageRegistrationFilterPointerType > m_RegistrationFilters;

};


} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGroupwiseRegistrationMethod.txx"
#endif

#endif




