/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef SurfaceRegViewPreferencePage_h
#define SurfaceRegViewPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <berryIPreferences.h>

class QWidget;
class QCheckBox;
class QSpinBox;

/**
 * \class SurfaceRegViewPreferencePage
 * \brief Preferences page for the Surface Based Registration View plugin.
 * \ingroup uk_ac_ucl_cmic_igisurfacereg_internal
 *
 */
class SurfaceRegViewPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  /**
   * \brief Stores the name of the preference node that contains the maximum iterations preference
   */
  static const QString MAXIMUM_NUMBER_OF_ITERATIONS;

  /**
   * \brief Stored the name of the preference node that contains the maximum number of points to use
   */
  static const QString MAXIMUM_NUMBER_OF_POINTS;

  /**
   * \brief Stored the name of the preference node that contains the maximum number of iterations to use in Trimmed Least Squares.
   */
  static const QString TLS_ITERATIONS;

  /**
   * \brief Stored the name of the preference node that contains the percentage to use in Trimmed Least Squares.
   */
  static const QString TLS_PERCENTAGE;

  SurfaceRegViewPreferencePage();
  SurfaceRegViewPreferencePage(const SurfaceRegViewPreferencePage& other);
  ~SurfaceRegViewPreferencePage();

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

  QWidget        *m_MainControl;
  QSpinBox       *m_MaximumIterations;
  QSpinBox       *m_MaximumPoints;
  QSpinBox       *m_TLSIterations;
  QSpinBox       *m_TLSPercentage;
  bool            m_Initializing;

  berry::IPreferences::Pointer m_SurfaceRegViewPreferencesNode;
};

#endif // SurfaceRegViewPreferencePage_h

