/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef MIDASMorphologicalSegmentorView_h
#define MIDASMorphologicalSegmentorView_h

#include <niftkBaseSegmentationView.h>

#include <mitkImage.h>

#include <MorphologicalSegmentorPipelineParams.h>
#include "MIDASMorphologicalSegmentorViewPreferencePage.h"
#include "MIDASMorphologicalSegmentorViewControlsImpl.h"
#include <niftkMIDASMorphologicalSegmentorPipelineManager.h>


/**
 * \class MIDASMorphologicalSegmentorView
 * \brief Provides the plugin component for the MIDAS brain segmentation functionality, originally developed at the Dementia Research Centre UCL.
 *
 * This plugin implements the paper:
 *
 * "Interactive algorithms for the segmentation and quantification of 3-D MRI brain scans"
 * by P. A. Freeborough, N. C. Fox and R. I. Kitney, published in
 * Computer Methods and Programs in Biomedicine 53 (1997) 15-25.
 *
 * \ingroup uk_ac_ucl_cmic_midasmorphologicalsegmentor_internal
 *
 * \sa niftkBaseSegmentationView
 * \sa MIDASMorphologicalSegmentorPipelineManager
 * \sa MorphologicalSegmentorPipeline
 * \sa MorphologicalSegmentorPipelineInterface
 * \sa MorphologicalSegmentorPipelineParams
 */
class MIDASMorphologicalSegmentorView : public niftkBaseSegmentationView
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  /// \brief Constructor, but most GUI construction is done in CreateQtPartControl().
  MIDASMorphologicalSegmentorView();

  /// \brief Copy constructor which deliberately throws a runtime exception, as no-one should call it.
  MIDASMorphologicalSegmentorView(const MIDASMorphologicalSegmentorView& other);

  /// \brief Destructor.
  virtual ~MIDASMorphologicalSegmentorView();

  /// \brief Each View for a plugin has its own globally unique ID.
  static const std::string VIEW_ID;

  /// \brief Returns VIEW_ID = uk.ac.ucl.cmic.midasmorphologicalsegmentor.
  virtual std::string GetViewID() const;

  /// \brief If the user hits the close icon, it is equivalent to a Cancel.
  virtual void ClosePart();

protected slots:
 
  /// \brief Called when the user hits the button "New segmentation", which creates the necessary reference data.
  void OnCreateNewSegmentationButtonPressed();

  /// \brief Called from MIDASMorphologicalSegmentorViewControlsImpl when thresholding sliders or spin boxes changed.
  void OnThresholdingValuesChanged(double lowerThreshold, double upperThreshold, int axialSliceNumber);

  /// \brief Called from MIDASMorphologicalSegmentorViewControlsImpl when erosion sliders or spin boxes changed.
  void OnErosionsValuesChanged(double upperThreshold, int numberOfErosions);

  /// \brief Called from MIDASMorphologicalSegmentorViewControlsImpl when dilation sliders or spin boxes changed.
  void OnDilationsValuesChanged(double lowerPercentage, double upperPercentage, int numberOfDilations);

  /// \brief Called from MIDASMorphologicalSegmentorViewControlsImpl when re-thresholding widgets changed.
  void OnRethresholdingValuesChanged(int boxSize);

  /// \brief Called from MIDASMorphologicalSegmentorViewControlsImpl when a tab changes.
  void OnTabChanged(int i);

  /// \brief Called from MIDASMorphologicalSegmentorViewControlsImpl when OK button is clicked, which should finalise / finish and accept the segmentation.
  void OnOKButtonClicked();

  /// \brief Called from MIDASMorphologicalSegmentorViewControlsImpl when Restart button is clicked, which means "back to start", like a "reset" button.
  void OnRestartButtonClicked();

  /// \brief Called from MIDASMorphologicalSegmentorViewControlsImpl when cancel button is clicked, which should mean "throw away" / "abandon" current segmentation.
  void OnCancelButtonClicked();

protected:

  /// \brief Registers the tools provided by this view.
  /// Registers the paintbrush tool.
  /// \param toolManager the tool manager where the tools should be registered
  void RegisterTools(mitk::ToolManager::Pointer toolManager) override;

  /// \brief Called by framework, this method creates all the controls for this view
  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Called by framework, sets the focus on a specific widget, but currently does nothing.
  virtual void SetFocus();

  /// \brief Connects the Qt signals from the GUI components to the methods in this class.
  virtual void CreateConnections();

  /// \brief For Morphological Editing, a Segmentation image should have a grey scale parent, and two binary children called SUBTRACTIONS_IMAGE_NAME and ADDITIONS_IMAGE_NAME.
  virtual bool IsNodeASegmentationImage(const mitk::DataNode::Pointer node);

  /// \brief For Morphological Editing, a Working image should be called either SUBTRACTIONS_IMAGE_NAME and ADDITIONS_IMAGE_NAME, and have a binary image parent.
  virtual bool IsNodeAWorkingImage(const mitk::DataNode::Pointer node);

  /// \brief For any binary image, we return true if the property midas.morph.stage is present, and false otherwise.
  virtual bool CanStartSegmentationForBinaryNode(const mitk::DataNode::Pointer node);

  /// \brief Assumes input is a valid segmentation node, then searches for the derived children of the node, looking for binary images called SUBTRACTIONS_IMAGE_NAME and ADDITIONS_IMAGE_NAME. Returns empty list if both not found.
  virtual mitk::ToolManager::DataVectorType GetWorkingDataFromSegmentationNode(const mitk::DataNode::Pointer node);

  /// \brief Assumes input is a valid working node, then searches for a binary parent node, returns NULL if not found.
  virtual mitk::DataNode* GetSegmentationNodeFromWorkingData(const mitk::DataNode::Pointer node);

  /// \brief Method to enable this and derived classes to turn widgets off/on
  virtual void EnableSegmentationWidgets(bool b);

  /// \brief Called when a node is removed.
  virtual void NodeRemoved(const mitk::DataNode* node);

  /// \brief Returns the name of the preferences node to look up.
  virtual QString GetPreferencesNodeName() { return MIDASMorphologicalSegmentorViewPreferencePage::PREFERENCES_NODE_NAME; }

  /// \brief \see QmitkAbstractView::OnSelectionChanged.
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);

  void onVisibilityChanged(const mitk::DataNode* node);

private:

  /// \brief Creates a node for storing the axial cut-off plane.
  mitk::DataNode::Pointer CreateAxialCutOffPlaneNode(const mitk::Image* referenceImage);

  /// \brief Looks up the reference image, and sets default parameter values on the segmentation node.
  void SetSegmentationNodePropsFromReferenceImage();

  /// \brief Sets the morphological controls to default values specified by reference image, like min/max intensity range, number of axial slices etc.
  void SetControlsFromReferenceImage();

  /// \brief Sets the morphological controls by the current property values stored on the segmentation node.
  void SetControlsFromSegmentationNodeProps();

  /// \brief Called when the segmentation is manually edited via the paintbrush tool.
  /// \param imageIndex tells which image has been modified: erosion addition / subtraction or dilation addition / subtraction.
  virtual void OnSegmentationEdited(int imageIndex);

  /// \brief Used to put the base class widgets, and these widgets above in a common layout.
  QGridLayout *m_Layout;

  /// \brief Container for the Morphological Controls Widgets (see this class).
  QWidget *m_ContainerForControlsWidget;

  /// \brief All the controls for the main Morphological Editor view part.
  MIDASMorphologicalSegmentorViewControlsImpl* m_MorphologicalControls;

  /// \brief As much "business logic" as possible is delegated to this class so we can unit test it, without a GUI.
  niftk::MIDASMorphologicalSegmentorPipelineManager::Pointer m_PipelineManager;

  /// \brief Keep local variable to update after the tab has changed.
  int m_TabIndex;
};

#endif
