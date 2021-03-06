/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/


#ifndef itkSimultaneousReconstructionAndRegistrationMethod_h
#define itkSimultaneousReconstructionAndRegistrationMethod_h

#include <itkProcessObject.h>
#include <itkImage.h>
#include <itkSimultaneousReconstructionRegistrationMetric.h>
#include <itkSingleValuedNonLinearOptimizer.h>
#include <itkProjectionGeometry.h>
#include <itkSimultaneousReconAndRegnUpdateCommand.h>


namespace itk
{

/** \class SimultaneousReconstructionAndRegistrationMethod
 * \brief Base class for Image Reconstruction Methods
 *
 * This class defines the generic interface for a reconstruction method.
 *
 * This class is templated over the type of the images to be
 * reconstructed. 
 *
 * The method uses a generic optimizer that can
 * be selected at run-time. The only restriction for the optimizer is
 * that it should be able to operate in single-valued cost functions
 * given that the metrics used to compare images provide a single 
 * value as output.
 */
template <class IntensityType = double>
class ITK_EXPORT SimultaneousReconstructionAndRegistrationMethod : public ProcessObject 
{
public:
  /** Standard class typedefs. */
  typedef SimultaneousReconstructionAndRegistrationMethod  Self;
  typedef ProcessObject  Superclass;
  typedef SmartPointer<Self>   Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  typedef itk::SimultaneousReconAndRegnUpdateCommand     SimultaneousReconAndRegnUpdateCommandType;
  typedef SimultaneousReconAndRegnUpdateCommand::Pointer SimultaneousReconAndRegnUpdateCommandPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(SimultaneousReconstructionAndRegistrationMethod, ProcessObject);

  // Some convenient typedefs.

  /** Intensity type has to be double because the optimizer expects
  the parameters (intensities) to be double */

  typedef Image<IntensityType, 3>                         InputProjectionVolumeType;
  typedef typename InputProjectionVolumeType::Pointer     InputProjectionVolumePointer;
  typedef typename InputProjectionVolumeType::RegionType  InputProjectionVolumeRegionType;
  typedef typename InputProjectionVolumeType::PixelType   InputProjectionVolumePixelType;
  typedef typename InputProjectionVolumeType::SizeType    InputProjectionVolumeSizeType;
  typedef typename InputProjectionVolumeType::SpacingType InputProjectionVolumeSpacingType;
  typedef typename InputProjectionVolumeType::PointType   InputProjectionVolumePointType;

  typedef Image<IntensityType, 3>                   ReconstructionType;
  typedef typename ReconstructionType::Pointer      ReconstructionPointer;
  typedef typename ReconstructionType::RegionType   ReconstructionRegionType;
  typedef typename ReconstructionType::PixelType    ReconstructionPixelType;
  typedef typename ReconstructionType::SizeType     ReconstructionSizeType;
  typedef typename ReconstructionType::SpacingType  ReconstructionSpacingType;
  typedef typename ReconstructionType::PointType    ReconstructionPointType;
  typedef typename ReconstructionType::IndexType    ReconstructionIndexType;

  /// Type of the optimizer.
  typedef SingleValuedNonLinearOptimizer           OptimizerType;
  typedef typename    OptimizerType::Pointer       OptimizerPointer;

  /// The type of the metric
  typedef SimultaneousReconstructionRegistrationMetric<IntensityType> MetricType;
  typedef typename MetricType::Pointer          											MetricPointer;

  /// The projection geometry type
  typedef itk::ProjectionGeometry<IntensityType>   ProjectionGeometryType;
  typedef typename ProjectionGeometryType::Pointer ProjectionGeometryPointer;

  /** Type of the optimisation parameters (reconstructed intensities).
   *  This is the same type used to represent the search space of the
   *  optimization algorithm */
  typedef typename MetricType::ParametersType    ParametersType;

  /** Type for the output: Using Decorator pattern for enabling
   *  the reconstructed volume to be passed in the data pipeline */
  typedef ReconstructionType                               ReconstructionOutputType;
  typedef typename ReconstructionOutputType::Pointer       ReconstructionOutputPointer;
  typedef typename ReconstructionOutputType::ConstPointer  ReconstructionOutputConstPointer;
  
  /** Set/Get the Optimizer. */
  itkSetObjectMacro( Optimizer,  OptimizerType );
  itkGetObjectMacro( Optimizer,  OptimizerType );

  /** Set/Get the Metric. */
  itkSetObjectMacro( Metric, MetricType );
  itkGetObjectMacro( Metric, MetricType );

  /** Set/Get the Projection Geometry. */
  itkSetObjectMacro( ProjectionGeometry, ProjectionGeometryType );
  itkGetObjectMacro( ProjectionGeometry, ProjectionGeometryType );

  /** Set/Get the SimultaneousReconAndRegnUpdateCommand. */
  itkSetObjectMacro( SimultaneousReconAndRegnUpdateCommand, SimultaneousReconAndRegnUpdateCommandType );
  itkGetObjectMacro( SimultaneousReconAndRegnUpdateCommand, SimultaneousReconAndRegnUpdateCommandType );

  /// Set the 3D reconstruction estimate volume 
  void SetReconEstimate( ReconstructionType *im3D);

  /// Update the 3D reconstruction estimate volume 
  void UpdateReconstructionEstimate( ReconstructionType *im3D);
  /** Update the 3D reconstruction estimate volume with the average of
      the existing estimate and the supplied volume. */
  void UpdateReconstructionEstimateWithAverage( ReconstructionType *im3D);
  /// Update the initial optimisation parameters
  void UpdateInitialParameters(void);

  /// Set the fixed image volume of projection images
  bool SetInputFixedImageProjections( InputProjectionVolumeType *imFixedProjections);
  /// Set the moving image volume of projection images
  bool SetInputMovingImageProjections( InputProjectionVolumeType *imMovingProjections);

  /// Set the size, resolution and origin of the reconstructed image
  void SetReconstructedVolumeSize(ReconstructionSizeType &reconSize) {m_ReconstructedVolumeSize = reconSize;};
  void SetReconstructedVolumeSpacing(ReconstructionSpacingType &reconSpacing) {m_ReconstructedVolumeSpacing = reconSpacing;};
  void SetReconstructedVolumeOrigin(ReconstructionPointType &reconOrigin) {m_ReconstructedVolumeOrigin = reconOrigin;};

  /** Initialise by setting the interconnects between the components. */
  virtual void Initialise() throw (ExceptionObject);

  /** Returns the input image  */
  InputProjectionVolumeType *GetInput();
  /** Returns the image resulting from the reconstruction process  */
  ReconstructionOutputType *GetOutput();

  /** Returns the image resulting from the reconstruction process  */
  ReconstructionOutputType *GetReconstructedVolume() const;

  /** Make a DataObject of the correct type to be used as the specified
   * output. */
  virtual DataObjectPointer MakeOutput(unsigned int idx);

  /** Method to return the latest modified time of this object or
   * any of its cached ivars */
  unsigned long GetMTime() const;  

  /** Set/Get the 'update 3D reconstruction estimate volume with average' flag */
  void SetFlagUpdateReconEstimateWithAverage( bool flag) {m_FlagUpdateReconEstimateWithAverage = flag; this->Modified();}
  bool GetFlagUpdateReconEstimateWithAverage( void ) {return m_FlagUpdateReconEstimateWithAverage;}

  /// Set the number of combined registration-reconstruction iterations to perform
  void SetNumberOfReconRegnIterations(unsigned int n) {
    m_NumberOfReconRegnIterations = n;
    this->Modified();
  }
  /// Get the number of combined registration-reconstruction iterations to perform
  void GetNumberOfReconRegnIterations(void) {
    return m_NumberOfReconRegnIterations;
  }
    
protected:
  SimultaneousReconstructionAndRegistrationMethod();
  virtual ~SimultaneousReconstructionAndRegistrationMethod() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** We avoid propagating the input region to the output by
  overloading this function */
  virtual void GenerateOutputInformation() {};
  
  /** Method that initiates the reconstruction. This will Initialise and ensure
   * that all inputs the registration needs are in place, via a call to 
   * Initialise() will then start the optimization process via a call to 
   * StartOptimization()  */
  void StartReconstruction(void);

  /** Method that initiates the optimization process. This method should not be
   * called directly by the users. Instead, this method is intended to be
   * invoked internally by the StartReconstruction() which is in turn invoked by
   * the Update() method. */
  void StartOptimization(void);

  /** Method invoked by the pipeline in order to trigger the computation of 
   * the reconstruction. */
  void  GenerateData ();


private:
  SimultaneousReconstructionAndRegistrationMethod(const Self&); // purposely not implemented
  void operator=(const Self&);	          // purposely not implemented
  
  bool                             m_FlagInitialised;

  OptimizerPointer                 m_Optimizer;
  MetricPointer                    m_Metric;
  ProjectionGeometryPointer        m_ProjectionGeometry;

  InputProjectionVolumePointer     m_ProjectionImagesFixed;
	InputProjectionVolumePointer     m_ProjectionImagesMoving;
  ReconstructionPointer            m_EnhancedAsOneReconstructor;

  ParametersType                   m_InitialParameters;
  ParametersType                   m_LastParameters;
    
  ReconstructionSizeType           m_ReconstructedVolumeSize;
  ReconstructionSpacingType        m_ReconstructedVolumeSpacing;
  ReconstructionPointType          m_ReconstructedVolumeOrigin;

  /* Flag indicating whether to update the 3D reconstruction estimate volume with the average of
      the existing estimate and the supplied volume. */
  bool m_FlagUpdateReconEstimateWithAverage;

  // The number of combined registration-reconstruction iterations to perform
  unsigned int m_NumberOfReconRegnIterations;

  /** To print out the reconstruction status as we go. */
  SimultaneousReconAndRegnUpdateCommandPointer m_SimultaneousReconAndRegnUpdateCommand;

};


} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSimultaneousReconstructionAndRegistrationMethod.txx"
#endif

#endif




