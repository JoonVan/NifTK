/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkIGIDataSource.h"
#include "QmitkIGIDataSourceBackgroundSaveThread.h"

//-----------------------------------------------------------------------------
QmitkIGIDataSource::QmitkIGIDataSource(mitk::DataStorage* storage)
: mitk::IGIDataSource(storage)
, m_SaveThread(NULL)
{
  m_SaveThread = new QmitkIGIDataSourceBackgroundSaveThread(this, this);
}


//-----------------------------------------------------------------------------
QmitkIGIDataSource::~QmitkIGIDataSource()
{
  if (m_SaveThread != NULL)
  {
    m_SaveThread->ForciblyStop();
    delete m_SaveThread;
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSource::EmitDataSourceStatusUpdatedSignal()
{
  emit DataSourceStatusUpdated(this->GetIdentifier());
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSource::StartRecording(const std::string& directoryPrefix, const bool& saveInBackground, const bool& saveOnReceipt)
{
  mitk::IGIDataSource::StartRecording(directoryPrefix, saveInBackground, saveOnReceipt);
  if (!m_SaveThread->isRunning())
  {
    m_SaveThread->start();
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSource::SetSavingInterval(int seconds)
{
  m_SaveThread->SetInterval(seconds*1000);
  this->Modified();
}


//-----------------------------------------------------------------------------
std::set<igtlUint64> QmitkIGIDataSource::ProbeTimeStampFiles(QDir path, const QString& suffix)
{
  // this should be a static assert...
  assert(std::numeric_limits<qulonglong>::max() >= std::numeric_limits<igtlUint64>::max());

  std::set<igtlUint64>  result;

  QStringList filters;
  filters << QString("*" + suffix);
  path.setNameFilters(filters);
  path.setFilter(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);

  QStringList files = path.entryList();
  if (!files.empty())
  {
    foreach (QString file, files)
    {
      if (file.endsWith(suffix))
      {
        // in the future, maybe add prefix parsing too.
        QString  middle = file.mid(0, file.size() - suffix.size());

        bool  ok = false;
        qulonglong value = middle.toULongLong(&ok);
        if (ok)
        {
          result.insert(value);
        }
      }
    }
  }

  return result;
}
