Welcome
-------

This is the @NIFTK_PLATFORM@ Translational Software Platform version @NIFTK_VERSION_STRING@.

For all questions and queries, please join the users mailing list: @NIFTK_USER_CONTACT@.

Documentation
-------------

The documentation is provided via

  an online @NIFTK_WIKI_TEXT@: @NIFTK_WIKI_URL@
  nightly generated documentation: @NIFTK_NIGHTLY_DOCS@

and each graphical user interface contains its own help files.

Notices
-------

We have included the Slicer Execution Model. This is downloaded at compile
time, in order to build command line applications. In addition the file
NifTK/CMake/niftkMacroBuildCLI.cmake contains a modified version of 
SEMMacroBuildCLI.cmake.

All or portions of this licensed product (such portions are the "Software") 
have been obtained under license from The Brigham and Women's Hospital, Inc. 
and are subject to the following terms and conditions:
  http://www.slicer.org/copyright/copyright.txt

Additional Libraries
--------------------

This platform uses these libraries/projects listed below. 
Please check the NifTK/Doc/Licenses directory for licenses.
These licenses should also be installed with the packaged binaries.
The exact number of packages installed depends on configure 
parameters at compile time.
     
Library : Boost
Website : http://www.boost.org
Purpose : Generic library of high quality C++ code. 
License : Doc/Licenses/Boost.txt (Boost Software License)

Library : VTK
Website : http://www.vtk.org
Purpose : Graphics and visualization.
License : Doc/Licenses/VTK.txt (BSD 3 clause license)

Library : GDCM
Website : http://www.creatis.insa-lyon.fr/software/public/Gdcm
Purpose : DICOM file loading library.
License : Doc/Licenses/GDCM.txt (BSD 3 clause license)

Library : DCMTK
Website : http://dicom.offis.de
Purpose : DICOM file loading and networking.
License : Doc/Licenses/DCMTK.txt (mainly BSD 3 clause, but a mixture, please read this)

Library : ITK
Website : http://www.itk.org
Purpose : Image processing algorithms.
License : Doc/Licenses/ITK.txt (Apache v2 license)
Mods    : 4.3.2.1: Removed path length check in top level CMakeLists.txt
          4.3.2.2: Raised https://issues.itk.org/jira/browse/ITK-3206
                   Applied patch directly to our 4.3.2.1 version.

Library : OpenCV
Website : http://opencv.willowgarage.com/wiki
Purpose : Computer Vision.
License : Doc/Licences/OpenCV.txt (BSD 3 clause license)

Library : Aruco
Website : http://www.uco.es/investiga/grupos/ava/node/26
Purpose : AR marker tracking.
License : Doc/Licenses/Aruco.txt (BSD 2 clause license)

Library : EIGEN
Website : http://eigen.tuxfamily.org
Purpose : Matrix library used by AprilTags, PCL etc.
License : http://www.mozilla.org/MPL/2.0/ (Mozilla Public License v2.0)

Library : AprilTags
Website : http://people.csail.mit.edu/kaess/apriltags
Purpose : AR marker tracking.
License : Doc/Licenses/AprilTags.txt (LGPL v2.1 license)

Library : FLANN
Website : http://www.cs.ubc.ca/research/flann
Purpose : Needed by PCL, fast, approx nearest neighbour algorithm.
License : Doc/Licenses/FLANN.txt (BSD 3 clause license)

Library : PCL
Website : http://pointclouds.org 
Purpose : Point cloud processing.
License : Doc/Licenses/PCL.txt (BSD 3 clause license)

Library : MITK
Website : http://www.mitk.org
Purpose : Application framework, common medical imaging specific classes.
License : Doc/Licenses/MITK.txt (Modified BSD 3 clause license)
          Specifically, modifications must be indicated.
          We have a modified version of MITK here:
          https://github.com/NifTK/MITK.git
          
Library : Slicer Execution Model 
Website : https://github.com/Slicer/SlicerExecutionModel
Purpose : Part of the build system for building command line apps
License : Doc/Licenses/SlicerExecutionModel (Slicer license)
          
Library : Qt
Website : http://qt.nokia.com/products
Purpose : Gui framework.
License : Doc/Licenses/Qt.txt (LGPL v2.1 license)

Library : CTK
Website : http://www.commontk.org
Purpose : Common medical imaging specific classes.
License : Doc/Licenses/CTK_LICENSE.txt (Apache v2.0 license)

Library : NiftyReg
Website : http://sourceforge.net/projects/niftyreg/ 
Purpose : Image Registration Library
License : Doc/Licenses/NiftyReg.txt (BSD 3 clause license)

Library : NiftySeg
Website : http://sourceforge.net/projects/niftyseg/ 
Purpose : Image Segmentation Library
License : Doc/Licenses/NiftySeg.txt (BSD 3 clause license)

Library : NiftySim
Website : http://sourceforge.net/projects/niftysim/
Purpose : Non-Linear Finite Element Solver Library
License : Doc/Licenses/NiftySim.txt (BSD 3 clause license)

Library : NiftyRec
Website : http://sourceforge.net/projects/niftyrec/
Purpose : Tomographic Reconstruction Library
License : Doc/Licenses/NiftyRec.txt (BSD 3 clause license)

Library : NiftyLink
Website : http://cmicdev.cs.ucl.ac.uk/NiftyLink/html/index.html
Purpose : Messaging Library to talk to client applications
License : Doc/Licenses/NiftyLink.txt (BSD 3 clause license)
