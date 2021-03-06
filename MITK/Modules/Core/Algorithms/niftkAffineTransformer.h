/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkAffineTransformer_h
#define niftkAffineTransformer_h

#include <itkObject.h>
#include <mitkBaseData.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <vtkSmartPointer.h>
#include <vtkLinearTransform.h>

#include "niftkAffineTransformParametersDataNodeProperty.h"
#include "niftkAffineTransformDataNodeProperty.h"

#include "niftkCoreExports.h"

namespace niftk
{

/**
 * \brief Class to contain all the ITK/MITK logic for the Affine Transformation Plugin,
 * to separate from AffineTransformationView to make unit testing easier.
 *
 * This class stores several AffineTransformDataNodeProperty on each data node:
 * <pre>
 * 1. The "Initial" transformation     = The transformation that was on the object, before this view added anything.
 *                                       So, if you load an image from file, this transformation is a copy of the geometry implied by the image header.
 *
 * 2. The "Incremental" transformation = Firstly, the dataNode->GetData()->GetGeometry() can only be composed with.
 *                                       So, we always have to calculate, for any change, the delta to be composed onto the existing transformation.
 *
 * 3. The "Pre-Loaded" transformation  = A transformation loaded from file.
 *                                       Loading a transformation from file also resets the GUI parameters.
 *                                       So, if you then add a rotation of 10 degrees about X axis, it is performed AFTER the transformation loaded from file.
 *
 * 4. The "Displayed" transformation   = The transformation that matches the GUI display.
 *                                       So, if you then add a rotation of 10 degrees about X axis, this transformation is just that.
 *
 * </pre>
 * and additionally a single AffineTransformParametersDataNodeProperty:
 * <pre>
 * 1. The "Displayed" parameters to match the "Displayed" transformation above.
 * </pre>
 * At no point are parameters derived or extracted from the affine transformation matrix,
 * as this is ambiguous and prone to numerical instability.
 */

class NIFTKCORE_EXPORT AffineTransformer : public itk::Object
{

public:

  /// \brief Simply stores the view name = "uk.ac.ucl.cmic.affinetransformview"
  static const std::string VIEW_ID;

  /// \brief See class introduction.
  static const std::string INITIAL_TRANSFORM_KEY;

  /// \brief See class introduction.
  static const std::string INCREMENTAL_TRANSFORM_KEY;

  /// \brief See class introduction.
  static const std::string PRELOADED_TRANSFORM_KEY;

  /// \brief See class introduction.
  static const std::string DISPLAYED_TRANSFORM_KEY;

  /// \brief See class introduction.
  static const std::string DISPLAYED_PARAMETERS_KEY;

  mitkClassMacroItkParent(AffineTransformer, itk::Object)
  itkNewMacro(AffineTransformer)

  /// Get / Set the "RotateAroundCenter" flag
  itkGetMacro(RotateAroundCenter, bool);
  itkSetMacro(RotateAroundCenter, bool);

  /// \brief Sets the mitk::DataStorage on this object.
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  /// \brief Gets the DataStorage pointer from this object.
  mitk::DataStorage::Pointer GetDataStorage() const;

  /// \brief Get the transform parameters from the current data node
  AffineTransformParametersDataNodeProperty::Pointer GetCurrentTransformParameters() const;

  /// \brief Get a transform matrix from the current data node
  vtkSmartPointer<vtkMatrix4x4> GetTransformMatrixFromNode(std::string which) const;

  /// \brief Computes and returns the transformation matrix based on the current set of parameters
  vtkSmartPointer<vtkMatrix4x4> GetCurrentTransformMatrix() const;

  ///  \brief Reset the transform to initial state
  void ResetTransform();

  /// \brief Called when a node changed.
  void OnNodeChanged(mitk::DataNode::Pointer node);

  /** \brief Slot for all changes to transformation parameters. */
  void OnParametersChanged(AffineTransformParametersDataNodeProperty::Pointer paramsProperty);

  /** \brief Slot for saving transform to disk. */
  void OnSaveTransform(std::string filename);

  /** \brief Slot for loading transform from disk. */
  void OnLoadTransform(std::string filename);

  /** \brief Slot for updating the direction cosines with the current transformation. */
  void OnApplyTransform();

  /** \brief Slot for resampling the current image. */
  void OnResampleTransform();

  /** Called by _InitialiseNodeProperties to initialise (to Identity) a specified transform property on a node. */
  void InitialiseTransformProperty(std::string name, mitk::DataNode::Pointer node);

  /** Called by OnSelectionChanged to setup a node with default transformation properties, if it doesn't already have them. */
  void InitialiseNodeProperties(mitk::DataNode::Pointer node);

  /** Called by _UpdateTransformationGeometry to set new transformations in the right properties of the node. */
  void UpdateNodeProperties(const vtkSmartPointer<vtkMatrix4x4> displayedTransformFromParameters,
                            const vtkSmartPointer<vtkMatrix4x4> incrementalTransformToBeComposed,
                            mitk::DataNode::Pointer);

  /** Called by _UpdateNodeProperties to update a transform property on a given node. */
  void UpdateTransformProperty(std::string name, vtkSmartPointer<vtkMatrix4x4> transform, mitk::DataNode::Pointer node);

  /** The transform loaded from file is applied to the current node, and all its children, and it resets the GUI parameters to Identity, and hence the DISPLAY_TRANSFORM and DISPLAY_PARAMETERS to Identity.*/
  void ApplyTransformToNode(const vtkSmartPointer<vtkMatrix4x4> transformFromFile, mitk::DataNode::Pointer node);

  /** \brief Applies a re-sampling to the current node. */
  void ApplyResampleToCurrentNode();

protected:

  AffineTransformer();
  virtual ~AffineTransformer();

  AffineTransformer(const AffineTransformer&); // Purposefully not implemented.
  AffineTransformer& operator=(const AffineTransformer&); // Purposefully not implemented.

  /// \brief Computes a new linear transform (as 4x4 transform matrix) from the parameters set through the UI.
  virtual vtkSmartPointer<vtkMatrix4x4> ComputeTransformFromParameters(void) const;

  /// \brief Updates the transform on the current node, and it's children.
  void UpdateTransformationGeometry();

private:
  /// \brief This member stores the actual transformation paramters updated from the UI
  AffineTransformParametersDataNodeProperty::Pointer m_CurrDispTransfProp;

  /// \brief This class needs a DataStorage to work.
  mitk::DataStorage::Pointer m_DataStorage;

  /// \brief Pointer to the current data node
  mitk::DataNode::Pointer    m_CurrentDataNode;

  /// \brief Flag to set rotation around center
  bool                       m_RotateAroundCenter;

  /// \brief Stores the coordinates of the center of rotation
  double                     m_CentreOfRotation[3];

  /// \brief Stores the coordinates transformation parameters
  double                     m_Translation[3];
  double                     m_Rotation[3];
  double                     m_Scaling[3];
  double                     m_Shearing[3];
};

}

#endif
