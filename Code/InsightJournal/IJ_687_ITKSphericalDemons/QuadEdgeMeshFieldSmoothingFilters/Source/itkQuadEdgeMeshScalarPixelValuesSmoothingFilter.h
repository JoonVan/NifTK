/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkNodeScalarGradientCalculator.h,v $
  Language:  C++
  Date:      $Date: 2010-05-26 10:55:12 +0100 (Wed, 26 May 2010) $
  Version:   $Revision: 3302 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkQuadEdgeMeshScalarPixelValuesSmoothingFilter_h
#define __itkQuadEdgeMeshScalarPixelValuesSmoothingFilter_h

#include "itkQuadEdgeMeshToQuadEdgeMeshFilter.h"
#include "itkQuadEdgeMeshParamMatrixCoefficients.h"

namespace itk
{

/**
 * \class QuadEdgeMeshScalarPixelValuesSmoothingFilter
 * \brief This filter smooths scalar pixel values associated with points.
 *
 * This filter was based on the filter provided by 
 * Arnaud Gelas, Alex Gouaillard and Sean Megason in their Insight Journal paper
 * http://hdl.handle.net/1926/1518
 * http://www.insight-journal.org/browse/publication/313
 *
 * The difference between this current filter and the one above is that this
 * filter smooths the values associated with the points (PointData) without
 * changing the actual positions of the points in space, while the filter above
 * smooths the point positions while leaving unchanged the pixel values
 * associated with the points.
 *
 * This filter expects the PixelType to be of Scalar type. At every node, the
 * scalar values be averaged using a weighted sum. The smoothing process is
 * performed for a user-specified number of iterations.
 *
 * A full description of this filter is available in the TMI paper:
 *
 * "Spherical Demons: Fast Diffeomorphic Landmark-Free Surface Registration"
 *
 * by
 *       B.T. Thomas Yeo, Mert R. Sabuncu, Tom Vercauteren, 
 *       Nicholas Ayache, Bruce Fischl, Polina Golland.
 *
 * \sa QuadEdgeMeshVectorPixelValuesSmoothingFilter
 * \ingroup MeshFilters
 *
 */
template< class TInputMesh, class TOutputMesh >
class QuadEdgeMeshScalarPixelValuesSmoothingFilter :
  public QuadEdgeMeshToQuadEdgeMeshFilter< TInputMesh, TOutputMesh >
{
public:
  typedef QuadEdgeMeshScalarPixelValuesSmoothingFilter  Self;
  typedef QuadEdgeMeshToQuadEdgeMeshFilter< 
    TInputMesh, TOutputMesh >                           Superclass;
  typedef SmartPointer< Self >                          Pointer;
  typedef SmartPointer< const Self >                    ConstPointer;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( QuadEdgeMeshScalarPixelValuesSmoothingFilter, QuadEdgeMeshToQuadEdgeMeshFilter );

  /** New macro for creation of through a Smart Pointer   */
  itkNewMacro( Self );

  typedef TInputMesh                                       InputMeshType;
  typedef typename InputMeshType::Pointer                  InputMeshPointer;
  typedef typename InputMeshType::PixelType                InputPixelType;
  typedef typename InputMeshType::PointDataContainer       InputPointDataContainer;

  typedef TOutputMesh                                        OutputMeshType;
  typedef typename OutputMeshType::Pointer                   OutputMeshPointer;
  typedef typename OutputMeshType::EdgeCellType              OutputEdgeCellType;
  typedef typename OutputMeshType::PolygonCellType           OutputPolygonCellType;
  typedef typename OutputMeshType::QEType                    OutputQEType;
  typedef typename OutputMeshType::PointIdentifier           OutputPointIdentifier;
  typedef typename OutputMeshType::PointType                 OutputPointType;
  typedef typename OutputPointType::VectorType               OutputVectorType;
  typedef typename OutputPointType::CoordRepType             OutputCoordType;
  typedef typename OutputMeshType::PointsContainer           OutputPointsContainer;
  typedef typename OutputMeshType::PointsContainerPointer    OutputPointsContainerPointer;
  typedef typename OutputMeshType::PointsContainerIterator   OutputPointsContainerIterator;
  typedef typename OutputMeshType::CellsContainerPointer     OutputCellsContainerPointer;
  typedef typename OutputMeshType::CellsContainerIterator    OutputCellsContainerIterator;
  typedef typename OutputMeshType::PointDataContainer        OutputPointDataContainer;
  typedef typename OutputMeshType::PointDataContainerPointer OutputPointDataContainerPointer;
  typedef typename OutputMeshType::PixelType                 OutputPixelType;

  itkStaticConstMacro( PointDimension, unsigned int, OutputMeshType::PointDimension );

  /** The smoothing filter will run iteratively until reaching this maximum
   * number of iterations. Emprical observartions indicate that ten iterations
   * are enough for typical deformation fields, but of course this would depend
   * on the process that you used for generating your deformation field. 
   */
  itkSetMacro( MaximumNumberOfIterations, unsigned long );
  itkGetMacro( MaximumNumberOfIterations, unsigned long );

  /** Factor that controls the degree of Smoothing. Large values of Lambda
   * result is stronger smoothing.  The Lambda factor is used to compute the
   * weights of the smoothing kernel as
   *
   * \f$
   * \frac{ \exp( \frac{-1}{2 \lambda} }{ 1 + \abs{ N_i } \exp( \frac{-1}{2 \lambda} }
   * \f$
   *
   * where \f$ N_i \f$ is the number of neighbor nodes around node \f$ i \f$.
   *
   * The default value of Lambda is 1.0.
   *
   * The filter assumes that the neighbor nodes of any given nodes are located
   * at similar distances, and therefore uses the same weight for each one of
   * the neighbor values when computing their weighted average.
   *
   */
  itkSetMacro( Lambda, double );
  itkGetMacro( Lambda, double );


protected:
  QuadEdgeMeshScalarPixelValuesSmoothingFilter();
  ~QuadEdgeMeshScalarPixelValuesSmoothingFilter();

  void GenerateData();

private:

  QuadEdgeMeshScalarPixelValuesSmoothingFilter( const Self& ); //purposely not implemented
  void operator=( const Self& ); //purposely not implemented

  unsigned long             m_MaximumNumberOfIterations;
  double                    m_Lambda;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkQuadEdgeMeshScalarPixelValuesSmoothingFilter.txx"
#endif

#endif
