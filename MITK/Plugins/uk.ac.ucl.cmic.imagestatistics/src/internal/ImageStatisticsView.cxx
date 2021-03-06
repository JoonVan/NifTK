/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryPlatform.h>

// Qmitk
#include "ImageStatisticsView.h"
#include "ImageStatisticsViewPreferencesPage.h"

// Qt
#include <QClipboard>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTableWidgetItem>

// ITK
#include <itkImage.h>
#include <itkImageRegionConstIterator.h>

// MITK
#include <mitkImageAccessByItk.h>

const QString ImageStatisticsView::VIEW_ID = "uk.ac.ucl.cmic.imagestatistics";


//-----------------------------------------------------------------------------
ImageStatisticsView::ImageStatisticsView()
: m_AutoUpdate(false)
, m_RequireSameSizeImage(true)
, m_AssumeBinary(true)
, m_BackgroundValue(0)
, m_MaskNode(NULL)
, m_ImageNode(NULL)
, m_PerSliceStats(false)
, m_Orientation(itk::ORIENTATION_AXIAL)
{
}


//-----------------------------------------------------------------------------
ImageStatisticsView::~ImageStatisticsView()
{
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::SetFocus()
{
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::CreateQtPartControl(QWidget* parent)
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  this->EnableControls(false);

  parent->installEventFilter(this);

  this->InitializeTable();

  // Retrieve up-to-date preference values.
  this->RetrievePreferenceValues();

  // Connect slots, so we are ready for action.
  this->connect(m_Controls.m_PerSliceStatsCheckBox, SIGNAL(toggled(bool)), SLOT(OnPerSliceStatsCheckBoxToggled(bool)));
  this->connect(m_Controls.m_AxialRadioButton, SIGNAL(toggled(bool)), SLOT(OnAxialRadioButtonToggled(bool)));
  this->connect(m_Controls.m_SagittalRadioButton, SIGNAL(toggled(bool)), SLOT(OnSagittalRadioButtonToggled(bool)));
  this->connect(m_Controls.m_CoronalRadioButton, SIGNAL(toggled(bool)), SLOT(OnCoronalRadioButtonToggled(bool)));
  this->connect(m_Controls.m_UpdateButton, SIGNAL(clicked()), SLOT(OnUpdateButtonClicked()));
  this->connect(m_Controls.m_CopyAllButton, SIGNAL(clicked()), SLOT(OnCopyAllButtonClicked()));
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::EnableControls(bool enabled)
{
  m_Controls.m_ImageLabel->setEnabled(enabled);
  m_Controls.m_ImageNameLabel->setEnabled(enabled);
  m_Controls.m_MaskLabel->setEnabled(enabled);
  m_Controls.m_MaskNameLabel->setEnabled(enabled);
  m_Controls.m_TreeWidget->setEnabled(enabled);
  m_Controls.m_PerSliceStatsCheckBox->setEnabled(enabled);
  m_Controls.m_AxialRadioButton->setEnabled(enabled && m_PerSliceStats);
  m_Controls.m_SagittalRadioButton->setEnabled(enabled && m_PerSliceStats);
  m_Controls.m_CoronalRadioButton->setEnabled(enabled && m_PerSliceStats);
  m_Controls.m_UpdateButton->setEnabled(enabled && !m_AutoUpdate);
  m_Controls.m_CopyAllButton->setEnabled(enabled);
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
  // Retrieve up-to-date preference values.
  this->RetrievePreferenceValues();
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::RetrievePreferenceValues()
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  berry::IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node(VIEW_ID);
  assert( prefs );

  m_AutoUpdate = prefs->GetBool(ImageStatisticsViewPreferencesPage::AUTO_UPDATE_NAME, false);
  m_AssumeBinary = prefs->GetBool(ImageStatisticsViewPreferencesPage::ASSUME_BINARY_NAME, true);
  m_RequireSameSizeImage = prefs->GetBool(ImageStatisticsViewPreferencesPage::REQUIRE_SAME_SIZE_IMAGE_NAME, true);
  m_BackgroundValue = prefs->GetInt(ImageStatisticsViewPreferencesPage::BACKGROUND_VALUE_NAME, 0);
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  bool isValid = this->IsSelectionValid(nodes);

  // If the selection is valid, we enable the controls.
  this->EnableControls(isValid);

  // If the selection is value, we update the image labels to indicate we have selected.
  if (isValid)
  {
    m_ImageNode = NULL;
    if (nodes.count() > 0)
    {
      m_ImageNode = nodes[0];
      m_Controls.m_ImageNameLabel->setText(QString::fromStdString(m_ImageNode->GetName()));
      m_Controls.m_ImageNameLabel->setToolTip(QString::fromStdString(m_ImageNode->GetName()));
    }
    else
    {
      m_Controls.m_ImageNameLabel->setText("please select an image");
      m_Controls.m_ImageNameLabel->setToolTip("please select an image");
    }

    m_MaskNode = NULL;
    if (nodes.size() > 1)
    {
      m_MaskNode = nodes[1];
      m_Controls.m_MaskNameLabel->setText(QString::fromStdString(m_MaskNode->GetName()));
      m_Controls.m_MaskNameLabel->setToolTip(QString::fromStdString(m_MaskNode->GetName()));
    }
    else
    {
      m_Controls.m_MaskNameLabel->setText("please select an image");
      m_Controls.m_MaskNameLabel->setToolTip("please select an image");
    }
  }
  else
  {
    m_MaskNode = NULL;
    m_ImageNode = NULL;
  }

  // Optionally (depending on m_AutoUpdate preference), trigger the update.
  // The alternative, is for the user to hit the Update button.
  if (isValid && m_AutoUpdate)
  {
    this->Update(nodes);
  }
}


//-----------------------------------------------------------------------------
bool ImageStatisticsView::IsSelectionValid(const QList<mitk::DataNode::Pointer>& nodes)
{
  bool isValid = true;

  // We must have either 1, or 2 nodes selected.
  if (nodes.count() == 0 || nodes.count() > 2)
  {
    isValid = false;
  }

  // All nodes must be non null images.
  for (mitk::DataNode::Pointer node: nodes)
  {
    if(node.IsNull())
    {
      isValid = false;
    }

    if (node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) == NULL)
    {
      isValid = false;
    }
  }

  // Give up now, if the input is invalid.
  if (!isValid)
  {
    return isValid;
  }

  // If we have 2 images and m_RequireSameSizeImage is true, they must be the same size.
  if (nodes.count() == 2 && m_RequireSameSizeImage)
  {
    mitk::Image::Pointer images[2] = {NULL, NULL};
    for(int i = 0; i < nodes.count(); i++)
    {
      images[i] = dynamic_cast<mitk::Image*>(nodes[i]->GetData());
    }

    if (images[0]->GetDimension() != images[1]->GetDimension())
    {
      isValid = false;
    }
    else
    {
      for (unsigned int i = 0; i < images[0]->GetDimension(); i++)
      {
        if (images[0]->GetDimension(i) != images[1]->GetDimension(i))
        {
          isValid = false;
          break;
        }
      }
    }
  } // end if nodes.count()
  return isValid;
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::OnUpdateButtonClicked()
{
  // This does not work, as when you select the Update button, you lose the selection in the DataManager as it loses focus.
  // const QList<mitk::DataNode::Pointer>& nodes = this->GetDataManagerSelection();

  // So create a list.
  QList<mitk::DataNode::Pointer> nodes;

  if (m_ImageNode.IsNotNull())
  {
    nodes.push_back(m_ImageNode);
  }
  if (m_MaskNode.IsNotNull())
  {
    nodes.push_back(m_MaskNode);
  }

  // Check nodes.
  bool isValid = this->IsSelectionValid(nodes);
  if (isValid)
  {
    this->Update(nodes);
  }
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::Update(const QList<mitk::DataNode::Pointer>& nodes)
{
  // We are assuming nodes is valid input, and not checking it any further.
  try
  {
    mitk::DataNode::Pointer imageNode = NULL;
    if (nodes.count() > 0)
    {
      imageNode = nodes[0];
    }
    mitk::DataNode::Pointer maskNode = NULL;
    if (nodes.size() > 1)
    {
      maskNode = nodes[1];
    }

    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(imageNode->GetData());
    mitk::Image::Pointer mask = NULL;

    if (maskNode.IsNotNull())
    {
      mask = dynamic_cast<mitk::Image*>(maskNode->GetData());
    }

    if (image.IsNotNull() && mask.IsNull())
    {
      int dimensions = image->GetDimension();
      switch(dimensions)
      {
      case 2:
        AccessFixedDimensionByItk(image, UpdateTable, 2);
        break;
      case 3:
        AccessFixedDimensionByItk(image, UpdateTable, 3);
        break;
      case 4:
        AccessFixedDimensionByItk(image, UpdateTable, 4);
        break;
      default:
        MITK_ERROR << "During ImageStatisticsView::UpdateTable, unsupported number of dimensions:" << dimensions << std::endl;
      }
    }
    else if (image.IsNotNull() && mask.IsNotNull())
    {
      int dimensions = image->GetDimension();
      switch(dimensions)
      {
      case 2:
        AccessTwoImagesFixedDimensionByItk(image, mask, UpdateTableWithMask, 2);
        break;
      case 3:
        AccessTwoImagesFixedDimensionByItk(image, mask, UpdateTableWithMask, 3);
        break;
      case 4:
        AccessTwoImagesFixedDimensionByItk(image, mask, UpdateTableWithMask, 4);
        break;
      default:
        MITK_ERROR << "During ImageStatisticsView::UpdateTableWithMask, unsupported number of dimensions:" << dimensions << std::endl;
      }
    }
  }
  catch(const mitk::AccessByItkException& e)
  {
    MITK_ERROR << "During ImageStatisticsView::Update, caught mitk::AccessByItkException caused by:" << e.what() << std::endl;
  }
  catch(itk::ExceptionObject& err)
  {
    MITK_ERROR << "During ImageStatisticsView::Update, caught itk::ExceptionObject caused by:" << err.what() << std::endl;
  }
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::InitializeTable()
{
  m_Controls.m_TreeWidget->clear();
  int columnCount = 10;
  if (m_PerSliceStats)
  {
    ++columnCount;
  }
  m_Controls.m_TreeWidget->setColumnCount(columnCount);

  // The order of these columns must match the order in CreateTableRow.
  QStringList headers;
  if (m_PerSliceStats)
  {
    QString orientationName;
    if (m_Orientation == itk::ORIENTATION_AXIAL)
    {
      orientationName = "slice (I->S)";
    }
    else if (m_Orientation == itk::ORIENTATION_SAGITTAL)
    {
      orientationName = "slice (L->R)";
    }
    else if (m_Orientation == itk::ORIENTATION_CORONAL)
    {
      orientationName = "slice (P->A)";
    }
    headers << orientationName;
  }
  headers << "value";
  headers << "volume (ml)";
  headers << "mean";
  headers << "mean 60%";
  headers << "mean 70%";
  headers << "median";
  headers << "std dev";
  headers << "min";
  headers << "max";
  headers << "count";

  m_Controls.m_TreeWidget->setHeaderLabels(headers);
}


//-----------------------------------------------------------------------------
template <typename PixelType>
QTreeWidgetItem*
ImageStatisticsView
::CreateTableRow(QTreeWidgetItem* parentItem,
    const QString& value, PixelType min, PixelType max, double mean, double median,
    double stdDev, unsigned long count, double volume, int sliceIndex)
{
  QStringList values;
  if (m_PerSliceStats)
  {
    values.append(QString("%1").arg(sliceIndex));
  }
  values.append(QString("%1").arg(value));
  values.append(QString("%1").arg(volume / 1000.0));
  values.append(QString("%1").arg(mean));
  values.append(QString("%1").arg(mean * 0.6));
  values.append(QString("%1").arg(mean * 0.7));
  values.append(QString("%1").arg(median));
  values.append(QString("%1").arg(stdDev));
  values.append(QString("%1").arg(min));
  values.append(QString("%1").arg(max));
  values.append(QString("%1").arg(count));

  return new QTreeWidgetItem(parentItem, values);
}


//-----------------------------------------------------------------------------
template <typename PixelType, unsigned int VImageDimension>
void
ImageStatisticsView
::GetLabelValues(
    itk::Image<PixelType, VImageDimension>* itkImage,
    std::set<PixelType>& labels)
{
  labels.clear();

  itk::ImageRegionConstIterator< itk::Image<PixelType, VImageDimension> >
    iterator(itkImage, itkImage->GetLargestPossibleRegion());

  for (iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
  {
    labels.insert(iterator.Get());
  }
}


//-----------------------------------------------------------------------------
template <typename PixelType, unsigned int VImageDimension>
void
ImageStatisticsView
::GetVoxelVolume(
    itk::Image<PixelType, VImageDimension>* itkImage,
    double& volume
    )
{

  typedef itk::Image<PixelType, VImageDimension> ImageType;
  typedef typename ImageType::SpacingType SpacingType;

  SpacingType spacing = itkImage->GetSpacing();

  volume = 1;
  for (unsigned int i = 0; i < SpacingType::GetVectorDimension(); i++)
  {
    volume *= spacing[i];
  }
}


//-----------------------------------------------------------------------------
template <typename TPixel>
void
ImageStatisticsView
::TestMinAndMax(
    TPixel imageValue,
    TPixel& min,
    TPixel& max
    )
{
  if (imageValue < min)
  {
    min = imageValue;
  }
  if (imageValue > max)
  {
    max = imageValue;
  }
}


//-----------------------------------------------------------------------------
template <typename TPixel>
void
ImageStatisticsView
::AccumulateData(
    TPixel imageValue,
    double& mean,
    double& s0,
    double& s1,
    double& s2,
    unsigned long& counter
    )
{
  mean += imageValue;
  s0 += 1;
  s1 += imageValue;
  s2 += imageValue * imageValue;
  counter++;
}


//-----------------------------------------------------------------------------
template <typename TPixel>
void
ImageStatisticsView
::AccumulateValue(
    TPixel imageValue,
    TPixel& min,
    TPixel& max,
    double& mean,
    double& s0,
    double& s1,
    double& s2,
    unsigned long& counter,
    TPixel* imagePixelsCopy
    )
{
  if (imageValue != (TPixel)m_BackgroundValue)
  {
    this->TestMinAndMax<TPixel>(imageValue, min, max);
    imagePixelsCopy[counter] = imageValue;
    this->AccumulateData<TPixel>(imageValue, mean, s0, s1, s2, counter);
  }
}


//-----------------------------------------------------------------------------
template <typename TPixel1, typename TPixel2>
void
ImageStatisticsView
::AccumulateValue(
    bool invert,
    TPixel2 valueToCompareMaskAgainst,
    TPixel1 imageValue,
    TPixel2 maskValue,
    TPixel1& min,
    TPixel1& max,
    double& mean,
    double& s0,
    double& s1,
    double& s2,
    unsigned long& counter,
    TPixel1* imagePixelsCopy
    )
{
  if (   (!invert && maskValue == (TPixel2)valueToCompareMaskAgainst)
      || (invert &&  maskValue != (TPixel2)valueToCompareMaskAgainst)
      )
  {
    this->TestMinAndMax<TPixel1>(imageValue, min, max);
    imagePixelsCopy[counter] = imageValue;
    this->AccumulateData<TPixel1>(imageValue, mean, s0, s1, s2, counter);
  }
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::CalculateMeanAndStdDev(
    double& mean,
    double s0,
    double s1,
    double s2,
    double& stdDev,
    unsigned long counter
    )
{
  if (counter > 0)
  {
    mean /= (double)counter;
    stdDev = sqrt( (double)((s0*s2 - s1*s1) / (s0*(s0 - 1))) );
  }
  else
  {
    mean = 0;
    stdDev = 0;
  }
}


//-----------------------------------------------------------------------------
template <typename TPixel, unsigned int VImageDimension>
void
ImageStatisticsView
::UpdateTable(
    itk::Image<TPixel, VImageDimension>* itkImage
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> GreyImage;
  double mean, s0, s1, s2, stdDev, median;
  unsigned long int counter;
  TPixel min, max;

  // Initialize table.
  this->InitializeTable();

  // Get voxel volume.
  double voxelVolume;
  this->GetVoxelVolume<TPixel, VImageDimension>(itkImage, voxelVolume);

  if (!m_PerSliceStats)
  {
    typename GreyImage::RegionType region = itkImage->GetLargestPossibleRegion();
    TPixel* imagePixelsCopy = new TPixel[region.GetNumberOfPixels()];

    // Calculate Stats.
    this->CalculateStats(itkImage, region, min, max, mean, s0, s1, s2, stdDev, counter, imagePixelsCopy, median);

    double volume = voxelVolume * counter;

    QString value = tr("All except %1").arg(m_BackgroundValue);
    QTreeWidgetItem* item = this->CreateTableRow(0, value, min, max, mean, median, stdDev, counter, volume);
    m_Controls.m_TreeWidget->addTopLevelItem(item);

    delete[] imagePixelsCopy;
  }
  else
  {
    if (VImageDimension == 3)
    {
      typedef typename itk::Image<TPixel, 3> GreyImage3D;
      GreyImage3D* itkImage3D = reinterpret_cast<GreyImage3D*>(itkImage);

      typename GreyImage3D::RegionType region = itkImage3D->GetLargestPossibleRegion();

      int axis;
      itk::GetAxisFromITKImage(itkImage3D, m_Orientation, axis);
      int upDirection;
      itk::GetUpDirectionFromITKImage(itkImage3D, m_Orientation, upDirection);

      int sliceNumber = region.GetSize(axis);
      int startSlice = upDirection > 0 ? 0 : sliceNumber - 1;
      int endSlice = upDirection > 0 ? sliceNumber : -1;

      region.SetSize(axis, 1);

      TPixel* imagePixelsCopy = new TPixel[region.GetNumberOfPixels()];

      for (int sliceIndex = startSlice; sliceIndex != endSlice; sliceIndex += upDirection)
      {
        region.SetIndex(axis, sliceIndex);

        this->CalculateStats(itkImage3D, region, min, max, mean, s0, s1, s2, stdDev, counter, imagePixelsCopy, median);

        double volume = voxelVolume * counter;

        QString value = tr("All except %1").arg(m_BackgroundValue);
        QTreeWidgetItem* item = this->CreateTableRow(0, value, min, max, mean, median, stdDev, counter, volume, sliceIndex);
        m_Controls.m_TreeWidget->addTopLevelItem(item);
      }

      delete[] imagePixelsCopy;
    }
  }
}


//-----------------------------------------------------------------------------
template <typename TPixel, unsigned VImageDimension>
void
ImageStatisticsView
::CalculateStats(
    itk::Image<TPixel, VImageDimension>* itkImage,
    const itk::ImageRegion<VImageDimension>& region,
    TPixel& min,
    TPixel& max,
    double& mean,
    double& s0,
    double& s1,
    double& s2,
    double& stdDev,
    unsigned long& counter,
    TPixel* imagePixelsCopy,
    double& median
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> GreyImageType;

  min = std::numeric_limits<TPixel>::max();
  max = std::numeric_limits<TPixel>::min();
  mean = 0.0;
  s0 = 0.0;
  s1 = 0.0;
  s2 = 0.0;
  stdDev = 0.0;
  counter = 0;
  median = 0.0;

  // Iterate through image, calculating stats for anything != background value.
  itk::ImageRegionConstIterator<GreyImageType> iter(itkImage, region);
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    TPixel greyPixel = iter.Get();
    this->AccumulateValue<TPixel>(greyPixel, min, max, mean, s0, s1, s2, counter, imagePixelsCopy);
  }
  this->CalculateMeanAndStdDev(mean, s0, s1, s2, stdDev, counter);

  /// Median is put in the middle of the copy array.
  std::nth_element(imagePixelsCopy, imagePixelsCopy + counter / 2, imagePixelsCopy + counter);
  median = imagePixelsCopy[counter / 2];
}


//-----------------------------------------------------------------------------
template <typename TPixel1, unsigned VImageDimension1, typename TPixel2, unsigned VImageDimension2>
void
ImageStatisticsView
::UpdateTableWithMask(
    itk::Image<TPixel1, VImageDimension1>* itkImage,
    itk::Image<TPixel2, VImageDimension2>* itkMask
    )
{
  typedef typename itk::Image<TPixel1, VImageDimension1> GreyImage;
  typedef typename itk::Image<TPixel2, VImageDimension2> MaskImage;
  double mean, s0, s1, s2, stdDev, median;
  unsigned long int counter;
  TPixel1 min, max;

  // Get a list of values in itkMask.
  std::set<TPixel2> labels;
  if (!m_AssumeBinary)
  {
    this->GetLabelValues<TPixel2, VImageDimension2>(itkMask, labels);
  }

  // Initialize table.
  this->InitializeTable();

  // Get voxel volume.
  double voxelVolume;
  this->GetVoxelVolume<TPixel1, VImageDimension1>(itkImage, voxelVolume);

  if (m_AssumeBinary)
  {
    TPixel2 labelValue = m_BackgroundValue;

    if (!m_PerSliceStats)
    {
      typename GreyImage::RegionType region = itkImage->GetLargestPossibleRegion();
      TPixel1* imagePixelsCopy = new TPixel1[region.GetNumberOfPixels()];

      // Calculate Stats.
      this->CalculateStatsWithMask(itkImage, itkMask, region, true, labelValue, min, max, mean, s0, s1, s2, stdDev, counter, imagePixelsCopy, median);

      double volume = voxelVolume * counter;

      QString value = tr("All except %1").arg(m_BackgroundValue);
      QTreeWidgetItem* item = this->CreateTableRow(0, value, min, max, mean, median, stdDev, counter, volume);
      m_Controls.m_TreeWidget->addTopLevelItem(item);

      delete[] imagePixelsCopy;
    }
    else
    {
      if (VImageDimension1 == 3)
      {
        typedef typename itk::Image<TPixel1, 3> GreyImage3D;
        typedef typename itk::Image<TPixel2, 3> MaskImage3D;
        GreyImage3D* itkImage3D = reinterpret_cast<GreyImage3D*>(itkImage);
        MaskImage3D* itkMask3D = reinterpret_cast<MaskImage3D*>(itkMask);

        typename GreyImage3D::RegionType region = itkImage3D->GetLargestPossibleRegion();

        int axis;
        itk::GetAxisFromITKImage(itkImage3D, m_Orientation, axis);
        int upDirection;
        itk::GetUpDirectionFromITKImage(itkImage3D, m_Orientation, upDirection);

        int sliceNumber = region.GetSize(axis);
        int startSlice = upDirection > 0 ? 0 : sliceNumber - 1;
        int endSlice = upDirection > 0 ? sliceNumber : -1;

        region.SetSize(axis, 1);

        TPixel1* imagePixelsCopy = new TPixel1[region.GetNumberOfPixels()];

        for (int sliceIndex = startSlice; sliceIndex != endSlice; sliceIndex += upDirection)
        {
          region.SetIndex(axis, sliceIndex);

          this->CalculateStatsWithMask(itkImage3D, itkMask3D, region, true, labelValue, min, max, mean, s0, s1, s2, stdDev, counter, imagePixelsCopy, median);

          double volume = voxelVolume * counter;

          QString value = tr("All except %1").arg(m_BackgroundValue);
          QTreeWidgetItem* item = this->CreateTableRow(0, value, min, max, mean, median, stdDev, counter, volume, sliceIndex);
          m_Controls.m_TreeWidget->addTopLevelItem(item);
        }

        delete[] imagePixelsCopy;
      }
    }
  }
  else
  {
    if (!m_PerSliceStats)
    {
      typename GreyImage::RegionType region = itkImage->GetLargestPossibleRegion();
      TPixel1* imagePixelsCopy = new TPixel1[region.GetNumberOfPixels()];

      // We compute stats for EACH label.
      // This is a bit slow, as we repeatedly iterate over the image.

      typename std::set<TPixel2>::iterator itLabels;
      for (itLabels = labels.begin(); itLabels != labels.end(); itLabels++)
      {
        TPixel2 labelValue = *itLabels;

        // Calculate Stats.
        this->CalculateStatsWithMask(itkImage, itkMask, region, false, labelValue, min, max, mean, s0, s1, s2, stdDev, counter, imagePixelsCopy, median);

        double volume = voxelVolume * counter;

        QString value = tr("%1").arg(labelValue);
        QTreeWidgetItem* item = this->CreateTableRow(0, value, min, max, mean, median, stdDev, counter, volume);
        m_Controls.m_TreeWidget->addTopLevelItem(item);
      }

      delete[] imagePixelsCopy;
    }
    else
    {
      if (VImageDimension1 == 3)
      {
        typedef typename itk::Image<TPixel1, 3> GreyImage3D;
        typedef typename itk::Image<TPixel2, 3> MaskImage3D;
        GreyImage3D* itkImage3D = reinterpret_cast<GreyImage3D*>(itkImage);
        MaskImage3D* itkMask3D = reinterpret_cast<MaskImage3D*>(itkMask);

        typename GreyImage3D::RegionType region = itkImage3D->GetLargestPossibleRegion();

        int axis;
        itk::GetAxisFromITKImage(itkImage3D, m_Orientation, axis);
        int upDirection;
        itk::GetUpDirectionFromITKImage(itkImage3D, m_Orientation, upDirection);

        int sliceNumber = region.GetSize(axis);
        int startSlice = upDirection > 0 ? 0 : sliceNumber - 1;
        int endSlice = upDirection > 0 ? sliceNumber : -1;

        region.SetSize(axis, 1);

        TPixel1* imagePixelsCopy = new TPixel1[region.GetNumberOfPixels()];

        for (int sliceIndex = startSlice; sliceIndex != endSlice; sliceIndex += upDirection)
        {
          region.SetIndex(axis, sliceIndex);

          // We compute stats for EACH label.
          // This is a bit slow, as we repeatedly iterate over the image.

          typename std::set<TPixel2>::iterator itLabels;
          for (itLabels = labels.begin(); itLabels != labels.end(); itLabels++)
          {
            TPixel2 labelValue = *itLabels;

            // Calculate Stats.
            this->CalculateStatsWithMask(itkImage3D, itkMask3D, region, true, labelValue, min, max, mean, s0, s1, s2, stdDev, counter, imagePixelsCopy, median);

            double volume = voxelVolume * counter;

            QString value = tr("%1").arg(labelValue);
            QTreeWidgetItem* item = this->CreateTableRow(0, value, min, max, mean, median, stdDev, counter, volume, sliceIndex);
            m_Controls.m_TreeWidget->addTopLevelItem(item);
          }
        }

        delete[] imagePixelsCopy;
      }
    }
  }
}


//-----------------------------------------------------------------------------
template <typename TPixel1, typename TPixel2, unsigned int VImageDimension>
void
ImageStatisticsView
::CalculateStatsWithMask(
    itk::Image<TPixel1, VImageDimension>* itkImage,
    itk::Image<TPixel2, VImageDimension>* itkMask,
    const itk::ImageRegion<VImageDimension>& region,
    bool invert,
    TPixel2 label,
    TPixel1& min,
    TPixel1& max,
    double& mean,
    double& s0,
    double& s1,
    double& s2,
    double& stdDev,
    unsigned long& counter,
    TPixel1* imagePixelsCopy,
    double& median
    )
{
  typedef typename itk::Image<TPixel1, VImageDimension> GreyImage;
  typedef typename itk::Image<TPixel2, VImageDimension> MaskImage;

  // Initialize variables
  min = std::numeric_limits<TPixel1>::max();
  max = std::numeric_limits<TPixel1>::min();
  mean = 0.0;
  s0 = 0.0;
  s1 = 0.0;
  s2 = 0.0;
  stdDev = 0.0;
  counter = 0;
  median = 0.0;

  // We iterate over the image, calculating stats for any voxel where the mask value is NOT the background value.
  // i.e. we are using the mask image, but if the mask has multiple labels, we treat any label except the background label as foreground, and accumulate stats.

  itk::ImageRegionConstIterator<GreyImage> greyIter(itkImage, region);
  itk::ImageRegionConstIterator<MaskImage> binaryIter(itkMask, region);
  for (greyIter.GoToBegin(), binaryIter.GoToBegin(); !greyIter.IsAtEnd() && !binaryIter.IsAtEnd(); ++greyIter, ++binaryIter)
  {
    TPixel1 greyPixel = greyIter.Get();
    TPixel2 maskPixel = binaryIter.Get();

    this->AccumulateValue<TPixel1, TPixel2>
      (invert, label, greyPixel, maskPixel, min, max, mean, s0, s1, s2, counter, imagePixelsCopy);
  }
  this->CalculateMeanAndStdDev(mean, s0, s1, s2, stdDev, counter);

  std::nth_element(imagePixelsCopy, imagePixelsCopy + counter / 2, imagePixelsCopy + counter);
  median = imagePixelsCopy[counter / 2];
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::OnPerSliceStatsCheckBoxToggled(bool toggled)
{
  m_PerSliceStats = toggled;
  m_Controls.m_AxialRadioButton->setEnabled(m_PerSliceStats);
  m_Controls.m_SagittalRadioButton->setEnabled(m_PerSliceStats);
  m_Controls.m_CoronalRadioButton->setEnabled(m_PerSliceStats);

  this->InitializeTable();
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::OnAxialRadioButtonToggled(bool toggled)
{
  if (toggled)
  {
    m_Orientation = itk::ORIENTATION_AXIAL;

    this->InitializeTable();
  }
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::OnSagittalRadioButtonToggled(bool toggled)
{
  if (toggled)
  {
    m_Orientation = itk::ORIENTATION_SAGITTAL;

    this->InitializeTable();
  }
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::OnCoronalRadioButtonToggled(bool toggled)
{
  if (toggled)
  {
    m_Orientation = itk::ORIENTATION_CORONAL;

    this->InitializeTable();
  }
}


//-----------------------------------------------------------------------------
bool ImageStatisticsView::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::KeyPress)
  {
    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
    if (keyEvent->matches(QKeySequence::SelectAll))
    {
      m_Controls.m_TreeWidget->selectAll();
      return true;
    }
    else if (keyEvent->matches(QKeySequence::Copy))
    {
      this->Copy();
      return true;
    }
  }

  // pass the event on to the parent class
  return Superclass::eventFilter(object, event);
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::OnCopyAllButtonClicked()
{
  m_Controls.m_TreeWidget->selectAll();
  this->Copy();
}


//-----------------------------------------------------------------------------
void ImageStatisticsView::Copy()
{
  QAbstractItemModel* model = m_Controls.m_TreeWidget->model();
  QItemSelectionModel* selection = m_Controls.m_TreeWidget->selectionModel();
  QModelIndexList indexes = selection->selectedRows();

  int columnCount = model->columnCount();

  QString selectedText;

  QVariant data = model->headerData(0, Qt::Horizontal);
  selectedText.append(data.toString());

  for (int column = 1; column < columnCount; ++column)
  {
    selectedText.append(QLatin1Char('\t'));
    data = model->headerData(column, Qt::Horizontal);
    selectedText.append(data.toString());
  }

  selectedText.append(QLatin1Char('\n'));

  for (QModelIndex current: indexes)
  {
    int row = current.row();

    data = model->data(current);
    selectedText.append(data.toString());

    for (int column = 1; column < columnCount; ++column)
    {
      selectedText.append(QLatin1Char('\t'));
      data = model->data(model->index(row, column));
      selectedText.append(data.toString());
    }

    selectedText.append(QLatin1Char('\n'));
  }

  qApp->clipboard()->setText(selectedText);
}
