/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkHelpAboutDialog.h"

#include <QMessageBox>
#include <QString>
#include <iostream>
#include <NifTKConfigure.h>

QmitkHelpAboutDialog::QmitkHelpAboutDialog(QWidget *parent, QString applicationName)
                : m_ApplicationName(applicationName)
{
  if(!m_ApplicationName.isEmpty())
  {
	  this->setupUi(this);
	  this->GenerateHelpAboutText(m_ApplicationName);
	  m_HelpAboutLabel->setWordWrap(true);
	  m_HelpAboutLabel->setOpenExternalLinks(true);
	  m_HelpAboutLabel->setTextFormat(Qt::RichText);

  }
}

QmitkHelpAboutDialog::~QmitkHelpAboutDialog()
{

}

void QmitkHelpAboutDialog::GenerateHelpAboutText(QString applicationName)
{
  // This stuff gets generated during CMake into NifTKConfigure.h
  QString platformName(NIFTK_PLATFORM);
  QString versionNumber(NIFTK_VERSION_STRING);
  QString copyrightText(NIFTK_COPYRIGHT);
  QString originURL(NIFTK_ORIGIN_URL);
  QString originShortText(NIFTK_ORIGIN_SHORT_TEXT);
  QString originLongText(NIFTK_ORIGIN_LONG_TEXT);
  QString wikiURL(NIFTK_WIKI_URL);
  QString wikiText(NIFTK_WIKI_TEXT);
  QString dashboardURL(NIFTK_DASHBOARD_URL);
  QString dashboardText(NIFTK_DASHBOARD_TEXT);
  QString userContact(NIFTK_USER_CONTACT);
  QString qtVersion(NIFTK_QT_VERSION);
  QString boostVersion(NIFTK_BOOST_VERSION);
  QString gdcmVersion(NIFTK_GDCM_VERSION);
  QString dcmtkVersion(NIFTK_DCMTK_VERSION);
  QString itkVersion(NIFTK_ITK_VERSION);
  QString vtkVersion(NIFTK_VTK_VERSION);
  QString ctkVersion(NIFTK_CTK_VERSION);
  QString mitkVersion(NIFTK_MITK_VERSION);
  QString niftkVersion(NIFTK_VERSION);
  QString niftkDateTime(NIFTK_DATE_TIME);
  QString boostLocation(NIFTK_BOOST_LOCATION);
  QString gdcmLocation(NIFTK_GDCM_LOCATION);
  QString dcmtkLocation(NIFTK_DCMTK_LOCATION);
  QString itkLocation(NIFTK_ITK_LOCATION);
  QString vtkLocation(NIFTK_VTK_LOCATION);
  QString ctkLocation(NIFTK_CTK_LOCATION);
  QString mitkLocation(NIFTK_MITK_LOCATION);
  
#ifdef USE_NIFTYREC
  QString niftyRecVersion(NIFTK_NIFTYREC_VERSION);
  QString niftyRecLocation(NIFTK_NIFTYREC_LOCATION);
#endif

  // Main titles with application name, release version and copyright statement.
  QString titles = QObject::tr(
      "<p>"
      "<h1>About %1 - %2</h1>"
      "(git hash %3, at %4, from <a href=\"https://cmicdev.cs.ucl.ac.uk/git/?p=NifTK.git;a=summary\">here</a>)"
      "</p>"
      "<p>%5 Please go to the installation folder for a full license description for this product and dependencies.</p>"
      ).arg(applicationName).arg(versionNumber).arg(niftkVersion).arg(niftkDateTime).arg(copyrightText);

  // Short introduction.
  QString introduction = QObject::tr(
      "<p>"
      "%1 is the user interface for the <a href=\"%2\">%3 (%4)</a> translational imaging platform called <a href=\"http://cmic.cs.ucl.ac.uk/home/software/\">%5</a>."
      "</p>"
      "<p>"
      "%1 was developed with funding from the NIHR and the Comprehensive Biomedical Research Centre at UCL and UCLH grant 168 and TSB grant M1638A. "
      "The principal investigator is <a href=\"http://cmic.cs.ucl.ac.uk/staff/sebastien_ourselin/\">Sebastien Ourselin</a> "
      "and team leader is <a href=\"http://cmic.cs.ucl.ac.uk/staff/matt_clarkson/\">Matt Clarkson</a>."
      "</p>"
      ).arg(applicationName).arg(originURL).arg(originLongText).arg(originShortText).arg(platformName);

  // Over time, insert more collaborators, as we conquer the world!!
  // (mwah ha ha ha .. evil laughter).
  QString collaborators = QObject::tr(
      "<p>"
      "%1 is grateful for the continued support of our clinical and research collaborators including:"
      "<ul>"
      "<li>the <a href=\"http://dementia.ion.ucl.ac.uk/\">UCL Dementia Research Centre</a>.</li>"
      "<li>the <a href=\"http://www.ucl.ac.uk/ion/departments/neuroinflammation/\">UCL Department of Neuroinflammation</a>.</li>"
      "<li>the <a href=\"http://www.ucl.ac.uk/cabi/\">UCL Centre for Advanced Biomedical Imaging</a>.</li>"
      "</ul>"
      "In addition, the software development team would like to acknowledge the kind support of the open-source software community "
      "during development of NifTK and are especially grateful to the developers of "
      "<a href=\"http://www.mitk.org\">MITK</a> and <a href=\"http://www.commontk.org\">CTK</a>. "
      "In addition, various clip art comes from <a href=\"http://www.openclipart.org\">openclipart.org</a>."
      "</p>"
      ).arg(originShortText);

  // Over time, insert more software packages, as platform expands,
  // (and dependencies get exponentially more frustrating :-).
  QString versionsStart = QObject::tr(
      "<h3>Software Versions</h3>"
      "<p>"
      "%1 has been developed using the following core libraries."
      "</p>"
      "<p><table>"
      "<tr><td><a href=\"http://www.boost.org\">Boost</a></td><td>%2</td><td><a href=\"http://www.boost.org/LICENSE_1_0.txt\">Boost v1.0</a></td><td><a href=\"%3\">from here</a></td></tr>"
      "<tr><td><a href=\"http://qt.nokia.com/products\">Qt</a></td><td>%4</td><td><a href=\"http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html\">LGPL v2.1</a></td><td><a href=\"http://qt.nokia.com/products\">from here</a></td></tr>"
      "<tr><td><a href=\"http://www.creatis.insa-lyon.fr/software/public/Gdcm/\">GDCM</a></td><td>%5</td><td><a href=\"http://www.creatis.insa-lyon.fr/software/public/Gdcm/License.html\">GDCM</a></td><td><a href=\"%6\">from here</a></td></tr>"
      "<tr><td><a href=\"http://dicom.offis.de/\">DCMTK</a></td><td>%7</td><td><a href=\"ftp://dicom.offis.de/pub/dicom/offis/software/dcmtk/dcmtk360/COPYRIGHT\">DCMTK</a></td><td><a href=\"%8\">from here</a></td></tr>"
      "<tr><td><a href=\"http://www.itk.org\">ITK</a>(Patched)</td><td>%9</td><td><a href=\"http://itk.org/ITK/project/license.html\">Apache v2.0</a></td><td><a href=\"%10\">from here</a></td></tr>"
      "<tr><td><a href=\"http://www.vtk.org\">VTK</a></td><td>%11</td><td><a href=\"http://www.vtk.org/VTK/project/license.html\">BSD</a></td><td><a href=\"%12\">from here</a></td></tr>"
      "<tr><td><a href=\"http://www.commontk.org\">CTK</a></td><td>%13</td><td><a href=\"http://www.apache.org/licenses/LICENSE-2.0.html\">Apache v2.0</a></td><td><a href=\"%14\">from here</a></td></tr>"
      "<tr><td><a href=\"http://www.mitk.org\">MITK</a>(Modified)</td><td>%15</td><td><a href=\"http://www.mitk.org/wiki/License\">BSD-style</a></td><td><a href=\"%16\">from here</a></td></tr>"
      )
      .arg(applicationName)
      .arg(boostVersion)
      .arg(boostLocation)
      .arg(qtVersion)
      .arg(gdcmVersion)
      .arg(gdcmLocation)
      .arg(dcmtkVersion)
      .arg(dcmtkLocation)
      .arg(itkVersion)
      .arg(itkLocation)
      .arg(vtkVersion)
      .arg(vtkLocation)
      .arg(ctkVersion.left(10))
      .arg(ctkLocation)
      .arg(mitkVersion.left(10))
      .arg(mitkLocation)
      ;

  #ifdef USE_NIFTYREG
    QString niftyRegVersion(NIFTK_NIFTYREG_VERSION);
    QString niftyRegLocation(NIFTK_NIFTYREG_LOCATION);
    QString niftyRegText = QObject::tr(
        "<tr><td><a href=\"http://sourceforge.net/projects/niftyreg/?source=directory\">NiftyReg</a></td><td>%1</td><td><a href=\"http://sourceforge.net/p/niftyreg/code/395/tree/trunk/nifty_reg/LICENSE.txt\">BSD</a></td><td><a href=\"%2\">from here</a></td></tr>"
        ).arg(niftyRegVersion).arg(niftyRegLocation);
  #endif

  #ifdef USE_NIFTYSEG
    QString niftySegVersion(NIFTK_NIFTYSEG_VERSION);
    QString niftySegLocation(NIFTK_NIFTYSEG_LOCATION);
    QString niftySegText = QObject::tr(
        "<tr><td><a href=\"http://sourceforge.net/projects/niftyseg/?source=directory\">NiftySeg</a></td><td>%1</td><td><a href=\"http://sourceforge.net/p/niftyseg/code/145/tree/LICENSE.txt\">BSD</a></td><td><a href=\"%2\">from here</a></td></tr>"
        ).arg(niftySegVersion).arg(niftySegLocation);
  #endif

  #ifdef USE_NIFTYSIM
    QString niftySimVersion(NIFTK_NIFTYSIM_VERSION);
    QString niftySimLocation(NIFTK_NIFTYSIM_LOCATION);
    QString niftySimText = QObject::tr(
        "<tr><td><a href=\"http://sourceforge.net/projects/niftysim/?source=directory\">NiftySim</a></td><td>%1</td><td><a href=\"http://sourceforge.net/p/niftysim/code/ci/master/tree/nifty_sim/LICENSE.txt\">BSD</a></td><td><a href=\"%2\">from here</a></td></tr>"
        ).arg(niftySimVersion).arg(niftySimLocation);
  #endif

  #ifdef BUILD_IGI
    QString niftyLinkVersion(NIFTK_NIFTYLINK_VERSION);
    //QString niftyLinkLocation(NIFTK_NIFTYLINK_LOCATION);
    QString niftyLinkText = QObject::tr(
      "<tr><td><a href=\"https://cmicdev.cs.ucl.ac.uk/git/?p=NiftyLink.git;a=summary\">NiftyLink</a></td><td>%1</td><td>Private NiftyLink License</td><td><a href=\"https://cmicdev.cs.ucl.ac.uk/git/?p=NiftyLink.git;a=summary\">from here</a></td></tr>"
      ).arg(niftyLinkVersion.left(10));//.arg(niftyLinkLocation);

    QString arucoVersion(NIFTK_VERSION_ARUCO);
    QString arucoLocation(NIFTK_LOCATION_ARUCO);
    QString arucoText = QObject::tr(
      "<tr><td><a href=\"http://www.uco.es/investiga/grupos/ava/node/26\">ARUCO</a></td><td>%1</td><td><a href=\"http://opensource.org/licenses/BSD-2-Clause\">BSD</a></td><td><a href=\"%2\">from here</a></td></tr>"
      ).arg(arucoVersion).arg(arucoLocation);

    QString eigenVersion(NIFTK_VERSION_EIGEN);
    QString eigenLocation(NIFTK_LOCATION_EIGEN);
    QString eigenText = QObject::tr(
      "<tr><td><a href=\"http://eigen.tuxfamily.org/\">EIGEN</a></td><td>%1</td><td><a href=\"http://opensource.org/licenses/MPL-2.0\">MPL v2</a></td><td><a href=\"%2\">from here</a></td></tr>"
      ).arg(eigenVersion).arg(eigenLocation);
 
    QString aprilTagsVersion(NIFTK_VERSION_APRILTAGS);
    QString aprilTagsLocation(NIFTK_LOCATION_APRILTAGS);
    QString aprilTagsText = QObject::tr(
      "<tr><td><a href=\"http://people.csail.mit.edu/kaess/apriltags/\">April Tags</a></td><td>%1</td><td><a href=\"http://opensource.org/licenses/LGPL-2.1\">LGPL v2.1</a></td><td><a href=\"%2\">from here</a></td></tr>"
      ).arg(aprilTagsVersion).arg(aprilTagsLocation);

    QString flannVersion(NIFTK_VERSION_FLANN);
    QString flannLocation(NIFTK_LOCATION_FLANN);
    QString flannText = QObject::tr(
      "<tr><td><a href=\"http://www.cs.ubc.ca/research/flann/\">FLANN</a>(Patched)</td><td>%1</td><td><a href=\"http://opensource.org/licenses/BSD-3-Clause\">BSD</a></ltd><td><a href=\"%2\">from here</a></td></tr>"
      ).arg(flannVersion).arg(flannLocation);

    QString pclVersion(NIFTK_VERSION_PCL);
    QString pclLocation(NIFTK_LOCATION_PCL);
    QString pclText = QObject::tr(
      "<tr><td><a href=\"http://pointclouds.org/\">PCL</a></td><td>%1</td><td><a href=\"https://github.com/PointCloudLibrary/pcl/blob/master/LICENSE.txt\">BSD</a></td><td><a href=\"%2\">from here</a></td></tr>"
      ).arg(pclVersion).arg(pclLocation);

    QString openCVVersion(NIFTK_VERSION_OPENCV);
    QString openCVLocation(NIFTK_LOCATION_OPENCV);
    QString openCVText = QObject::tr(
      "<tr><td><a href=\"http://opencv.org/\">OpenCV</a></td><td>%1</td><td><a href=\"https://github.com/Itseez/opencv/blob/master/doc/license.txt\">BSD</a></td><td><a href=\"%2\">from here</a></td></tr>"
      ).arg(openCVVersion).arg(openCVLocation);
    
  #endif

  QString versionsEnd = QObject::tr(
      "</table></p>"
      );

  // Over time, insert more platforms that we have tested on,
  // (but these should be backed up with a Dashboard or else it ain't worth diddly-squat).
  QString testingDetails = QObject::tr(
      "<p>"
      "%1 has been compiled and tested on the following platforms:"
      "<ul>"
      "<li>Mac OSX 10.9 (Mavericks)</li>"
      "<li>Mac OSX 10.8 (Mountain Lion)</li>"
      "<li>Ubuntu 12.04</li>"
      "<li>Ubuntu 11.04</li>"
      "<li>Linux Mint 14</li>"
      "<li>Scientific Linux 6.1</li>"
      "<li>Debian 7.0.4</li>"
      "<li>Windows 7</li>"
      "<li>Windows 8</li>"
      "</ul>"
      "We use a 64 bit operating system. Our software quality control statistics can be seen on this <a href=\"%2\">%3</a>."
      "</p>"
      ).arg(applicationName).arg(dashboardURL).arg(dashboardText);

  QString furtherInformation = QObject::tr(
      "<p>"
      "Further information can be obtained by:"
      "<ul>"
      "<li>Emailing the %1 <a href=\"%2\">users mailing list</a>.</li>"
      "<li>Visiting the %1 <a href=\"%3\">%4</a>.</li>"
      "</ul>"
      "</p>"
      ).arg(platformName).arg(userContact).arg(wikiURL).arg(wikiText);

  // Stick it all together.
  QString totalText =
      titles
      .append(introduction)
      .append(collaborators)
      .append(furtherInformation)
      .append(versionsStart)
#ifdef USE_NIFTYREG
      .append(niftyRegText)
#endif
#ifdef USE_NIFTYSEG
      .append(niftySegText)
#endif
#ifdef USE_NIFTYSIM
      .append(niftySimText)
#endif
#ifdef BUILD_IGI
      .append(niftyLinkText)
      .append(arucoText)
      .append(eigenText)
      .append(aprilTagsText)
      .append(flannText)
      .append(pclText)
      .append(openCVText)
#endif
      .append(versionsEnd)
      .append(testingDetails)
      ;

  this->setWindowTitle(tr("About %1").arg(applicationName));
  QIcon helpAboutIcon(QLatin1String(":/Icons/icon.png"));

  if (!helpAboutIcon.isNull())
  {
    this->setWindowIcon(helpAboutIcon);
  }

  m_HelpAboutLabel->setText(totalText);

}
