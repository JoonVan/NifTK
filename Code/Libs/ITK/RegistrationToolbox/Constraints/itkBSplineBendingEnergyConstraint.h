/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.
 
 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-09-14 11:37:54 +0100 (Wed, 14 Sep 2011) $
 Revision          : $Revision: 7310 $
 Last modified by  : $Author: ad $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#ifndef __itkBSplineBendingEnergyConstraint_h
#define __itkBSplineBendingEnergyConstraint_h

#include "itkConstraint.h"
#include "itkBSplineTransform.h"


namespace itk
{
  
/** 
 * \class BSplineBendingEnergyConstraint
 * \brief Calculated the bending energy, to be used as regulariser in FFD.
 *
 * In practice, you create this object, create your BSplineTransform,
 * and inject the BSplineTransform into this class.
 * When EvaluateContraint is called, this class delegates back to 
 * the BSplineTransform, as the BSplineTransform can calculate it's
 * own bending energy.
 * 
 * \ingroup RegistrationMetrics
 */
template <
    class TFixedImage,                   // Templated over the image type.
    class TScalarType,                   // Data type for scalars
    unsigned int NDimensions,            // Number of Dimensions i.e. 2D or 3D
    class TDeformationScalar             // The data type for the vector field
    >            
class ITK_EXPORT BSplineBendingEnergyConstraint : 
    public Constraint
{
public:
  /** Standard "Self" typedef. */
  typedef BSplineBendingEnergyConstraint Self;
  typedef Constraint                     Superclass;
  typedef SmartPointer<Self>             Pointer;
  typedef SmartPointer<const Self>       ConstPointer;
  typedef Superclass::MeasureType        MeasureType;
  typedef Superclass::DerivativeType     DerivativeType;
  typedef Superclass::ParametersType     ParametersType;

  /**  Type of the Transform . */
  typedef typename itk::BSplineTransform<TFixedImage, TScalarType, NDimensions, TDeformationScalar > TransformType;
  typedef typename TransformType::Pointer                                                            TransformPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro( BSplineBendingEnergyConstraint, Constraint );

  /** Calculates the bending energy. */
  virtual MeasureType EvaluateConstraint(const ParametersType & parameters);

  /** Calculates the derivative of the bending energy. */
  virtual void EvaluateDerivative(const ParametersType & parameters, DerivativeType & derivative ) const;
  
  /** Set/Get the Transfrom. */
  itkSetObjectMacro( Transform, TransformType );
  itkGetObjectMacro( Transform, TransformType );

protected:
  
  BSplineBendingEnergyConstraint();
  virtual ~BSplineBendingEnergyConstraint() {};

  void PrintSelf(std::ostream& os, Indent indent) const;

private:  
  
  BSplineBendingEnergyConstraint(const Self&); // purposely not implemented
  void operator=(const Self&);                 // purposely not implemented

  /** The transform we are evaluating. */
  TransformPointer m_Transform;
  
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBSplineBendingEnergyConstraint.txx"
#endif

#endif
