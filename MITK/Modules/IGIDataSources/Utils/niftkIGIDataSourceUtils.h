/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkIGIDataSourceUtils_h
#define niftkIGIDataSourceUtils_h

#include <niftkIGIDataSourcesExports.h>
#include <niftkIGIDataType.h>
#include <set>
#include <QDir>
#include <QString>
#include <QMap>
#include <QHash>

/**
 * \file mitkIGIDataSourceUtils.h
 * \brief Some useful functions to help process IGI Data Sources
 */
namespace niftk
{

/**
* \brief Returns the platform specific directory separator.
*/
NIFTKIGIDATASOURCES_EXPORT
QString GetPreferredSlash();

/**
* \brief Scans the path for individual files that match a timestamp pattern and suffix.
* \param suffix for example ".jpg" or "-ultrasoundImage.nii".
*/
NIFTKIGIDATASOURCES_EXPORT
void ProbeTimeStampFiles(QDir path,
                         const QString& suffix,
                         std::set<niftk::IGIDataType::IGITimeType>& timeStamps,
                         QHash<niftk::IGIDataType::IGITimeType, QString>& timeStampToFileName);

/**
* \brief Scans the path for individual files that match a timestamp pattern and suffix.
* \param suffix for example ".jpg" or "-ultrasoundImage.nii".
*/
NIFTKIGIDATASOURCES_EXPORT
void ProbeTimeStampFiles(QDir path,
                         const QString& suffix,
                         std::set<niftk::IGIDataType::IGITimeType>& timeStamps
                         );

/**
* \brief Returns the list of timestamps, by source name.
*/
NIFTKIGIDATASOURCES_EXPORT
void GetPlaybackIndex(
    const QString& directory,
    const QString& fileExtension,
    QMap<QString, std::set<niftk::IGIDataType::IGITimeType> >& bufferToTimeStamp,
    QMap<QString, QHash<niftk::IGIDataType::IGITimeType, QStringList> >& bufferToTimeStampToFileNames
    );

/**
* \brief Returns the minimum and maximum timestamped of all files under the specified
* path, with the specified fileExtension, that look like they are timestamped.
*/
NIFTKIGIDATASOURCES_EXPORT
bool ProbeRecordedData(const QString& path,
                       const QString& fileExtension,
                       niftk::IGIDataType::IGITimeType* firstTimeStampInStore,
                       niftk::IGIDataType::IGITimeType* lastTimeStampInStore);

} // end namespace

#endif
