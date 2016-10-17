/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef SurfaceReconViewPreferencePage_h
#define SurfaceReconViewPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <berryIPreferences.h>
#include <QString>
#include "ui_SurfaceReconViewPreferencePage.h"

class QWidget;
class QPushButton;

/**
 * \class SurfaceReconViewPreferencePage
 * \brief Preferences page for the Surface Reconstruction View plugin.
 * \ingroup uk_ac_ucl_cmic_igisurfacerecon_internal
 *
 */
class SurfaceReconViewPreferencePage : public QObject, public berry::IQtPreferencePage, public Ui::SurfaceReconViewPreferencePageForm
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  static const char*      s_DefaultCalibrationFilePathPrefsName;
  static const char*      s_UseUndistortionDefaultPathPrefsName;
  static const char*      s_DefaultTriangulationErrorPrefsName;
  static const char*      s_DefaultMinDepthRangePrefsName;
  static const char*      s_DefaultMaxDepthRangePrefsName;
  static const char*      s_DefaultBakeCameraTransformPrefsName;

  SurfaceReconViewPreferencePage();
  SurfaceReconViewPreferencePage(const SurfaceReconViewPreferencePage& other);
  ~SurfaceReconViewPreferencePage();

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* widget) override;

  QWidget* GetQtControl() const override;

  /**
   * \see IPreferencePage::PerformOk()
   */
  virtual bool PerformOk() override;

  /**
   * \see IPreferencePage::PerformCancel()
   */
  virtual void PerformCancel() override;

  /**
   * \see IPreferencePage::Update()
   */
  virtual void Update() override;

protected slots:
  void OnDefaultPathBrowseButtonClicked();
  // used for both radio buttons
  void OnUseUndistortRadioButtonClicked();


private:
  berry::IPreferences::Pointer      m_SurfaceReconViewPreferencesNode;
  QString                           m_DefaultCalibrationFilePath;
  bool                              m_UseUndistortPluginDefaultPath;
};

#endif // SurfaceReconViewPreferencePage_h

