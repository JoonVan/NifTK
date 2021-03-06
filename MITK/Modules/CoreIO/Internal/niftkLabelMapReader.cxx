/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkLabelMapReader.h"
#include "niftkCoreIOMimeTypes.h"
#include "niftkLookupTableContainer.h"

#include <mitkCustomMimeType.h>
#include <mitkLogMacros.h>
#include <mitkGeometryData.h>

#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <vtkIntArray.h>
#include <vtkStringArray.h>
#include <QFile>
#include <sstream>
#include <iostream>


namespace niftk
{

//-----------------------------------------------------------------------------
LabelMapReader::LabelMapReader()
: mitk::AbstractFileReader(mitk::CustomMimeType(niftk::CoreIOMimeTypes::LABELMAP_MIMETYPE_NAME()),
                           niftk::CoreIOMimeTypes::LABELMAP_MIMETYPE_DESCRIPTION())
, m_Order(0)
{
  m_ServiceReg = this->RegisterService();
}


//-----------------------------------------------------------------------------
LabelMapReader::LabelMapReader(const LabelMapReader &other)
: mitk::AbstractFileReader(other)
, m_Order(0)
{
}


//-----------------------------------------------------------------------------
LabelMapReader * LabelMapReader::Clone() const
{
  return new niftk::LabelMapReader(*this);
}


//-----------------------------------------------------------------------------
std::vector<itk::SmartPointer<mitk::BaseData> > LabelMapReader::Read()
{
  // make sure the internal datatypes are empty
  m_Labels.clear();
  m_Colors.clear();

  std::vector<itk::SmartPointer<mitk::BaseData> > result;

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );
  setlocale(LC_ALL, locale.c_str());

  std::string fileName = this->GetInputLocation();
  QString labelName = QString::fromStdString(fileName);
  bool isLoaded = false;

  if (fileName.find(":") == 0)
  {
    QFile lutFile(fileName.c_str());
    lutFile.open(QIODevice::ReadOnly);

    // this is a dirty hack to get the resource file in the right format to read
    std::string fileStr(lutFile.readAll());
    std::stringstream sStream;
    sStream << fileStr;
    isLoaded = this->ReadLabelMap(sStream);
    lutFile.close();
  }
  else
  {
    // detect if : starts the fileName
    std::ifstream infile(fileName, std::ifstream::in);

    if (infile.is_open())
    {
      isLoaded = this->ReadLabelMap(infile);
      infile.close();
    }
  }
  if (isLoaded)
  {
    int startInd = labelName.lastIndexOf("/") + 1;
    int endInd = labelName.lastIndexOf(".");
    m_DisplayName = labelName.mid(startInd, endInd - startInd);
    setlocale(LC_ALL, currLocale.c_str());
    MITK_DEBUG << "NifTK label map read.";
  }
  else
  {
    result.clear();
    MITK_ERROR << "Unable to read NifTK label map!";
  }

  LookupTableContainer::Pointer containter = GetLookupTableContainer();
  result.push_back(itk::SmartPointer<mitk::BaseData>(containter));
  return result;
}


//-----------------------------------------------------------------------------
bool LabelMapReader::ReadLabelMap(std::istream & file)
{
  bool isLoaded = false;

  //while there are still lines in the file, keep reading:
  while (!file.eof())
  {
    std::string line;
    //place the line from input into the raw string
    getline(file, line);

    if (line.empty() || line.at(0) == '#' || line == "\r")
    {
      continue;
    }

    try
    {
      int value = 0;
      int red = 0;
      int green = 0;
      int blue = 0;
      int alpha = 0;
      
      // find value
      size_t firstSpace = line.find_first_of(' ');
      std::string firstDigit = line.substr(0, firstSpace);
      sscanf(firstDigit.c_str(), "%10i", &value);

      // find name
      size_t firstLtr = line.find_first_not_of(' ', firstSpace);
      size_t lastLtr  = line.find_first_of(' ', firstLtr);

      std::string nameInFile = line.substr(firstLtr, (lastLtr) - firstLtr);
      QString name = QString::fromStdString(nameInFile);

      // if the name is just the special character set as empty
      if (name.compare(QString('*')) == 0)
        name.clear();

      name.replace('*',' '); // swapping the white space back in

      // colors;
      std::string colorStr = line.substr(lastLtr, line.size() - lastLtr);
      sscanf(colorStr.c_str(), "%3i %3i %3i %3i", &red, &green, &blue, &alpha);

      LookupTableContainer::LabelType label = std::make_pair(value, name);
      m_Labels.push_back(label);

      QColor fileColor(red, green, blue, alpha);
      m_Colors.push_back(fileColor);

      isLoaded = true;
    }
    catch(const std::exception &)
    {
      std::cout <<"Unable to parse line " << line.c_str() << ". Skipping." << std::endl;
    }
  }
  
  file.clear();
  return isLoaded;
}


//-----------------------------------------------------------------------------
LookupTableContainer* LabelMapReader::GetLookupTableContainer()
{
  if (m_Colors.empty() || m_Labels.empty())
  {
    return NULL;
  }
  
  MITK_DEBUG << "GetLookupTableContainer():labels.size()=" << m_Labels.size();

  // get the size of vtkLUT from the range of values
  int max = m_Labels.at(0).first;

  for (unsigned int i = 1; i < m_Labels.size(); i++)
  {
    int val = m_Labels.at(i).first;
    if (val > max)
    {
      max = val;
    }
  }

  vtkSmartPointer<vtkLookupTable> lookupTable = vtkLookupTable::New();
  
  /**
   * To initialize a table with all values for one default color
   * (black,completely transparent), I restrict all of the ranges.
   */
  lookupTable->SetValueRange(0,0);
  lookupTable->SetHueRange(0,0);
  lookupTable->SetSaturationRange(0,0);
  lookupTable->SetAlphaRange(0,0);

  /**
   * Number of table values: to map values above/below range to
   * the default color, define table value above/below label range.
   */
  int numberOfValues = max + 2;
  lookupTable->SetNumberOfTableValues(numberOfValues);
  lookupTable->SetTableRange(0, max);
  lookupTable->SetNanColor(0, 0, 0, 0);

  lookupTable->SetIndexedLookup(true);
  lookupTable->Build();

  vtkSmartPointer<vtkIntArray> annotationValueArray = vtkIntArray::New();
  vtkSmartPointer<vtkStringArray> annotationNameArray = vtkStringArray::New();

  // iterate and assign each color value
  for (unsigned int i = 0; i < m_Colors.size(); i++)
  {
    int vtkInd = m_Labels.at(i).first;
    std::string name = m_Labels.at(i).second.toStdString();

    double r = m_Colors.at(i).redF();
    double g = m_Colors.at(i).greenF();
    double b = m_Colors.at(i).blueF();
    double a = m_Colors.at(i).alphaF();

    lookupTable->SetTableValue(vtkInd, r, g, b, a);

    annotationValueArray->InsertValue(vtkInd, vtkInd);
    annotationNameArray->InsertValue(vtkInd, name);
  }

  lookupTable->SetAnnotations(annotationValueArray, annotationNameArray);

  // place into container
  LookupTableContainer* lookupTableContainer = new LookupTableContainer(lookupTable, m_Labels);
  lookupTableContainer->SetDisplayName(m_DisplayName);
  lookupTableContainer->SetOrder(m_Order);

  return lookupTableContainer;
}

}
