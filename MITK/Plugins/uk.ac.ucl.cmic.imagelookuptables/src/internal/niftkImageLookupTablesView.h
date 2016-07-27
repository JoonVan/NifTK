/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkImageLookupTablesView_h
#define niftkImageLookupTablesView_h

#include "ui_ImageLookupTablesViewControls.h"

#include <niftkBaseView.h>
#include <mitkLevelWindowManager.h>
#include <mitkDataNode.h>
#include <mitkImage.h>

class QWidget;
class QSlider;
class vtkLookupTable;

namespace niftk
{

class LookupTableManager;

/**
 * \class ImageLookupTablesView
 * \brief Provides a simple GUI enabling the selection of window, level, min, max and a choice of lookup tables.
 * \ingroup uk_ac_ucl_cmic_imagelookuptables_internal
 */
class ImageLookupTablesView : public niftk::BaseView
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  explicit ImageLookupTablesView();
  ImageLookupTablesView(const ImageLookupTablesView& other);
  virtual ~ImageLookupTablesView();

  /**
   * \brief Each view for a plugin has its own globally unique ID.
   */
  static const QString VIEW_ID;

  /**
   * \brief Called by framework, this method creates all the controls for this view.
   */
  virtual void CreateQtPartControl(QWidget *parent) override;

  /**
   * \brief Called by framework when DataManager's selection has changed.
   */
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                   const QList<mitk::DataNode::Pointer>& nodes ) override;
protected:

  /**
   * \brief \see niftk::BaseView::Activated()
   */
  virtual void Activated() override;

  /**
   * \brief Called by framework, sets the focus on a specific widget.
   */
  virtual void SetFocus() override;

  /**
   * \brief We listen to the Level/Window property on the registered image, so this callback updates this GUI when the watched property changes.
   */
  virtual void OnPropertyChanged(const itk::EventObject&);

  /**
   * \brief Called when the user toggles the opacity control properties.
   */
  virtual void OnLookupTablePropertyChanged(const itk::EventObject&);

  /**
   * \brief Actually updates the GUI when property changes.
   */
  virtual void OnPropertyChanged();

protected slots:

  /**
   * \brief Called when a different image is selected or the min/max intensity limits has changed,
   * which affects the actual minimum and maximum values that the sliders can reach.
   */
  void OnRangeChanged();

  /**
   * \brief Called when the min/max slider has changed.
   */
  void OnWindowBoundsChanged();

  /**
   * \brief Called when the window/level slider has changed.
   */
  void OnLevelWindowChanged();

  /**
   * \brief Called when the lookup table combo box value changes, trigger the lookup table to be swapped.
   */
  void OnLookupTableComboBoxChanged(int comboBoxIndex);

  /**
   * \brief Called when the reset button is pressed which will recalculate the minimum and maximum intensity values, and set the minimum and maximum value on the sliders.
   */
  void OnResetButtonPressed();

  /**
   * \brief Called when the load button is pressed which will display a dialog allowing the user to load a lookup table
   */
  void OnLoadButtonPressed();

  /**
   * \brief Called when the save button is pressed which will display a dialog allowing the user to save the current lookup table
   */
  void OnSaveButtonPressed();

  /**
   * \brief Called when the create new lookup table is pressed which will create an unscaled lookup table that is empty
   */
  void OnNewButtonPressed();

  /**
   * \brief Add a label to the current label map
   */
  void OnAddLabelButtonPressed();

  /**
   * \brief Remove the selected labels to the current label map
   */
  void OnRemoveLabelButtonPressed();

  /**
   * \brief Move the selected labels up one place in the list
   */
  void OnMoveLabelUpButtonPressed();

  /**
   * \brief Move the selected labels down one place in the list
   */
  void OnMoveLabelDownButtonPressed();

  /**
   * \brief Change color of pressed label color
   */
  void OnColorButtonPressed(int);

  /**
   * \brief Change name or value of selected cell
   */
  void OnLabelMapTableCellChanged(int, int);

private:

  /**
   * \brief Creation of the connections of widgets to slots.
   */
  void CreateConnections();

  /**
   * \brief Checks the GUI selection for a valid non-null, non-helper image.
   * \see QmitkAbstractView::IsCurrentSelectionValid() which just checks the selection service to see if something is selected.
   */
  bool IsSelectionValid(const QList<mitk::DataNode::Pointer>& nodes);

  /**
   * \brief Registers the given node, as the one we are tracking.
   */
  void Register(const mitk::DataNode::Pointer node);

  /**
   * \brief Unregisters the given node.
   */
  void Unregister();

  /**
   * \brief Called when a node is successfully registered, to initialize/reset the controls to appropriate ranges.
   */
  void DifferentImageSelected();

  /**
   * \brief Makes the GUI controls reflect the values on the level window property.
   */
  void UpdateGuiFromLevelWindow();

  /**
   * \brief Blocks all signals.
   */
  void BlockSignals(bool b);

  /**
   * \brief Enables/Disables controls.
   */
  void EnableControls(bool b);

  /**
   * \brief Enable/Disable controls related to scaling.
   */
  void EnableScaleControls(bool b);
;

  /**
   * \brief Shows/hides controls related to a label map
   */
  void EnableLabelControls(bool b);


  /**
   * \brief Updates the label map table
   */
  void UpdateLabelMapTable();


  /**
   * \brief Updates the list of lookuptables
   */
  void UpdateLookupTableComboBox();

  /**
   * \brief Retrieves the pref values from preference service, and stores in member variables.
   */
  void RetrievePreferenceValues();

  /**
   * \brief Attempts to load all lookuptables cached in the berry service.
   */
  void LoadCachedLookupTables();

  /**
   * \brief Attempts to load LookupTable from given file, returning display name of LookupTable if successful. 
   */
  QString LoadLookupTable(QString& fileName);

  /**
   * \brief BlueBerry's notification about preference changes (e.g. from a preferences dialog).
   */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences*) override;

  /**
   * \brief All the controls for the main view part.
   */
  Ui::ImageLookupTablesViewControls* m_Controls;

  /**
   * \brief Tracks the currently selected node.
   */
  mitk::DataNode::Pointer m_CurrentNode;

  /**
   * \brief Tracks the currently selected image.
   */
  mitk::Image::Pointer m_CurrentImage;

  /**
   * \brief Stores the precision, as you could have float images, with intensity range between 0 and 1.
   */
  int m_Precision;

  /**
   * \brief To stop re-entering the same piece of code recursively.
   */
  bool m_InUpdate;

  /**
   * \brief Used to affect the step size on the sliders/spin boxes.
   */
  int m_ThresholdForIntegerBehaviour;

  /**
   * \brief To store the observer ID on the LevelWindow property.
   */
  unsigned long int m_LevelWindowPropertyObserverTag;
};

}

#endif
