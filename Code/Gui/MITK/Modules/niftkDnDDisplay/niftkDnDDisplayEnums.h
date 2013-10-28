/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkDnDDisplayEnums_h
#define niftkDnDDisplayEnums_h

/*!
 * \file niftkDnDDisplayEnums.h
 * \brief Contains MIDAS enums, which we move out of the classes, so
 * they are independent, which makes manually analysing classes for
 * their include dependencies a bit easier.
 */

/*!
 * \enum MIDASLayout
 * \brief Describes the different window layouts that can be achieved in
 * the MIDAS style Display window. So one MIDASLayout could have
 * multiple MIDASOrientations, but most often will contain either Axial,
 * Coronal or Sagittal. This is different to the MIDASLayout as a layout
 * can contain multiple orientations.
 */
enum WindowLayout
{
  WINDOW_LAYOUT_AXIAL = 0,
  WINDOW_LAYOUT_SAGITTAL = 1,
  WINDOW_LAYOUT_CORONAL = 2,
  WINDOW_LAYOUT_ORTHO = 3,
  WINDOW_LAYOUT_3D = 4,
  WINDOW_LAYOUT_3H = 5,
  WINDOW_LAYOUT_3V = 6,
  WINDOW_LAYOUT_AS_ACQUIRED = 7,
  WINDOW_LAYOUT_UNKNOWN = 8,
  WINDOW_LAYOUT_COR_SAG_H = 9,
  WINDOW_LAYOUT_COR_SAG_V = 10,
  WINDOW_LAYOUT_COR_AX_H = 11,
  WINDOW_LAYOUT_COR_AX_V = 12,
  WINDOW_LAYOUT_SAG_AX_H = 13,
  WINDOW_LAYOUT_SAG_AX_V = 14
};


/*!
 * \brief Returns true if the layout contains only one window, otherwise false.
 */
inline bool IsSingleWindowLayout(WindowLayout layout)
{
  return layout == WINDOW_LAYOUT_AXIAL
      || layout == WINDOW_LAYOUT_SAGITTAL
      || layout == WINDOW_LAYOUT_CORONAL
      || layout == WINDOW_LAYOUT_3D;
}


/*!
 * \brief Returns true if the layout contains multiple windows, otherwise false.
 */
inline bool IsMultiWindowLayout(WindowLayout layout)
{
  return !IsSingleWindowLayout(layout);
}

/*!
 * \brief The number of the possible window layouts.
 */
const int WINDOW_LAYOUT_NUMBER = 15;

/*!
 * \enum MIDASDropType
 * \brief Describes the different modes that can be used when drag and dropping
 * into the MIDAS style Display window.
 */
enum MIDASDropType
{
  MIDAS_DROP_TYPE_SINGLE = 0,   /** This means that multiple nodes are dropped into a single window. */
  MIDAS_DROP_TYPE_MULTIPLE = 1, /** This means that multiple nodes are dropped across multiple windows. */
  MIDAS_DROP_TYPE_ALL = 2       /** This means that multiple nodes are dropped across all windows for a thumnail effect. */
};

/*!
 * \enum MIDASDefaultInterpolationType
 * \brief Describes what the interpolation type should be set to when an image is dropped.
 */
enum MIDASDefaultInterpolationType
{
  MIDAS_INTERPOLATION_NONE,
  MIDAS_INTERPOLATION_LINEAR,
  MIDAS_INTERPOLATION_CUBIC
};

/*!
 * \enum MIDASBindType
 * \brief Describes valid bind modes.
 */
enum MIDASBindType
{
  MIDAS_BIND_NONE = 0,
  MIDAS_BIND_LAYOUT = 1,
  MIDAS_BIND_CURSORS = 2,
  MIDAS_BIND_MAGNIFICATION = 4,
  MIDAS_BIND_GEOMETRY = 8
};

#endif
