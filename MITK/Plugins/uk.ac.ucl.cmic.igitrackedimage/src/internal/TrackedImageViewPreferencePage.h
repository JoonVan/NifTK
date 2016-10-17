/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef TrackedImageViewPreferencePage_h
#define TrackedImageViewPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <berryIPreferences.h>
#include <QString>

class QWidget;
class QPushButton;
class QDoubleSpinBox;
class ctkPathLineEdit;
class QCheckBox;

/**
 * \class TrackedImageViewPreferencePage
 * \brief Preferences page for the Tracked Image View plugin.
 * \ingroup uk_ac_ucl_cmic_igitrackedimage_internal
 *
 */
class TrackedImageViewPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  /**
   * \brief Stores the name of the preference node that contains the name of the calibration file.
   */
  static const QString CALIBRATION_FILE_NAME;

  /**
   * \brief Stores the name of the preference node that contains the name of the scale file.
   */
  static const QString SCALE_FILE_NAME;

  /**
   * \brief Stores the name of the preference node that contains the name of the scale file.
   */
  static const QString EMTOWORLDCALIBRATION_FILE_NAME;

  /**
   * \brief Stores the name of the preference node that contains whether we flip the x scale factor to be negative.
   */
  static const QString FLIP_X_SCALING;

  /**
   * \brief Stores the name of the preference node that contains whether we flip the y scale factor to be negative.
   */
  static const QString FLIP_Y_SCALING;

  /**
   * \brief Stores the name of the preference node that contains status whether we show the clone image button.
   */
  static const QString CLONE_IMAGE;

  /**
   * \brief Stores the name of the preference node that controls if we show the 2D window.
   */
  static const QString SHOW_2D_WINDOW;

  TrackedImageViewPreferencePage();
  TrackedImageViewPreferencePage(const TrackedImageViewPreferencePage& other);
  ~TrackedImageViewPreferencePage();

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

private slots:

private:

  QWidget         *m_MainControl;
  ctkPathLineEdit *m_CalibrationFileName;
  ctkPathLineEdit *m_ScaleFileName;
  ctkPathLineEdit *m_EmToWorldCalibrationFileName;

  bool             m_Initializing;
  QCheckBox       *m_FlipXScaling;
  QCheckBox       *m_FlipYScaling;

  QCheckBox       *m_CloneImage;
  QCheckBox       *m_Show2DWindow;

  berry::IPreferences::Pointer m_TrackedImageViewPreferencesNode;
};

#endif // TrackedImageViewPreferencePage_h

