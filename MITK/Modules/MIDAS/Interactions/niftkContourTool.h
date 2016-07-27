/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkContourTool_h
#define niftkContourTool_h

#include "niftkMIDASExports.h"

#include <mitkContourModel.h>
#include <mitkContourModelSet.h>
#include <mitkMessage.h>
#include <mitkOperation.h>
#include <mitkOperationActor.h>

#include <niftkPointUtils.h>

#include "niftkContourToolEventInterface.h"
#include "niftkTool.h"

namespace niftk
{

/**
 * \class ContourTool
 * \brief Provides common functionality for DrawTool and PolyTool
 * where these two tools enable drawing lines and poly-lines around voxel edges.
 *
 * This class derives from mitk::FeedbackContourTool, and uses several contours to
 * do its magic.  The base class "FeedbackContour" is the one that is visible as the tool
 * is used. In addition, in this class we store a "BackgroundContour". The FeedbackContour
 * goes round the edges of each voxel, and the BackgroundContour simply stores each
 * mouse position, as each mouse event is received, and hence contains the trajectory of the
 * mouse.
 *
 * \sa mitk::FeedbackContourTool
 * \sa niftk::Tool
 * \sa niftk::DrawTool
 * \sa niftk::PolyTool
 */
class NIFTKMIDAS_EXPORT ContourTool : public Tool {

public:

  mitkClassMacro(ContourTool, Tool)

  /// \brief We store the name of a property to say we are editing.
  static const std::string EDITING_PROPERTY_NAME;

  /// \brief We store the name of the background contour, which is the contour storing exact mouse position events.
  static const std::string MIDAS_CONTOUR_TOOL_BACKGROUND_CONTOUR;

  /// \brief Method to enable this class to interact with the Undo/Redo framework.
  virtual void ExecuteOperation(mitk::Operation* operation);

  /// \brief Clears the contour, meaning it re-initialised the feedback contour in mitk::FeedbackContourTool, and also the background contour herein.
  virtual void ClearData();

  /// \brief Get a pointer to the current feedback contour.
  virtual mitk::ContourModel* GetContour();

  /// \brief Turns the feedback contour on/off.
  virtual void SetFeedbackContourVisible(bool);

  /// \brief Copies contour from a to b.
  static void CopyContour(mitk::ContourModel &a, mitk::ContourModel &b);

  /// \brief Copies contour set from a to b.
  static void CopyContourSet(mitk::ContourModelSet &a, mitk::ContourModelSet &b, bool initialise=true);

  /// \brief Initialises the output contour b with properties like, closed, width and selected, copied from the reference contour a.
  static void InitialiseContour(mitk::ContourModel &a, mitk::ContourModel &b);

  /// \brief Used to signal that the contours have changed.
  mitk::Message<> ContoursHaveChanged;

protected:

  ContourTool(); // purposely hidden
  virtual ~ContourTool(); // purposely hidden

  /// \brief Calls the FeedbackContour::OnMousePressed method, then checks for working image, reference image and geometry.
  virtual bool OnMousePressed(mitk::StateMachineAction* action, mitk::InteractionEvent* event);

  /// \brief This method makes sure that the argument node will not show up in ANY 3D viewer thats currently registered with the global mitk::RenderingManager.
  void Disable3dRenderingOfNode(mitk::DataNode* node);

  /// \brief Adds the given contour to the Working Data registered with mitk::ToolManager, where the ToolManager can have multiple data sets registered, so we add the contour to the dataset specified by dataSetNumber.
  void AccumulateContourInWorkingData(mitk::ContourModel& contour, int contourIndex);

  // Utility methods for helping draw lines that require m_Geometry to be set.
  void ConvertPointInMmToVx(const mitk::Point3D& pointInMm, mitk::Point3D& pointInVx);
  void ConvertPointToNearestVoxelCentreInVx(const mitk::Point3D& pointInMm, mitk::Point3D& nearestVoxelCentreInVx);
  void ConvertPointToNearestVoxelCentreInMm(const mitk::Point3D& pointInMm, mitk::Point3D& nearestVoxelCentreInMm);
  void GetClosestCornerPoint2D(const mitk::Point3D& pointInMm, int* whichTwoAxesInVx, mitk::Point3D& cornerPointInMm);

  /// \brief Compares two coordinates of the points and tells which of them are equal.
  /// The axes of the two coordinates to compare are given in @a whichTwoAxesInVx.
  /// Returns 0 if both coordinates are different, 1 if only the first coordinates are equal,
  /// 2 if only the second coordinates are equal and 3 the points are the same.
  int GetEqualCoordinateAxes(const mitk::Point3D& cornerPoint1InMm, const mitk::Point3D& cornerPoint2InMm, int* whichTwoAxesInVx);

  void GetAdditionalCornerPoint(const mitk::Point3D& cornerPoint1InMm, const mitk::Point3D& point2InMm, const mitk::Point3D& cornerPoint2InMm, int* whichTwoAxesInVx, mitk::Point3D& newCornerPointInMm);

  // Main method for drawing a line:
  //   1.) from previousPoint to currentPoint working around voxel corners, output in contourAroundCorners
  //   2.) from previousPoint to currentPoint working in a straight line, output in contourAlongLine
  // Returns true if new points added to contourAroundCorners or the last point updated, otherwise false.
  bool DrawLineAroundVoxelEdges(
      const mitk::Image* image,                 // input
      const mitk::BaseGeometry* geometry,       // input
      const mitk::PlaneGeometry* planeGeometry, // input
      const mitk::Point3D& currentPoint,        // input
      const mitk::Point3D& previousPoint,       // input
      mitk::ContourModel& contourAroundCorners,      // output
      mitk::ContourModel& contourAlongLine           // output
      );

  // Methods for manipulating the "BackgroundContour", which typically doesn't get drawn, but is useful for converting to image coordinates, e.g. for rendering into images for boundaries.
  mitk::ContourModel* GetBackgroundContour();
  void SetBackgroundContour(mitk::ContourModel&);
  void Disable3dRenderingOfBackgroundContour();
  void SetBackgroundContourVisible(bool);
  void SetBackgroundContourColor( float r, float g, float b );
  void SetBackgroundContourColorDefault();

  // We default this to 1, and use throughout.
  int m_ContourWidth;

  // We default this to false, and use throughout.
  bool m_ContourClosed;

  // We default this to 0.01, and use throughout when comparing point positions.
  float m_Tolerance;

  // This is the 3D geometry associated with the m_WorkingImage
  mitk::BaseGeometry* m_SegmentationImageGeometry;

  // This is the current 3D working image (the image that is the segmentation, i.e. a binary image)
  mitk::Image* m_SegmentationImage;

  // This is the current 3D reference image (the image that is being segmented, i.e. a grey scale image)
  mitk::Image* m_ReferenceImage;

  // Like the base class mitkFeedbackContourTool, we keep a contour that is the straight line, exaclty as we iterate, not working around voxel corners.
  mitk::ContourModel::Pointer  m_BackgroundContour;
  mitk::DataNode::Pointer m_BackgroundContourNode;
  bool                    m_BackgroundContourVisible;

private:

  // Operation constant, used in Undo/Redo framework.
  static const mitk::OperationType MIDAS_CONTOUR_TOOL_OP_ACCUMULATE_CONTOUR;

  /// \brief Pointer to interface object, used as callback in Undo/Redo framework
  ContourToolEventInterface::Pointer m_Interface;

};

}

#endif
