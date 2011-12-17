/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.
 
 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2010-05-28 22:05:02 +0100 (Fri, 28 May 2010) $
 Revision          : $Revision: 3326 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#ifndef __itkJEImageToImageMetric_h
#define __itkJEImageToImageMetric_h

#include "itkHistogramSimilarityMeasure.h"

namespace itk
{
/** 
 * \class JEImageToImageMetric
 * \brief Implements Joint Entropy of a histogram for a similarity measure.
 *
 * \ingroup RegistrationMetrics
 */
template < class TFixedImage, class TMovingImage > 
class ITK_EXPORT JEImageToImageMetric : 
    public HistogramSimilarityMeasure< TFixedImage, TMovingImage>
{
public:

  /** Standard class typedefs. */
  typedef JEImageToImageMetric                                            Self;
  typedef HistogramSimilarityMeasure<TFixedImage, TMovingImage >          Superclass;
  typedef SmartPointer<Self>                                              Pointer;
  typedef SmartPointer<const Self>                                        ConstPointer;
  typedef typename Superclass::FixedImageType::PixelType                  FixedImagePixelType;
  typedef typename Superclass::MovingImageType::PixelType                 MovingImagePixelType;  
  typedef typename Superclass::MeasureType                                MeasureType;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
 
  /** Run-time type information (and related methods). */
  itkTypeMacro(JEImageToImageMetric, HistogramSimilarityMeasure);

protected:
  
  JEImageToImageMetric() {};
  virtual ~JEImageToImageMetric() {};

  /**
   * In this method, we do any final aggregating,
   * which basically means "evaluate the histogram".
   * Filling the histogram is in the base class.
   */
  MeasureType FinalizeCostFunction()
    {
      return this->JointEntropy();
    }

private:
  JEImageToImageMetric(const Self&); // purposefully not implemented
  void operator=(const Self&);       // purposefully not implemented  
};

} // end namespace itk

#endif



