/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-09-14 11:37:54 +0100 (Wed, 14 Sep 2011) $
 Revision          : $Revision: 7310 $
 Last modified by  : $Author: ad $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>
#include "itkTestMain.h" 

void RegisterTests()
{
  REGISTER_TEST(LaplacianSolverImageFilterTest);
  REGISTER_TEST(ScalarImageToNormalizedGradientVectorImageFilterTest);
  REGISTER_TEST(StreamlinesFilterTest);
  REGISTER_TEST(CorrectGMUsingPVMapTest);
  REGISTER_TEST(CorrectGMUsingNeighbourhoodTest);
  REGISTER_TEST(LagrangianInitializedStreamlinesFilterTest);
  REGISTER_TEST(Bourgeat2008Test);
  REGISTER_TEST(FourthOrderRungeKuttaVelocityFieldIntegrationTest);
  REGISTER_TEST(FourthOrderRungeKuttaVelocityFieldThicknessTest);
  REGISTER_TEST(DemonsRegistrationFilterTest);
  REGISTER_TEST(DemonsRegistrationFilterUpdateTest);
  REGISTER_TEST(AddUpdateToTimeVaryingVelocityFilterTest);
  REGISTER_TEST(GaussianSmoothVectorFieldFilterTest);
  REGISTER_TEST(RegistrationBasedCorticalThicknessFilterTest);
}
