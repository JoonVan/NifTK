/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkMIDASPaintbrushTool_h
#define mitkMIDASPaintbrushTool_h

#include "niftkMIDASExports.h"

#include <itkImage.h>
#include <itkMIDASImageUpdatePixelWiseSingleValueProcessor.h>

#include <mitkGeometry3D.h>
#include <mitkImage.h>
#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkSegTool2D.h>
#include <usServiceReference.h>

#include "mitkMIDASPaintbrushToolOpEditImage.h"
#include "mitkMIDASPaintbrushToolEventInterface.h"
#include "mitkMIDASStateMachine.h"

namespace mitk
{

 /**
  * \class MIDASPaintbrushTool
  * \brief MIDAS paint brush tool used during editing on the morphological editor screen (a.k.a connection breaker).
  *
  * Note the following:
  * <pre>
  * 1.) Writes into 4 images, so ToolManager must have 4 working volume to edit into.
  *     We define Working Image[0] = "additions image for erosions", which is added to the main segmentation to add stuff back into the volume.
  *     We define Working Image[1] = "subtractions image for erosions", which is subtracted from the main segmentation to do connection breaking.
  *     We define Working Image[2] = "additions image for dilations", which is added to the main segmentation to add stuff back into the volume.
  *     We define Working Image[3] = "subtractions image for dilations", which is subtracted from the main segmentation to do connection breaking.
  * 2.) Then:
  *     Left mouse = paint into the "additions image".
  *     Middle mouse = paint into the "subtractions image".
  *     Right mouse = subtract from the "subtractions image".
  * 3.) We derive from SegTool2D to keep things simple, as we just need to convert from mm world points to voxel points, and paint.
  * 4.) Derives from mitk::OperationActor, so this tool supports undo/redo.
  * </pre>
  *
  * This class is a MITK tool with a GUI defined in QmitkMIDASPaintbrushToolGUI, and instantiated
  * using the object factory described in Maleike et. al. doi:10.1016/j.cmpb.2009.04.004.
  *
  * To effectively use this tool, you need a 3 button mouse.
  *
  * Trac 1695, 1700, 1701, 1706: Fixing up dilations: We change pipeline so that WorkingData 0,1 are
  * applied during erosions phase, and WorkingData 2,3 are applied during dilations phase.
  */
class NIFTKMIDAS_EXPORT MIDASPaintbrushTool : public SegTool2D//, public MIDASStateMachine
{

public:

  /// \brief Constants that identify the data needed for the morphological edit tools.
  /// They should be used for indexing the vector of working data.
  enum
  {
    EROSIONS_ADDITIONS,
    EROSIONS_SUBTRACTIONS,
    DILATIONS_ADDITIONS,
    DILATIONS_SUBTRACTIONS
  };

  /// \brief Stores the name of the MIDAS additions image, used in Morphological Editor.
  static const std::string EROSIONS_ADDITIONS_NAME;

  /// \brief Stores the name of the MIDAS connection breaker image, used in Morphological Editor.
  static const std::string EROSIONS_SUBTRACTIONS_NAME;

  /// \brief Stores the name of the MIDAS additions image, used in Morphological Editor.
  static const std::string DILATIONS_ADDITIONS_NAME;

  /// \brief Stores the name of the MIDAS connection breaker image, used in Morphological Editor.
  static const std::string DILATIONS_SUBTRACTIONS_NAME;


  mitkClassMacro(MIDASPaintbrushTool, SegTool2D);
  itkNewMacro(MIDASPaintbrushTool);

  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, 3> ImageType;
  typedef itk::MIDASImageUpdatePixelWiseSingleValueProcessor<mitk::Tool::DefaultSegmentationDataType, 3> ProcessorType;

  virtual void InitializeStateMachine();

  /** Strings to help the tool identify itself in GUI. */
  virtual const char* GetName() const;
  virtual const char** GetXPM() const;

  /** We store the name of a property that stores the image region. */
  static const std::string REGION_PROPERTY_NAME;

  /// \brief Gets the cursor size.
  /// Default size is 1 pixel.
  int GetCursorSize() const;

  /// \brief Sets the cursor size.
  void SetCursorSize(int cursorSize);

  /// \brief Gets the erosion mode.
  /// If true, we are editing image 0,1, and if false, we are editing image 2,3. Default true.
  bool GetErosionMode() const;

  /// \brief Sets the erosion mode.
  /// If true, we are editing image 0,1, and if false, we are editing image 2,3. Default true.
  void SetErosionMode(bool erosionMode);

  /** Used to send messages when the cursor size is changed or should be updated in a GUI. */
  Message1<int> CursorSizeChanged;

  /** Method to enable this class to interact with the Undo/Redo framework. */
  virtual void ExecuteOperation(Operation* operation);

  /** Process all mouse events. */
  virtual bool StartAddingAddition(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool KeepAddingAddition(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StopAddingAddition(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StartAddingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool KeepAddingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StopAddingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StartRemovingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool KeepRemovingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);
  virtual bool StopRemovingSubtraction(mitk::StateMachineAction* action, mitk::InteractionEvent* event);

protected:

  MIDASPaintbrushTool();          // purposely hidden
  virtual ~MIDASPaintbrushTool(); // purposely hidden

  /// \brief Connects state machine actions to functions.
  virtual void ConnectActionsAndFunctions();

  /// \brief Tells if this tool can handle the given event.
  ///
  /// This implementation delegates the call to mitk::MIDASStateMachine::CanHandleEvent(),
  /// that checks if the event is filtered by one of the installed event filters and if not,
  /// calls CanHandle() and returns with its result.
  ///
  /// Note that this function is purposefully not virtual. Eventual subclasses should
  /// override the CanHandle function.
  bool FilterEvents(mitk::InteractionEvent* event, mitk::DataNode* dataNode);

  /**
  \brief Called when the tool gets activated (registered to mitk::GlobalInteraction).

  Derived tools should call their parents implementation.
  */
  virtual void Activated();

  /**
  \brief Called when the tool gets deactivated (unregistered from mitk::GlobalInteraction).

  Derived tools should call their parents implementation.
  */
  virtual void Deactivated();

private:

  // Operation constant, used in Undo/Redo framework.
  static const mitk::OperationType MIDAS_PAINTBRUSH_TOOL_OP_EDIT_IMAGE;

  ///
  /// \brief Used for working out which voxels to edit.
  ///
  /// Essentially, we take two points, currentPoint and previousPoint in millimetre space
  /// and step along a line between them. At each step we convert from millimetres to voxels,
  /// and that list of voxels is the affected region.
  void GetListOfAffectedVoxels(
      const PlaneGeometry& planeGeometry,
      Point3D& currentPoint,
      Point3D& previousPoint,
      ProcessorType &processor);

  /// \brief Marks the initial mouse position when any of the left/middle/right mouse buttons are pressed.
  bool MarkInitialPosition(unsigned int dataIndex, mitk::StateMachineAction* action, mitk::InteractionEvent* event);

  /// \brief Sets an invalid region (indicating that we are not editing) on the chosen image number data node.
  void SetInvalidRegion(unsigned int dataIndex);

  /// \brief Sets a valid region property, taken from the bounding box of edited voxels, indicating that we are editing the given image number.
  void SetValidRegion(unsigned int dataIndex, const std::vector<int>& boundingBox);

  /// \brief Method that actually sets the region property on a working image.
  void SetRegion(unsigned int dataIndex, bool valid, const std::vector<int>& boundingBox = std::vector<int>());

  /// \brief Does the main functionality when the mouse moves.
  bool DoMouseMoved(mitk::StateMachineAction* action, mitk::InteractionEvent* event,
      int dataIndex,
      unsigned char valueForRedo,
      unsigned char valueForUndo
      );

  /// \brief Using the MITK to ITK access functions to run the ITK processor object.
  template<typename TPixel, unsigned int VImageDimension>
  void RunITKProcessor(
      itk::Image<TPixel, VImageDimension>* itkImage,
      ProcessorType::Pointer processor,
      bool redo,
      unsigned char valueToWrite
      );

  // Pointer to interface object, used as callback in Undo/Redo framework
  MIDASPaintbrushToolEventInterface::Pointer m_Interface;

  /// \brief Calculates the current image number.
  int GetDataIndex(bool isLeftMouseButton);

  // Cursor size for editing, and cursor type is currently always a cross.
  int m_CursorSize;

  // This is the 3D geometry associated with the m_WorkingImage, where we assume both working images have same size and geometry.
  mitk::Geometry3D* m_WorkingImageGeometry;

  // This points to the current working image, assuming that we are only ever processing, left, middle or right mouse button at any one time.
  mitk::Image* m_WorkingImage;

  // Used between MouseDown and MouseMoved events to track movement.
  mitk::Point3D m_MostRecentPointInMillimetres;

  // If m_ErosionMode is true, we update WorkingData 0 and 1, if m_ErosionMode is false, we update WorkingData 2 and 3.
  bool m_ErosionMode;

  // Stores the current display interactor configurations when this tool is activated.
  // The configurations are restored when the tool is deactivated.
  std::map<us::ServiceReferenceU, mitk::EventConfig> m_DisplayInteractorConfigs;

};//class

}//namespace

#endif
