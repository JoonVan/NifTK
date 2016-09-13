/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkDnDDisplayPreferencePage_h
#define niftkDnDDisplayPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <uk_ac_ucl_cmic_dnddisplay_Export.h>
#include <berryIPreferences.h>

class QWidget;


namespace Ui
{
class niftkDnDDisplayPreferencePage;
}

namespace niftk
{

/**
 * \class DnDDisplayPreferencePage
 * \brief Provides a preferences page for the NifTK DnD Display, including default number of rows,
 * default number of columns, image interpolation, default window layout and background colour.
 * \ingroup uk_ac_ucl_cmic_dnddisplay
 */
struct DNDDISPLAY_EXPORT DnDDisplayPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  DnDDisplayPreferencePage();
  DnDDisplayPreferencePage(const DnDDisplayPreferencePage& other);

  void CreateQtControl(QWidget* widget);
  QWidget* GetQtControl() const;

  /// \brief Nothing to do.
  void Init(berry::IWorkbench::Pointer workbench);

  /// \see IPreferencePage::PerformOk()
  virtual bool PerformOk();

  /// \see IPreferencePage::PerformCancel()
  virtual void PerformCancel();

  /// \see IPreferencePage::Update()
  virtual void Update();

  /// \brief Stores the preference name for the default image interpolation in the DnD Display.
  static const QString DNDDISPLAY_DEFAULT_INTERPOLATION_TYPE;

  /// \brief Stores the preference name for the default background colour in the DnD Display.
  static const QString DNDDISPLAY_BACKGROUND_COLOUR;

  /// \brief Stores the preference name for the default background colour stylesheet in the NifTK DnD Display.
  static const QString DNDDISPLAY_BACKGROUND_COLOUR_STYLESHEET;

  /// \brief Stores the preference name for slice select tracking
  static const QString DNDDISPLAY_SLICE_SELECT_TRACKING;

  /// \brief Stores the preference name for time select tracking
  static const QString DNDDISPLAY_TIME_SELECT_TRACKING;

  /// \brief Stores the preference name for magnification select tracking
  static const QString DNDDISPLAY_MAGNIFICATION_SELECT_TRACKING;

  /// \brief Stores the preference name for whether we show the 2D cursors as people may prefer them to always be off.
  static const QString DNDDISPLAY_SHOW_2D_CURSORS;

  /// \brief Stores the preference name for whether we show the direction annotations.
  static const QString DNDDISPLAY_SHOW_DIRECTION_ANNOTATIONS;

  /// \brief Stores the preference name for whether we show the position annotation.
  static const QString DNDDISPLAY_SHOW_POSITION_ANNOTATION;

  /// \brief Stores the preference name for whether we show the intensity annotation.
  static const QString DNDDISPLAY_SHOW_INTENSITY_ANNOTATION;

  /// \brief Stores the preference name for whether we show the property annotation.
  static const QString DNDDISPLAY_SHOW_PROPERTY_ANNOTATION;

  /// \brief Stores the preference name for the properties to show as annotation.
  static const QString DNDDISPLAY_PROPERTIES_FOR_ANNOTATION;

  /// \brief Stores the preference name for the default window layout in the NifTK DnD Display.
  static const QString DNDDISPLAY_DEFAULT_WINDOW_LAYOUT;

  /// \brief Stores the preference name for whether to revert to last remembered slice, timestep and magnification when switching window layout.
  static const QString DNDDISPLAY_REMEMBER_VIEWER_SETTINGS_PER_WINDOW_LAYOUT;

  /// \brief Stores the preference name for the default number of rows in the DnD Display.
  static const QString DNDDISPLAY_DEFAULT_VIEWER_ROW_NUMBER;

  /// \brief Stores the preference name for the default number of columns in the DnD Display.
  static const QString DNDDISPLAY_DEFAULT_VIEWER_COLUMN_NUMBER;

  /// \brief Stores the preference name for the default drop type (single, multiple, all).
  static const QString DNDDISPLAY_DEFAULT_DROP_TYPE;

  /// \brief Stores the preference name for whether we show the magnification slider, as most people wont need it.
  static const QString DNDDISPLAY_SHOW_MAGNIFICATION_SLIDER;

  /// \brief Stores the preference name for whether we show the the options to show/hide cursor,
  /// direction annotations and 3D window in multi window layout.
  static const QString DNDDISPLAY_SHOW_SHOWING_OPTIONS;

  /// \brief Stores the preference name for whether we show the window layout controls.
  static const QString DNDDISPLAY_SHOW_WINDOW_LAYOUT_CONTROLS;

  /// \brief Stores the preference name for whether we show the viewer number controls.
  static const QString DNDDISPLAY_SHOW_VIEWER_NUMBER_CONTROLS;

  /// \brief Stores the preference name for a simple on/off preference for whether we show the single, multiple, all checkbox.
  static const QString DNDDISPLAY_SHOW_DROP_TYPE_CONTROLS;

public slots:

  void OnBackgroundColourChanged();
  void OnResetBackgroundColour();
  void OnResetMIDASBackgroundColour();

private:

  QWidget* m_MainWidget;
  Ui::niftkDnDDisplayPreferencePage* ui;

  QString m_BackgroundColorStyleSheet;
  QString m_BackgroundColor;

  berry::IPreferences::Pointer m_DnDDisplayPreferencesNode;
};

}

#endif
