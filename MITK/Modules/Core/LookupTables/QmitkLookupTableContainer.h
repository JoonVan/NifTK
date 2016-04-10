/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkLookupTableContainer_h
#define QmitkLookupTableContainer_h

#include <niftkCoreExports.h>

#include <QString>
#include <vtkLookupTable.h>

/**
 * \class QmitkLookupTableContainer
 * \brief Class to contain a vtkLookupTable and to store meta-data attributes
 * like display name, which order to display it in in GUI, etc.
 */
class NIFTKCORE_EXPORT QmitkLookupTableContainer 
{

public:

  typedef std::pair<int, QString> LabelType;
  typedef std::vector<LabelType> LabelListType; 

  /** Constructor that takes a lookup table. */
  QmitkLookupTableContainer(const vtkLookupTable* lut);

  /** Constructor that takes a lookup table and a set of labels*/
  QmitkLookupTableContainer(const vtkLookupTable* lut, const LabelListType& labels);

  /** Destructor. */
  virtual ~QmitkLookupTableContainer();

  /** Get the vtkLookupTable. */
  const vtkLookupTable* GetLookupTable() const { return m_LookupTable; }

  /** Set the order that determines which order this vtkLookupTable will be displayed in GUI. */
  void SetOrder(int i) { m_Order = i;}

  /** Get the order that this lookup table will be displayed in GUI. */
  int GetOrder() const { return m_Order; }

  /** Set the display name. */
  void SetDisplayName(const QString s) { m_DisplayName = s; }

  /** Get the display name. */
  QString GetDisplayName() const { return m_DisplayName; }

  /** Set scaled property. */
  void SetIsScaled(bool s) { m_IsScaled = s; }

  /** Get scaled property. */
  bool GetIsScaled() const { return m_IsScaled; }
 

  /** Set labels. */
  void SetLabels(const LabelListType& labels){ m_Labels = labels; }
  
  /** Get labels. */
 LabelListType GetLabels()const { return m_Labels; }


private:

  /** Deliberately prohibit copy constructor. */
  QmitkLookupTableContainer(const QmitkLookupTableContainer&) {}

  /** Deliberately prohibit assignment. */
  void operator=(const QmitkLookupTableContainer&) {}

  /** This is it! */
  const vtkLookupTable* m_LookupTable;

  /** Store the order that it is to be displayed in GUI. */
  int m_Order;

  /** What type of lookup table */
  bool m_IsScaled;

  /** Display name for display in GUI. */
  QString m_DisplayName;

  /** Labels for the entries in the vtkLUT (optional)*/
  LabelListType m_Labels;

};

#endif
