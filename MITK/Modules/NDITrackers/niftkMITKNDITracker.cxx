/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkMITKNDITracker.h"
#include <niftkMITKMathsUtils.h>

#include <mitkException.h>
#include <mitkTrackingDeviceSourceConfigurator.h>

namespace niftk
{

//-----------------------------------------------------------------------------
MITKNDITracker::MITKNDITracker(mitk::DataStorage::Pointer dataStorage,
                               std::string portName,
                               mitk::TrackingDeviceData deviceData,
                               std::string toolConfigFileName,
                               int preferredFramesPerSecond
                              )
: NDITracker(dataStorage, portName, deviceData, toolConfigFileName, preferredFramesPerSecond)
, m_TrackerDevice(nullptr)
, m_TrackerSource(nullptr)
{

  // Setup tracker.
  m_TrackerDevice = mitk::NDITrackingDevice::New();
  m_TrackerDevice->SetData(m_DeviceData);
  m_TrackerDevice->SetType(m_DeviceData.Line);
#ifdef _WIN32
  // So for windows, the m_PortName should be a string containing just the number. e.g. for COM1, m_PortName=1.
  // This can then be converted to an int, and static_cast'ed to the right enum.
  m_TrackerDevice->SetPortNumber(static_cast<mitk::SerialCommunication::PortNumber>(std::stoi(m_PortName)));
#else
  // For Linux and Mac, this should be the full /dev/tty (Linux), /dev/cu.usb (Mac).
  m_TrackerDevice->SetDeviceName(m_PortName);
#endif

  // To Do. This should not be necessary. Trackers should be configured in the same way?
  if (deviceData.Line == mitk::NDIAurora)
  {
    try
    {
      m_TrackerDevice->OpenConnection();
      m_TrackerDevice->StartTracking();
    }
    catch(const mitk::Exception& e)
    {
      // If we don't connect, we should still try to create tracker.
      // This means that this class can still be used for playback.
      MITK_WARN << "Caught exception during construction, but carrying on regardless:" << e;
    }
    m_TrackerSource = mitk::TrackingDeviceSource::New();
    m_TrackerSource->SetTrackingDevice(m_TrackerDevice);
  }
  else
  {
    try
    {
      mitk::TrackingDeviceSourceConfigurator::Pointer myConfigurator = mitk::TrackingDeviceSourceConfigurator::New(m_NavigationToolStorage, m_TrackerDevice.GetPointer());
      m_TrackerSource = myConfigurator->CreateTrackingDeviceSource();

      m_TrackerDevice->OpenConnection();
      m_TrackerDevice->StartTracking();
    }
    catch(const mitk::Exception& e)
    {
      // If we don't connect, we should still try to create tracker.
      // This means that this class can still be used for playback.
      MITK_WARN << "Caught exception during construction, but carrying on regardless:" << e;
    }
  }
}


//-----------------------------------------------------------------------------
MITKNDITracker::~MITKNDITracker()
{
  try
  {
    // One should not throw exceptions from a destructor.
    this->StopTracking();
    this->CloseConnection();
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Caught exception during destruction, but carrying on regardless:" << e;
  }
}


//-----------------------------------------------------------------------------
void MITKNDITracker::OpenConnection()
{
  // You should only call this from constructor.
  if (m_TrackerDevice->GetState() == mitk::TrackingDevice::Setup)
  {
    m_TrackerDevice->OpenConnection();
    if (m_TrackerDevice->GetState() != mitk::TrackingDevice::Ready)
    {
      mitkThrow() << "Failed to connect to tracker";
    }
    MITK_INFO << "Opened connection to tracker on port " << m_PortName;
  }
  else
  {
    mitkThrow() << "Tracking device is not setup correctly";
  }
}


//-----------------------------------------------------------------------------
void MITKNDITracker::CloseConnection()
{
  // You should only call this from destructor.
  if (m_TrackerDevice->GetState() == mitk::TrackingDevice::Ready)
  {
    m_TrackerDevice->CloseConnection();
    if (m_TrackerDevice->GetState() != mitk::TrackingDevice::Setup)
    {
      mitkThrow() << "Failed to disconnect from tracker";
    }
    MITK_INFO << "Closed connection to tracker on port " << m_PortName;
  }
  else
  {
    mitkThrow() << "Tracking device is not setup correctly";
  }
}


//-----------------------------------------------------------------------------
void MITKNDITracker::StartTracking()
{
  if (m_TrackerDevice->GetState() == mitk::TrackingDevice::Tracking)
  {
    return;
  }

  m_TrackerDevice->StartTracking();

  if (m_TrackerDevice->GetState() != mitk::TrackingDevice::Tracking)
  {
    mitkThrow() << "Failed to start tracking";
  }
  MITK_INFO << "Started tracking for " << m_TrackerDevice->GetToolCount() << " tools.";
}


//-----------------------------------------------------------------------------
void MITKNDITracker::StopTracking()
{
  if (m_TrackerDevice->GetState() == mitk::TrackingDevice::Ready)
  {
    return;
  }

  m_TrackerDevice->StopTracking();

  if (m_TrackerDevice->GetState() != mitk::TrackingDevice::Ready)
  {
    mitkThrow() << "Failed to stop tracking";
  }
  MITK_INFO << "Stopped tracking for " << m_TrackerDevice->GetToolCount() << " tools.";
}


//-----------------------------------------------------------------------------
bool MITKNDITracker::IsTracking() const
{
  return m_TrackerDevice->GetState() == mitk::TrackingDevice::Tracking;
}


//-----------------------------------------------------------------------------
std::map<std::string, std::pair<mitk::Point4D, mitk::Vector3D> > MITKNDITracker::GetTrackingData()
{
  std::map<std::string, std::pair<mitk::Point4D, mitk::Vector3D> > result;

  // So, if not tracking (e.g didn't connect to tracker),
  // we can still play-back, so we should not fail to return.
  if (m_TrackerDevice->GetState() != mitk::TrackingDevice::Tracking)
  {
    return result;
  }

  m_TrackerSource->Update();

  for(unsigned int i=0; i< m_TrackerSource->GetNumberOfOutputs(); i++)
  {
    mitk::NavigationData::Pointer currentTool = m_TrackerSource->GetOutput(i);
    if(currentTool.IsNotNull())
    {
      if (currentTool->IsDataValid())
      {
        std::string name = currentTool->GetName();
        mitk::Matrix3D rotation = currentTool->GetRotationMatrix();
        mitk::Point3D position = currentTool->GetPosition();

        vtkSmartPointer<vtkMatrix4x4> transform = vtkSmartPointer<vtkMatrix4x4>::New();
        transform->Identity();
        for (int r = 0; r < 3; r++)
        {
          for (int c = 0; c < 3; c++)
          {
            transform->SetElement(r, c, rotation[r][c]);
          }
          transform->SetElement(r, 3, position[r]);
        }

        // Converting to translation vector and quaternion.
        // Note: This is a bit inefficient. However, we don't currently
        // (meaning 'as of 2017-05-22'), use this class at all. So,
        // I'm just implementing this to satisfy the base class API.
        mitk::Point4D rotationQuaternion;
        mitk::Vector3D translation;
        niftk::ConvertMatrixToRotationAndTranslation(*transform, rotationQuaternion, translation);
        std::pair<mitk::Point4D, mitk::Vector3D> transformAsQuaternion(rotationQuaternion, translation);
        result.insert(std::pair<std::string, std::pair<mitk::Point4D, mitk::Vector3D> >(name, transformAsQuaternion));
      }
    }
  }
  return result;
}

} // end namespace
