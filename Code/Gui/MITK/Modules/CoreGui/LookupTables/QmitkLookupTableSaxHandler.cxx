/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkLookupTableSaxHandler.h"
#include "QmitkLookupTableContainer.h"
#include <QMessageBox>
#include <vtkLookupTable.h>
#include <mitkLogMacros.h>

//-----------------------------------------------------------------------------
QmitkLookupTableSaxHandler::QmitkLookupTableSaxHandler()
{
  m_IsPreMultiplied = false;
  m_IsScaled = true;
  m_Order = -1;
  m_DisplayName = QString("None");
  m_List.clear();
}


//-----------------------------------------------------------------------------
QmitkLookupTableContainer* QmitkLookupTableSaxHandler::GetLookupTableContainer()
{
  MITK_DEBUG << "GetLookupTableContainer():list.size()=" << m_List.size();

  vtkLookupTable *lookupTable = vtkLookupTable::New();
  lookupTable->SetRampToLinear();
  lookupTable->SetScaleToLinear();
  lookupTable->SetNumberOfTableValues(m_List.size());

  for (unsigned int i = 0; i < m_List.size(); i++)
  {
    QColor c = m_List[i];
    lookupTable->SetTableValue(i, c.redF(), c.greenF(), c.blueF(), c.alphaF());
  }

  QmitkLookupTableContainer *lookupTableContainer = new QmitkLookupTableContainer(lookupTable);
  lookupTableContainer->SetOrder(m_Order);
  lookupTableContainer->SetDisplayName(m_DisplayName);
  lookupTableContainer->SetIsScaled(m_IsScaled);

  return lookupTableContainer;
}


//-----------------------------------------------------------------------------
bool QmitkLookupTableSaxHandler
::startElement(
    const QString& /* namespaceURI */,
    const QString& /* localName */,
    const QString& qName,
    const QXmlAttributes& attributes
	)
{
  if (qName == "lut")
  {
    m_Order = (attributes.value("order")).toInt();
    m_DisplayName = attributes.value("displayName");
    int premultiplied = (attributes.value("premultiplied")).toInt();
    if (premultiplied == 1)
    {
      m_IsPreMultiplied = true;
    }

    QString scaledStr = attributes.value("isScaled");
    bool isScaled = scaledStr.toInt();
    if (!scaledStr.isEmpty() && isScaled == 0)
    {
      m_IsScaled = false;
	} 
  }
  else if (qName == "colour")
  {
    float red = (attributes.value("r")).toFloat();
    float green = (attributes.value("g")).toFloat();
    float blue = (attributes.value("b")).toFloat();

    if (!m_IsPreMultiplied)
    {
      red /= 255;
      green /= 255;
      blue /= 255;
    }

    QColor tmp;
    tmp.setRedF(red);
    tmp.setGreenF(green);
    tmp.setBlueF(blue);

    m_List.push_back(tmp);
  }
  else
  {
    MITK_ERROR << "startElement():qName=" << qName.toLocal8Bit().constData() << ", which is unrecognised";
    return false;
  }
  
  return true;
}


//-----------------------------------------------------------------------------
bool QmitkLookupTableSaxHandler::characters(const QString &str)
{
  return true;
}


//-----------------------------------------------------------------------------
bool QmitkLookupTableSaxHandler
::endElement(
    const QString & /* namespaceURI */,
    const QString & /* localName */,
    const QString &qName
	)
{
  return true;
}


//-----------------------------------------------------------------------------
bool QmitkLookupTableSaxHandler::fatalError(const QXmlParseException &exception)
{
  QMessageBox::warning(0, QObject::tr("SAX Handler"),
                          QObject::tr("Parse error at line %1, column %2:\n%3.")
                         .arg(exception.lineNumber())
                         .arg(exception.columnNumber())
                         .arg(exception.message()));
  return false;
}
