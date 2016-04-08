/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkCommonAppsApplicationPreferencePage_h
#define QmitkCommonAppsApplicationPreferencePage_h

#include <uk_ac_ucl_cmic_commonapps_Export.h>
#include <berryIQtPreferencePage.h>
#include <berryIPreferences.h>

class QWidget;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;

/**
 * \class QmitkCommonAppsApplicationPreferencePage
 * \brief Preferences page for this plugin, providing application wide defaults.
 * \ingroup uk_ac_ucl_cmic_commonapps_internal
 *
 */
class CMIC_QT_COMMONAPPS QmitkCommonAppsApplicationPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkCommonAppsApplicationPreferencePage();
  QmitkCommonAppsApplicationPreferencePage(const QmitkCommonAppsApplicationPreferencePage& other);
  ~QmitkCommonAppsApplicationPreferencePage();

  static const QString IMAGE_RESLICE_INTERPOLATION;
  static const QString IMAGE_TEXTURE_INTERPOLATION;
  static const QString LOWEST_VALUE_OPACITY;
  static const QString HIGHEST_VALUE_OPACITY;
  static const QString BINARY_OPACITY_NAME;
  static const double BINARY_OPACITY_VALUE;

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* widget) override;

  QWidget* GetQtControl() const override;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  virtual bool PerformOk() override;

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  virtual void PerformCancel() override;

  ///
  /// \see IPreferencePage::Update()
  ///
  virtual void Update() override;

protected slots:

protected:

  QWidget        *m_MainControl;
  QComboBox      *m_ResliceInterpolation;
  QComboBox      *m_TextureInterpolation;
  QDoubleSpinBox *m_LowestValueOpacity;
  QDoubleSpinBox *m_HighestValueOpacity;
  QDoubleSpinBox *m_BinaryOpacity;

  berry::IPreferences::Pointer m_PreferencesNode;
};

#endif /* QMITKCOMMONAPPSAPPLICATIONPREFERENCEPAGE_H_ */

