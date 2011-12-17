/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-11-18 09:05:48 +0000 (Fri, 18 Nov 2011) $
 Revision          : $Revision: 7804 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef _QMITKIMAGELOOKUPTABLESPREFERENCE_PAGE_H_INCLUDED
#define _QMITKIMAGELOOKUPTABLESPREFERENCE_PAGE_H_INCLUDED

#include "berryIQtPreferencePage.h"
#include "uk_ac_ucl_cmic_imagelookuptables_Export.h"
#include <berryIPreferences.h>

class QWidget;
class QRadioButton;
class QDoubleSpinBox;
class QSpinBox;

/**
 * \class QmitkImageLookupTablesPreferencePage
 * \brief Preferences page for this plugin, enabling choice of spin box precision, and initialisation options
 * for the initial Level/Window widget, which can either be according the MITK LevelWindow side bar, or just
 * based on image intensity.
 * \ingroup uk_ac_ucl_cmic_gui_imagelookuptables
 *
 */
class IMAGELOOKUPTABLES_EXPORTS QmitkImageLookupTablesPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkImageLookupTablesPreferencePage();
  QmitkImageLookupTablesPreferencePage(const QmitkImageLookupTablesPreferencePage& other);
  ~QmitkImageLookupTablesPreferencePage();

  static const std::string USE_IMAGE_DATA_NAME;
  static const std::string PERCENTAGE_NAME;
  static const std::string PRECISION_NAME;

  void Init(berry::IWorkbench::Pointer workbench);

  void CreateQtControl(QWidget* widget);

  QWidget* GetQtControl() const;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  virtual bool PerformOk();

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  virtual void PerformCancel();

  ///
  /// \see IPreferencePage::Update()
  ///
  virtual void Update();

protected slots:

  void OnLevelWindowRadioButtonChecked(bool);
  void OnImageDataRadioButtonChecked(bool);
  void OnPercentageSpinBoxChanged(double);

protected:

  QWidget* m_MainControl;
  QRadioButton* m_UseLevelWindowRadioButton;
  QRadioButton* m_UseImageDataRadioButton;
  QDoubleSpinBox* m_PercentageOfDataRangeDoubleSpinBox;
  QSpinBox* m_Precision;

  bool m_Initializing;

  berry::IPreferences::Pointer m_ImageLookupTablesPreferencesNode;
};

#endif /* QMITKDATAMANAGERPREFERENCEPAGE_H_ */

