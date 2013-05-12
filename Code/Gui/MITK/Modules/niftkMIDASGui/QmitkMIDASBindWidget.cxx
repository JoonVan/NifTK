/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkMIDASBindWidget.h"

#include <mitkLogMacros.h>

//-----------------------------------------------------------------------------
QmitkMIDASBindWidget::QmitkMIDASBindWidget(QWidget *parent)
{
  m_BindType = MIDAS_BIND_NONE;
  setupUi(this);
}


//-----------------------------------------------------------------------------
QmitkMIDASBindWidget::~QmitkMIDASBindWidget()
{
}


//-----------------------------------------------------------------------------
void QmitkMIDASBindWidget::setupUi(QWidget* parent)
{
  Ui_QmitkMIDASBindWidget::setupUi(parent);
  m_BindLayoutCheckBox->setChecked(false);
  m_BindCursorsCheckBox->setChecked(false);
  m_BindMagnificationCheckBox->setChecked(false);
  m_BindGeometryCheckBox->setChecked(false);

  connect(m_BindLayoutCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnLayoutCheckBoxToggled(bool)));
  connect(m_BindCursorsCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnCursorsCheckBoxToggled(bool)));
  connect(m_BindMagnificationCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnMagnificationCheckBoxToggled(bool)));
  connect(m_BindGeometryCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnGeometryCheckBoxToggled(bool)));
}


//-----------------------------------------------------------------------------
bool QmitkMIDASBindWidget::IsLayoutBound() const
{
  return m_BindType & MIDAS_BIND_LAYOUT;
}


//-----------------------------------------------------------------------------
bool QmitkMIDASBindWidget::AreCursorsBound() const
{
  return m_BindType & MIDAS_BIND_CURSORS;
}


//-----------------------------------------------------------------------------
bool QmitkMIDASBindWidget::IsMagnificationBound() const
{
  return m_BindType & MIDAS_BIND_MAGNIFICATION;
}


//-----------------------------------------------------------------------------
bool QmitkMIDASBindWidget::IsGeometryBound() const
{
  return m_BindType & MIDAS_BIND_GEOMETRY;
}


//-----------------------------------------------------------------------------
void QmitkMIDASBindWidget::SetToBindType(int bindType)
{
  if (bindType == m_BindType)
  {
    // Nothing to do.
    return;
  }

  bool wasBlocked = m_BindLayoutCheckBox->blockSignals(true);
  m_BindLayoutCheckBox->setChecked(bindType & MIDAS_BIND_LAYOUT);
  m_BindLayoutCheckBox->blockSignals(wasBlocked);

  wasBlocked = m_BindCursorsCheckBox->blockSignals(true);
  m_BindCursorsCheckBox->setChecked(bindType & MIDAS_BIND_CURSORS);
  m_BindCursorsCheckBox->blockSignals(wasBlocked);

  wasBlocked = m_BindMagnificationCheckBox->blockSignals(true);
  m_BindMagnificationCheckBox->setChecked(bindType & MIDAS_BIND_MAGNIFICATION);
  m_BindMagnificationCheckBox->blockSignals(wasBlocked);

  wasBlocked = m_BindGeometryCheckBox->blockSignals(true);
  m_BindGeometryCheckBox->setChecked(bindType & MIDAS_BIND_GEOMETRY);
  m_BindGeometryCheckBox->blockSignals(wasBlocked);

  m_BindType = bindType;
}


//-----------------------------------------------------------------------------
void QmitkMIDASBindWidget::OnLayoutCheckBoxToggled(bool value)
{
  if (value)
  {
    m_BindType |= MIDAS_BIND_LAYOUT;
  }
  else
  {
    m_BindType &= ~MIDAS_BIND_LAYOUT;
  }
  emit BindTypeChanged();
}


//-----------------------------------------------------------------------------
void QmitkMIDASBindWidget::OnCursorsCheckBoxToggled(bool value)
{
  if (value)
  {
    m_BindType |= MIDAS_BIND_CURSORS;
  }
  else
  {
    m_BindType &= ~MIDAS_BIND_CURSORS;
  }
  emit BindTypeChanged();
}


//-----------------------------------------------------------------------------
void QmitkMIDASBindWidget::OnMagnificationCheckBoxToggled(bool value)
{
  if (value)
  {
    m_BindType |= MIDAS_BIND_MAGNIFICATION;
  }
  else
  {
    m_BindType &= ~MIDAS_BIND_MAGNIFICATION;
  }
  emit BindTypeChanged();
}


//-----------------------------------------------------------------------------
void QmitkMIDASBindWidget::OnGeometryCheckBoxToggled(bool value)
{
  if (value)
  {
    m_BindType |= MIDAS_BIND_GEOMETRY;
  }
  else
  {
    m_BindType &= ~MIDAS_BIND_GEOMETRY;
  }
  emit BindTypeChanged();
}