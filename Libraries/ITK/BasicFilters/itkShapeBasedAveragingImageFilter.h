/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef itkShapeBasedAveragingImageFilter_h
#define itkShapeBasedAveragingImageFilter_h
 
#include <itkImageToImageFilter.h>
#include <stdlib.h>
#include <time.h>

namespace itk
{
  
/**
 * \class ShapeBasedAveragingImageFilter.
 * \brief Combines several segmentations/shapes into an average shape according to the
 * Shaped-Based Averaging, Rohlfing and Maurer, TMI, Jan 2007. 
 */
template<class TInputImage, class TOutputImage>
class ITK_EXPORT ShapeBasedAveragingImageFilter: 
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /**
   * House keeping for the object factory. 
   */ 
  typedef ShapeBasedAveragingImageFilter Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;
  /**
   * More typedefs. 
   */
  typedef Image<float, TInputImage::ImageDimension> FloatImageType; 
  typedef FloatImageType AverageDistanceMapType; 
  /**
   * Mean mode types. 
   */
  typedef enum 
  {
    // Simple mean. 
    MEAN = 0, 
    // Median. 
    MEDIAN = 1, 
    // Interquartile mean - more robust. 
    INTERQUARTILE_MEAN = 2,
    CORRECT_INTERQUARTILE_MEAN = 3
  } MeanModeType;
  /** 
   * Method for creation through the object factory. 
   */
  itkNewMacro(Self);  
  /** 
   * Runtime information support. 
   */
  itkTypeMacro(ShapeBasedAveragingImageFilter, ImageToImageFilter);
  /**
   * Unset label for undecided pixels. 
   */
  void UnsetLabelForUndecidedPixels() { this->m_IsUserDefinedLabelForUndecidedPixels = false; }
  /**
   * Set the label for undecided pixels. 
   * By default, the label used for undecided pixels is the maximum label value used in the input images plus one. 
   */
  void SetLabelForUndecidedPixels(typename TOutputImage::PixelType value) 
  {
    this->m_IsUserDefinedLabelForUndecidedPixels = true;
    this->m_LabelForUndecidedPixels = value; 
  }
  /**
   * Set mean mode. 
   */
  itkSetMacro(MeanMode, MeanModeType); 
  /**
   * Get the average distance map.
  */
  const FloatImageType* GetAverageDistanceMap() const
  {
    return this->m_AverageDistanceMap;
  }
  /**
   * Get the variability map.
   */
  const FloatImageType* GetVariabilityMap() const
  {
    return this->m_VariabilityMap;
  }
  /**
    * Get the probability map.
    */
  const FloatImageType* GetProbabilityMap() const
  {
    return this->m_ProbabilityMap;
  }
  FloatImageType* GetProbabilityMap()
  {
    return this->m_ProbabilityMap;
  }
  /**
   * Compute MRF.
   */
  void ComputeMRF(FloatImageType* probabilityMap, double mrf, int numberOfIterations);
      
protected:
  /**
   * Constructor. 
   */
  ShapeBasedAveragingImageFilter() : m_IsUserDefinedLabelForUndecidedPixels(false), m_LabelForUndecidedPixels(0), m_MeanMode(MEAN) 
  { 
    srand(time(NULL)); 
  }
  /**
   * Destructor. 
   */
  virtual ~ShapeBasedAveragingImageFilter() {}
  /**
   * Do the dirty work. 
   */
  void GenerateData();
  /**
   * Reliability of the input segmentation. 
   */
  void CalculateReliability(); 
  /**
   *  Variance of the distance map. 
   */
  double CalculateVariance(typename AverageDistanceMapType::Pointer averageDistanceMap); 
  
protected:  
  /**
   * The number of labels we have. 
   */
  bool m_IsUserDefinedLabelForUndecidedPixels; 
  /**
   * Label for undicided pixels. 
   */
  typename TOutputImage::PixelType m_LabelForUndecidedPixels; 
  /**
   * Option to use mean, median or interquartile mean. 
   */
  int m_MeanMode;
  /**
   * Input segmentation reliability. 
   */
  std::vector<double> m_SegmentationReliability; 
  /**
    * The average distance and minimum distance map.
    */
  typename FloatImageType::Pointer m_AverageDistanceMap;  
  /**
   * The variability of the distance.
   */
  typename FloatImageType::Pointer m_VariabilityMap;
  /**
   * The unnormalised probability map.
   */
  typename FloatImageType::Pointer m_ProbabilityMap;


private:
  /**
   * Prohibited copy and assingment. 
   */
  ShapeBasedAveragingImageFilter(const Self&); 
  void operator=(const Self&); 
      

}; 

    
} // end of namespace itk.

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkShapeBasedAveragingImageFilter.txx"
#endif


#endif // ITKSHAPEBASEDAVERAGINGIMAGEFILTER_H_
             
             
             
             
             

             
