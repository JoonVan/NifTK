/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef __niftkMorphologicalSegmentationViewControls_h_
#define __niftkMorphologicalSegmentationViewControls_h_

#include "ui_niftkMorphologicalSegmentationViewControls.h"
#include <MorphologicalSegmentorPipelineParams.h>
#include <niftkBaseSegmentationViewControls.h>

class QAbstractButton;

/**
 * \class niftkMorphologicalSegmentationViewControls
 * \brief Implements a few Qt specific things that are of no interest to the MITK view class.
 * \ingroup uk_ac_ucl_cmic_midasmorphologicalsegmentor_internal
 */
class niftkMorphologicalSegmentationViewControls
  : public niftkBaseSegmentationViewControls,
    public Ui_niftkMorphologicalSegmentationViewControls
{
  Q_OBJECT

public:

  /// \brief Constructor.
  niftkMorphologicalSegmentationViewControls(QWidget* parent = nullptr);

  /// \brief Destructor.
  virtual ~niftkMorphologicalSegmentationViewControls();

  /// \brief Creates the GUI, initialising everything to off.
  void setupUi(QWidget* parent);

  /// \brief Get the current tab index.
  int GetTabIndex();

  /// \brief Enables/disables all controls.
  void SetEnabled(bool enabled);

  /// \brief Set the dialog according to relevant image data.
  void SetControlsByReferenceImage(double lowestValue, double highestValue, int numberOfAxialSlices, int upDirection);

  /// \brief Set the dialog according to current parameter values
  void SetControlsByPipelineParams(MorphologicalSegmentorPipelineParams& params);

signals:

  void ThresholdingValuesChanged(double lowerThreshold, double upperThreshold, int axialSlicerNumber);
  void ErosionsValuesChanged(double upperThreshold, int numberOfErosions);
  void DilationsValuesChanged(double lowerPercentage, double upperPercentage, int numberOfDilations);
  void RethresholdingValuesChanged(int boxSize);
  void TabChanged(int tabIndex);
  void OKButtonClicked();
  void CancelButtonClicked();
  void RestartButtonClicked();

protected slots:

  void OnThresholdLowerValueChanged();
  void OnThresholdUpperValueChanged();
  void OnAxialCutOffSliderChanged();
  void OnBackButtonClicked();
  void OnNextButtonClicked();
  void OnErosionsUpperThresholdChanged();
  void OnErosionsIterationsChanged();
  void OnDilationsLowerThresholdChanged();
  void OnDilationsUpperThresholdChanged();
  void OnDilationsIterationsChanged();
  void OnRethresholdingSliderChanged();
  void OnRestartButtonClicked();

private:

  void EmitThresholdingValues();
  void EmitErosionValues();
  void EmitDilationValues();
  void EmitRethresholdingValues();

};

#endif
