/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef itkGE5000_TomosynthesisGeometry_h
#define itkGE5000_TomosynthesisGeometry_h

#include "itkProjectionGeometry.h"

namespace itk {

/** \class GE5000_TomosynthesisGeometry
 *  \brief Class to calculate the geometry of a GE tomosynthesis
 *  machine.
 */
template <class IntensityType = float>
class ITK_EXPORT GE5000_TomosynthesisGeometry :
    public ProjectionGeometry<IntensityType>
{
public:

  /** Standard class typedefs. */
  typedef GE5000_TomosynthesisGeometry      Self;
  typedef ProjectionGeometry<IntensityType> Superclass;
  typedef SmartPointer<Self>                Pointer;
  typedef SmartPointer<const Self>          ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GE5000_TomosynthesisGeometry, ProjectionGeometry);

  /** Some convenient typedefs. */
  typedef typename Superclass::ProjectionSizeType            ProjectionSizeType;
  typedef typename Superclass::ProjectionSpacingType         ProjectionSpacingType;

  typedef typename Superclass::VolumeSizeType                VolumeSizeType;
  typedef typename Superclass::VolumeSpacingType             VolumeSpacingType;

  typedef typename Superclass::EulerAffineTransformType                  EulerAffineTransformType;
  typedef typename Superclass::EulerAffineTransformPointerType           EulerAffineTransformPointerType;

  typedef typename Superclass::PerspectiveProjectionTransformType        PerspectiveProjectionTransformType;
  typedef typename Superclass::PerspectiveProjectionTransformPointerType PerspectiveProjectionTransformPointerType;

  /** Return a pointer to the perspective projection matrix for
      projection 'i'. */
  virtual PerspectiveProjectionTransformPointerType GetPerspectiveTransform(int i);

  /** Return a pointer to the affine transformation matrix for
      projection 'i'. */
  virtual EulerAffineTransformPointerType GetAffineTransform(int i);

  /// Return the number of projections for this geometry
  virtual unsigned int GetNumberOfProjections(void) { return 11; }


protected:
  GE5000_TomosynthesisGeometry();
  virtual ~GE5000_TomosynthesisGeometry() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /// Calculate the projection normal position
  double CalcNormalPosition(double alpha);


private:
  GE5000_TomosynthesisGeometry(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGE5000_TomosynthesisGeometry.txx"
#endif

#endif
