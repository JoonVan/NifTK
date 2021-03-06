/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkCSVRow_h
#define niftkCSVRow_h

#include <NifTKConfigure.h>
#include "niftkCommonWin32ExportHeader.h"

#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace niftk
{

/**
* \class CSVRow
* \brief Comma separated value (CSV) row class used to parse rows from
* an input stream and split them into a vector of strin elements.
*
* Example usage:
\code{.cpp}
int main()
{
  std::ifstream fin("fileDataInput.csv");

  niftk::CSVRow row;
  while( fin >> row )
  {
    std::cout << row << std::endl;
  }
}
\endcode
*/
class NIFTKCOMMON_WINEXPORT CSVRow
{
public:

  /** Construct CSVRow by allocating the vector */
  CSVRow();

  /** Destructor */
  ~CSVRow();

  /** Get an element of the row. */
  std::string const& operator[]( std::size_t index ) const
  {
    return (*m_Data)[index];
  }

  /** Return the size of the row (number of elements). */
  std::size_t Size() const
  {
    return m_Data->size();
  }

  /** Read the next row in the input stream */
  void ReadNextRow( std::istream& inStream );

private:

  std::vector<std::string> *m_Data;
};


/** Specialisation of 'operator>>' for the niftk::CSVRow class */
std::istream& operator>>( std::istream& str, CSVRow &data )
{
  data.ReadNextRow( str );
  return str;
}

/** Specialisation of 'operator<<' for the niftk::CSVRow class */
std::ostream& operator<<( std::ostream &os, const CSVRow &data )
{
  for (unsigned int i=0; i < data.Size(); i++)
  {
    os << data[i];

    if ( i < data.Size() - 1 )
    {
      os << ",";
    }
  }
  
  return os;
}

} // end namespace

#endif // niftkCSVRow_h
