/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkIGINVidiaDataSourceGui.h"
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QGridLayout>
#include <QGLWidget>
#include <QmitkIGIDataSourceMacro.h>
#include "QmitkIGINVidiaDataSource.h"
#include "QmitkVideoPreviewWidget.h"

NIFTK_IGISOURCE_GUI_MACRO(NIFTKNVIDIAGUI_EXPORT, QmitkIGINVidiaDataSourceGui, "IGI NVidia Video Gui")

//-----------------------------------------------------------------------------
QmitkIGINVidiaDataSourceGui::QmitkIGINVidiaDataSourceGui()
  : m_OglWin(0), m_PreviousBaseResolution(0)
{
  // To do.
}


//-----------------------------------------------------------------------------
QmitkIGINVidiaDataSourceGui::~QmitkIGINVidiaDataSourceGui()
{
  // gui is destroyed before data source (by igi data manager)
  // so disconnect ourselfs from source
  QmitkIGINVidiaDataSource* source = GetQmitkIGINVidiaDataSource();
  if (source)
  {
    // this is receiver
    // and source is sender
    this->disconnect(source);
  }

  // FIXME: not sure how to properly cleanup qt
  
  delete m_OglWin;
}


//-----------------------------------------------------------------------------
QmitkIGINVidiaDataSource* QmitkIGINVidiaDataSourceGui::GetQmitkIGINVidiaDataSource() const
{
  QmitkIGINVidiaDataSource* result = NULL;

  if (this->GetSource() != NULL)
  {
    result = dynamic_cast<QmitkIGINVidiaDataSource*>(this->GetSource());
  }

  return result;
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSourceGui::Initialize(QWidget *parent)
{
  setupUi(this);

  QmitkIGINVidiaDataSource *source = this->GetQmitkIGINVidiaDataSource();
  if (source != NULL)
  {
    if (m_OglWin == 0)
    {
      QGLWidget* capturecontext = source->GetCaptureContext();
      if (capturecontext != 0)
      {
        // one preview window for all channels
        m_OglWin = new QmitkVideoPreviewWidget(PreviewGroupBox, capturecontext);
        PreviewGroupBox->layout()->addWidget(m_OglWin);
        m_OglWin->show();

        // connect gui controls only if everything else is fine
        connect(FieldModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFieldModeChange(int)));

        // explicitly update at least once
        Update();
      }
    }
  }
  else
  {
    MITK_ERROR << "QmitkIGINVidiaDataSourceGui: source is NULL, which suggests a programming bug" << std::endl;
  }

}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSourceGui::OnFieldModeChange(int index)
{
  QmitkIGINVidiaDataSource *source = this->GetQmitkIGINVidiaDataSource();
  if (source != NULL)
  {
    // only allow changing field mode during live capture.
    // for playback the combobox is disabled anyway and allowing this handler to
    // proceed will mess up the internal state of the data source.
    if (!source->GetIsPlayingBack())
    {
      // FIXME: if we are recording do not allow changing it!
      //        see https://cmicdev.cs.ucl.ac.uk/trac/ticket/2559

      // if we dont stop then preview-widget will reference a deleted texture
      StopPreviewWidget();

      bool    wascapturing = source->IsCapturing();
      if (wascapturing)
      {
        source->StopCapturing();
      }

      QmitkIGINVidiaDataSource::InterlacedBehaviour   ib(QmitkIGINVidiaDataSource::DO_NOTHING_SPECIAL);
      switch (index)
      {
        case 0:   ib = QmitkIGINVidiaDataSource::DO_NOTHING_SPECIAL;             break;
        case 1:   ib = QmitkIGINVidiaDataSource::DROP_ONE_FIELD;                 break;
        case 2:   ib = QmitkIGINVidiaDataSource::SPLIT_LINE_INTERLEAVED_STEREO;  break;
        default:
          assert(false);
      }

      source->SetFieldMode(ib);

      if (wascapturing)
      {
        source->StartCapturing();
      }
    }
  }
  else
  {
    MITK_ERROR << "QmitkIGINVidiaDataSourceGui: source is NULL, which suggests a programming bug" << std::endl;
  }
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSourceGui::StopPreviewWidget()
{
  assert(PreviewGroupBox->layout() != 0);
  for (int i = 0; i < PreviewGroupBox->layout()->count(); ++i)
  {
    QLayoutItem* l = PreviewGroupBox->layout()->itemAt(i);
    QWidget*     w = l->widget();
    if (w)
    {
      QmitkVideoPreviewWidget*   g = dynamic_cast<QmitkVideoPreviewWidget*>(w);
      if (g)
      {
        g->SetTextureId(0);
      }
    }
  }
}


//-----------------------------------------------------------------------------
void QmitkIGINVidiaDataSourceGui::Update()
{
  QmitkIGINVidiaDataSource *source = this->GetQmitkIGINVidiaDataSource();
  if (source != NULL)
  {
    int streamcount = source->GetNumberOfStreams();
    std::ostringstream    sc;
    sc << streamcount;
    QString   ss = QString::fromStdString(sc.str());
    // only change text if it's actually different
    // otherwise the window is resetting a selection all the time: annoying as hell
    if (StreamCountTextBox->text().compare(ss) != 0)
    {
      StreamCountTextBox->setText(ss);
    }

    QString wireformat(source->GetWireFormatString());
    if (FormatIDTextBox->text().compare(wireformat) != 0)
    {
      FormatIDTextBox->setText(wireformat);
    }

    int   fieldmodecomboboxindex = FieldModeComboBox->currentIndex();
    int   fieldmodeshouldbeindex = 0;
    switch (source->GetFieldMode())
    {
      case QmitkIGINVidiaDataSource::DO_NOTHING_SPECIAL:              fieldmodeshouldbeindex = 0;  break;
      case QmitkIGINVidiaDataSource::DROP_ONE_FIELD:                  fieldmodeshouldbeindex = 1;  break;
      case QmitkIGINVidiaDataSource::SPLIT_LINE_INTERLEAVED_STEREO:   fieldmodeshouldbeindex = 2;  break;
      default:
        assert(false);
    }

    if (fieldmodecomboboxindex != (int) fieldmodeshouldbeindex)
    {
      FieldModeComboBox->setCurrentIndex((int) fieldmodeshouldbeindex);
    }
    FieldModeComboBox->setEnabled(!source->GetIsPlayingBack());

    if (streamcount > 0)
    {
      int width = source->GetCaptureWidth();
      int height = source->GetCaptureHeight();
      float rr = source->GetRefreshRate();

      std::ostringstream    sf;
      sf << width << " x " << height << " @ " << rr << " Hz";

      ss = QString::fromStdString(sf.str());
      // only change text if it's actually different
      // otherwise the window is resetting a selection all the time: annoying as hell
      if (SignalTextBox->text().compare(ss) != 0)
      {
        SignalTextBox->setText(ss);
      }

      // there should be only one, really
      assert(PreviewGroupBox->layout() != 0);
      for (int i = 0; i < PreviewGroupBox->layout()->count(); ++i)
      {
        QLayoutItem* l = PreviewGroupBox->layout()->itemAt(i);
        QWidget*     w = l->widget();
        if (w)
        {
          QmitkVideoPreviewWidget*   g = dynamic_cast<QmitkVideoPreviewWidget*>(w);
          if (g)
          {
            // disable preview widget for now.
            // see https://cmicdev.cs.ucl.ac.uk/trac/ticket/2745
            // see https://cmicdev.cs.ucl.ac.uk/trac/ticket/2383
            g->SetTextureId(0);
            //g->SetVideoDimensions(width, height);
            //g->SetTextureId(source->GetTextureId(0));
            //g->updateGL();
            // one preview widget for all input streams
            break;
          }
        }
      }
    } // if streamcount
    else
    {
      SignalTextBox->setText("None");
    }
  }
}
