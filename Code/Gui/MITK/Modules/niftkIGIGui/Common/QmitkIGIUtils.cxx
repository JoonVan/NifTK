/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkIGIUtils.h"
#include <QFile>
#include <QMessageBox>
#include <mitkSTLFileReader.h>
#include <mitkFileIOUtils.h>
#include <mitkDataStorageUtils.h>
#include <igtlStringMessage.h>
#include <NiftyLinkSocketObject.h>
#include <Common/NiftyLinkXMLBuilder.h>

#include <vtkCubeSource.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>


//-----------------------------------------------------------------------------
mitk::Surface::Pointer LoadSurfaceFromSTLFile(const QString& surfaceFilename)
{
  mitk::Surface::Pointer toolSurface;

  QFile surfaceFile(surfaceFilename);

  if(surfaceFile.exists())
  {
    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();

    try
    {
      stlReader->SetFileName(surfaceFilename.toStdString().c_str());
      stlReader->Update();//load surface
      toolSurface = stlReader->GetOutput();
    }
    catch (std::exception& e)
    {
      MBI_ERROR<<"Could not load surface for tool!";
      MBI_ERROR<< e.what();
      throw e;
    }
  }

  return toolSurface;
}


//-----------------------------------------------------------------------------
QString CreateTestDeviceDescriptor()
{
  TrackerClientDescriptor tcld;
  tcld.SetDeviceName("NDI Polaris Vicra");
  tcld.SetDeviceType("Tracker");
  tcld.SetCommunicationType("Serial");
  tcld.SetPortName("Tracker not connected");
  tcld.SetClientIP(GetLocalHostAddress());
  tcld.SetClientPort(QString::number(3200));
  //tcld.AddTrackerTool("8700302.rom");
  tcld.AddTrackerTool("8700338.rom");
  //tcld.AddTrackerTool("8700339.rom");
  tcld.AddTrackerTool("8700340.rom");

  return tcld.GetXMLAsString();
}


//-----------------------------------------------------------------------------
QString ConvertNanoSecondsToString(const igtlUint32& nanosec)
{
  QString result = QString("%1").arg(nanosec, 9, 10, QChar('0'));
  return result;
}


//-----------------------------------------------------------------------------
bool SaveMatrixToFile(const vtkMatrix4x4& matrix, const QString& fileName)
{
  bool isSuccessful = false;

  if (fileName.length() == 0)
  {
    QMessageBox msgBox;
    msgBox.setText("The file name is empty.");
    msgBox.setInformativeText("Please select a file name.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
    return isSuccessful;
  }

  isSuccessful = mitk::SaveVtkMatrix4x4ToFile(fileName.toStdString(), matrix);

  if (!isSuccessful)
  {
    QMessageBox msgBox;
    msgBox.setText("The file failed to save.");
    msgBox.setInformativeText("Please check the file location.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
    return isSuccessful;
  }

  return isSuccessful;
}


//-----------------------------------------------------------------------------
// Deliberately not exposed.
//-----------------------------------------------------------------------------
void ApplyMatricesToAllTransformsInCheckbox(const vtkMatrix4x4& transform, const QmitkDataStorageCheckableComboBox& comboBox, bool composeRatherThanSet)
{
  std::vector<mitk::DataNode*> nodes = comboBox.GetSelectedNodes();
  mitk::DataNode::Pointer node = NULL;
  mitk::BaseData::Pointer data = NULL;

  if (nodes.size() == 0)
  {
    QMessageBox msgBox;
    msgBox.setText("There are no items selected.");
    msgBox.setInformativeText("Please select a valid data item.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
    return;
  }

  for (unsigned int i = 0; i < nodes.size(); ++i)
  {
    node = nodes[i];

    if (node.IsNotNull())
    {
      data = dynamic_cast<mitk::BaseData*>(node->GetData());
    }

    if (data.IsNull())
    {
      QMessageBox msgBox;
      msgBox.setText(QString("The data set for item ") + QString::fromStdString(node->GetName()) + QString("%1 is non-existent or does not contain data."));
      msgBox.setInformativeText("Please select a valid data set.");
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setDefaultButton(QMessageBox::Ok);
      msgBox.exec();
    }

    // These throw exceptions if there is any error.
    if (composeRatherThanSet)
    {
      mitk::ComposeTransformWithNode(transform, node);
    }
    else
    {
      mitk::ApplyTransformToNode(transform, node);
    }

  } // end foreach node
}


//-----------------------------------------------------------------------------
void ApplyTransformToNode(const vtkMatrix4x4& transform, const QmitkDataStorageCheckableComboBox& comboBox)
{
  ApplyMatricesToAllTransformsInCheckbox(transform, comboBox, false);
}


//-----------------------------------------------------------------------------
void ComposeTransformWithNode(const vtkMatrix4x4& transform, const QmitkDataStorageCheckableComboBox& comboBox)
{
  ApplyMatricesToAllTransformsInCheckbox(transform, comboBox, true);
}


//-----------------------------------------------------------------------------
mitk::Surface::Pointer MakeAWall ( const int& whichwall, const float& size, 
   const float& xOffset,  const float& yOffset,  const float& zOffset , 
   const float& thickness ) 
{
  vtkSmartPointer<vtkCubeSource> wall =  vtkSmartPointer<vtkCubeSource>::New();

  switch ( whichwall )
  {
    case 0: //the back wall
    {
      wall->SetXLength(size);
      wall->SetYLength(size);
      wall->SetZLength(thickness);
      wall->SetCenter(size * xOffset, size * yOffset, 
          size * zOffset + size * 0.5 + thickness * 0.5);
      break;
    }
    case 1: //the left wall
    {
      wall->SetXLength(size);
      wall->SetYLength(thickness);
      wall->SetZLength(size);
      wall->SetCenter(size * xOffset,
          size * yOffset + size * 0.5 + thickness * 0.5, size * zOffset) ;
      break;
    }
    case 2: //the front wall
    {
      wall->SetXLength(size);
      wall->SetYLength(size);
      wall->SetZLength(thickness);
      wall->SetCenter(size * xOffset, size * yOffset, 
          size * zOffset - size * 0.5 - thickness * 0.5);
      break;
    }
    case 3: //the right wall
    {
      wall->SetXLength(size);
      wall->SetYLength(thickness);
      wall->SetZLength(size);
      wall->SetCenter(size * xOffset,
          size * yOffset - size * 0.5 - thickness * 0.5, size * zOffset) ;
      break;
    }
    case 4: //the ceiling
    {
      wall->SetXLength(thickness);
      wall->SetYLength(size);
      wall->SetZLength(size);
      wall->SetCenter(size * xOffset + size * 0.5 + thickness * 0.5,
          size * yOffset, size * zOffset) ;
      break;
    }
    case 5: //the floor
    {
      wall->SetXLength(thickness);
      wall->SetYLength(size);
      wall->SetZLength(size);
      wall->SetCenter(size * xOffset - size * 0.5 - thickness * 0.5,
          size * yOffset, size * zOffset) ;
      break;
    }
    default: //a mistake
    {
      MITK_WARN << "Passed a bad number to MakeAWall : " << whichwall;
      return NULL;
    }
  }
  vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New(); 
  poly=wall->GetOutput();
  mitk::Surface::Pointer surface;
  surface->SetVtkPolyData(poly);
  return surface;

}
