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

#ifndef _SurfaceExtractorPreferencePage_h
#define _SurfaceExtractorPreferencePage_h

#include <QObject>
#include <berryIQtPreferencePage.h>

class QWidget;

class SurfaceExtractorPreferencePagePrivate;

/**
 * \class SurfaceExtractorPreferencePage
 * \brief Preferences page for this plugin, to set defaults for the Surface Extractor view.
 * \ingroup uk_ac_ucl_cmic_gui_surfaceextractor
 *
 */
class SurfaceExtractorPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  SurfaceExtractorPreferencePage();
  ~SurfaceExtractorPreferencePage();

  static const std::string GAUSSIAN_SMOOTH_NAME;
  static const bool GAUSSIAN_SMOOTH_DEFAULT;
  static const std::string GAUSSIAN_STDDEV_NAME;
  static const double GAUSSIAN_STDDEV_DEFAULT;
  static const std::string THRESHOLD_NAME;
  static const double THRESHOLD_DEFAULT;
  static const std::string TARGET_REDUCTION_NAME;
  static const double TARGET_REDUCTION_DEFAULT;
  static const std::string MAX_NUMBER_OF_POLYGONS_NAME;
  static const long MAX_NUMBER_OF_POLYGONS_DEFAULT;

  virtual void Init(berry::IWorkbench::Pointer workbench);

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

private slots:
  void on_cbxGaussianSmooth_toggled(bool);

private:
  QWidget* ui;
  QScopedPointer<SurfaceExtractorPreferencePagePrivate> d_ptr;

  Q_DECLARE_PRIVATE(SurfaceExtractorPreferencePage);
  Q_DISABLE_COPY(SurfaceExtractorPreferencePage);
};

#endif /* _SurfaceExtractorPreferencePage_h */
