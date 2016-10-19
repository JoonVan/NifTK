/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkMathsUtils.h"
#include <numeric>
#include <algorithm>
#include <functional>
#include <cmath>

namespace niftk {

//-----------------------------------------------------------------------------
bool IsCloseToZero(const double& value, const double& tolerance)
{
  if (fabs(value) < tolerance)
  {
    return true;
  }
  else
  {
    return false;
  }
}


//-----------------------------------------------------------------------------
std::pair <double, double >  FindMinimumValues (
    std::vector < std::pair < double, double > > inputValues,
    std::pair < unsigned int , unsigned int >  * indexes )
{
  std::pair < double , double > minimumValues;

  if ( inputValues.size() > 0 )
  {
    minimumValues.first = inputValues[0].first;
    minimumValues.second = inputValues[0].second;

    if ( indexes != NULL )
    {
      indexes->first = 0;
      indexes->second = 0;
    }
  }
  for (unsigned int i = 0; i < inputValues.size(); i++ )
  {
    if ( inputValues[i].first < minimumValues.first )
    {
      minimumValues.first = inputValues[i].first;
      if ( indexes != NULL )
      {
        indexes->first = i;
      }
    }
    if ( inputValues[i].second < minimumValues.second )
    {
      minimumValues.second = inputValues[i].second;
      if ( indexes != NULL )
      {
        indexes->second = i;
      }
    }
  }
  return minimumValues;
}


//-----------------------------------------------------------------------------
double RMS(const std::vector<double>& input)
{
  double mean = Mean(input);
  return sqrt(mean);
}


//-----------------------------------------------------------------------------
double Mean(const std::vector<double>& input)
{
  if (input.size() == 0)
  {
    return 0;
  }
  double sum = std::accumulate(input.begin(), input.end(), 0.0);
  double mean = sum / input.size();
  return mean;
}


//-----------------------------------------------------------------------------
double Median(std::vector<double> input)
{
  if (input.size() == 0)
  {
    return 0;
  }
  else if (input.size() == 1)
  {
    return input[0];
  }
  else
  {
    std::sort(input.begin(), input.end());

    if (input.size() % 2 == 1)
    {
      return input[(input.size() - 1)/2];
    }
    else
    {
      return (input[input.size()/2 - 1] + input[input.size()/2])/2.0;
    }
  }
}


//-----------------------------------------------------------------------------
double StdDev(const std::vector<double>& input)
{
  if (input.size() < 2)
  {
    return 0;
  }

  double mean = niftk::Mean(input);

  std::vector<double> diff(input.size());
  std::transform(input.begin(), input.end(), diff.begin(), std::bind2nd(std::minus<double>(), mean));
  double squared = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  double stdev = std::sqrt(squared / ((double)(input.size()) - 1.0));
  return stdev;
}


//-----------------------------------------------------------------------------
double ModifiedSignum(double value)
{
  if ( value < 0.0 )
  {
    return -1.0;
  }
  return 1.0;
}


//-----------------------------------------------------------------------------
double SafeSQRT(double value)
{
  if ( value < 0 )
  {
    return 0.0;
  }
  return sqrt(value);
}

} // end namespace
