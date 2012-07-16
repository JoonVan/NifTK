/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date$
 Revision          : $Revision$
 Last modified by  : $Author$

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef QMITKMIDASSINGLEVIEWWIDGETLISTDROPMANAGER_H_
#define QMITKMIDASSINGLEVIEWWIDGETLISTDROPMANAGER_H_

#include <niftkQmitkExtExports.h>
#include <vector>
#include "QmitkMIDASSingleViewWidgetListManager.h"
#include "mitkMIDASEnums.h"
#include "mitkDataStorage.h"

namespace mitk
{
class DataNode;
}

class QmitkRenderWindow;
class QmitkMIDASSingleViewWidgetListVisibilityManager;

/**
 * \class QmitkMIDASSingleViewWidgetListDropManager
 * \brief Class to coordinate the necessary operations for when we drop images into a
 * MIDAS QmitkMIDASMultiViewWidget, coordinating across many QmitkMIDASSingleViewWidget.
 *
 * This class needs to have SetVisibilityManager and SetDataStorage called prior to use.
 */
class NIFTKQMITKEXT_EXPORT QmitkMIDASSingleViewWidgetListDropManager : public QmitkMIDASSingleViewWidgetListManager
{

public:

  /// \brief Constructor.
  QmitkMIDASSingleViewWidgetListDropManager();

  /// \brief Destructor.
  virtual ~QmitkMIDASSingleViewWidgetListDropManager();

  /// \brief When nodes are dropped, we set all the default properties, and renderer specific visibility flags etc.
  void OnNodesDropped(QmitkRenderWindow *window, std::vector<mitk::DataNode*> nodes);

  /// \brief Set the visibility manager for this class to use.
  void SetVisibilityManager(QmitkMIDASSingleViewWidgetListVisibilityManager*);

  /// \brief Set the data storage.
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  /// \brief Sets the default view.
  void SetDefaultView(const MIDASView& view);

  /// \brief Gets the default view.
  MIDASView GetDefaultView() const;

  /// \brief Set the drop type, which controls the behaviour when multiple images are dropped into a single widget.
  void SetDropType(const MIDASDropType& dropType);

  /// \brief Get the drop type, which controls the behaviour when multiple images are dropped into a single widget.
  MIDASDropType GetDropType() const;

  /// \brief Set the flag to determine if we accumulate images to a single geometry.
  void SetAccumulateWhenDropped(const bool& accumulateWhenDropped);

  /// \brief Get the flag to determine if we accumulate images to a single geometry.
  bool GetAccumulateWhenDropped() const;

protected:

private:

  MIDASView m_DefaultView;
  MIDASDropType m_DropType;
  bool m_AccumulateWhenDropped;
  mitk::DataStorage::Pointer m_DataStorage;
  QmitkMIDASSingleViewWidgetListVisibilityManager* m_VisibilityManager;
};

#endif
