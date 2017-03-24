/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <cstdlib>
#include <limits>
#include <mitkVector.h>
#include <QApplication>

#include <QmitkUltrasoundPinCalibrationWidget.h>
#include <niftkCalibrationPointClickerCLP.h>

int main(int argc, char** argv)
{
  PARSE_ARGS;
  int returnStatus = EXIT_FAILURE;

  if (inputImageDirectory.length() == 0
  ||  outputPointDirectory.length() == 0
  )
  {
    commandLine.getOutput()->usage(commandLine);
    return returnStatus;
  }

  try
  {

    QApplication app(argc,argv);
    
    QmitkUltrasoundPinCalibrationWidget cw(
      QString::fromStdString(inputImageDirectory),
      QString::fromStdString(outputPointDirectory),
      outputTimeStamp,
      outputZ
    );
    cw.show();
    
    returnStatus = app.exec();
  }
  catch (mitk::Exception& e)
  {
    MITK_ERROR << "Caught mitk::Exception: " << e.GetDescription() << ", from:" << e.GetFile() << "::" << e.GetLine() << std::endl;
    returnStatus = EXIT_FAILURE + 1;
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Caught std::exception: " << e.what() << std::endl;
    returnStatus = EXIT_FAILURE + 2;
  }
  catch (...)
  {
    MITK_ERROR << "Caught unknown exception:" << std::endl;
    returnStatus = EXIT_FAILURE + 3;
  }

  return returnStatus;
}
