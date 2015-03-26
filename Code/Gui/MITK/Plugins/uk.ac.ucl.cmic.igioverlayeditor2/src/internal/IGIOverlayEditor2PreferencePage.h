/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef IGIOverlayEditor2PreferencePage_h
#define IGIOverlayEditor2PreferencePage_h

#include <berryIQtPreferencePage.h>
#include <berryIPreferences.h>

class QWidget;
class QRadioButton;
class QPushButton;
class QWidgetAction;
class QCheckBox;
class ctkPathLineEdit;

/**
 * \class IGIOverlayEditor2PreferencePage
 * \brief Preference page for IGIOverlayEditor2, currently setting the gradient background.
 * \ingroup uk_ac_ucl_cmic_igioverlayeditor2_internal
 */
struct IGIOverlayEditor2PreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  IGIOverlayEditor2PreferencePage();

  void Init(berry::IWorkbench::Pointer workbench);
  void CreateQtControl(QWidget* widget);

  QWidget* GetQtControl() const;

  /**
   * \see IPreferencePage::PerformOk()
   */
  virtual bool PerformOk();

  /**
   * \see IPreferencePage::PerformCancel()
   */
  virtual void PerformCancel();

  /**
   * \see IPreferencePage::Update()
   */
  virtual void Update();

  ///**
  // * \brief Stores the name of the preference node that contains the stylesheet of the first background colour.
  // */
  //static const std::string FIRST_BACKGROUND_STYLE_SHEET;

  ///**
  // * \brief Stores the name of the preference node that contains the stylesheet of the second background colour.
  // */
  //static const std::string SECOND_BACKGROUND_STYLE_SHEET;

  ///**
  // * \brief Stores the name of the preference node that contains the first background colour.
  // */
  //static const std::string FIRST_BACKGROUND_COLOUR;

  ///**
  // * \brief Stores the name of the preference node that contains the second background colour.
  // */
  //static const std::string SECOND_BACKGROUND_COLOUR;

  ///**
  // * \brief Stores the name of the preference node containing the filename of the calibration (eg. hand-eye for a laparoscope).
  // */
  //static const std::string CALIBRATION_FILE_NAME;

  ///**
  // * \brief Stores the name of the preference node containing whether we are doing a camera tracking mode (for video), or the alternative is image tracking (e.g. for ultrasound).
  // */
  //static const std::string CAMERA_TRACKING_MODE;

  ///**
  // * \brief Stores the name of the preference node containing whether we are using clipping planes in Image Tracking mode.
  // */  
  //static const std::string CLIP_TO_IMAGE_PLANE;
  
public slots:

  //void FirstColorChanged();
  //void SecondColorChanged();
  //void ResetColors();

protected:

  QWidget         *m_MainControl;
  //QRadioButton    *m_CameraTrackingMode;
  //QRadioButton    *m_ImageTrackingMode;
  //QCheckBox       *m_ClipToImagePlane;
  //QPushButton     *m_ColorButton1;
  //QPushButton     *m_ColorButton2;
  //ctkPathLineEdit *m_CalibrationFileName;
  //std::string      m_FirstColor;
  //std::string      m_SecondColor;
  //QString          m_FirstColorStyleSheet;
  //QString          m_SecondColorStyleSheet;

  berry::IPreferences::Pointer m_IGIOverlayEditor2PreferencesNode;
};

#endif /* IGIOverlayEditor2PreferencePage_h */
