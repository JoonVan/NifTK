/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <ostream>
#include <sstream>
#include <deque>
#include "niftkFileHelper.h"
#include "niftkEnvironmentHelper.h"
#include <boost/regex.hpp>

namespace fs = boost::filesystem;

namespace niftk
{


//-----------------------------------------------------------------------------
std::string GetFileSeparator()
{	
  return FILE_SEPARATOR;
}


//-----------------------------------------------------------------------------
std::string ConcatenatePath(const std::string& path, const std::string& name)
{
  if ( ( path.length() > 0 ) && 
       ( path.substr( path.length() - 1 ) != GetFileSeparator() ) &&
       ( name.length() > 0 ) && 
       ( name.substr( 0, 1 ) != GetFileSeparator() ) )
    
  {
    return path + GetFileSeparator() + name;
  }
  else
  {
    return path + name;
  }
}


//-----------------------------------------------------------------------------
fs::path ConvertToFullPath(const std::string& pathName)
{
  if (pathName.length() == 0)
    {
      throw std::logic_error("Empty pathName supplied");
    }

  fs::path full_path(fs::initial_path<fs::path>());
  full_path = fs::system_complete(fs::path(pathName));
  return full_path;
}


//-----------------------------------------------------------------------------
std::string ConvertToFullNativePath(const std::string& pathName)
{
  fs::path full_path = ConvertToFullPath(pathName);  
  return full_path.string();
}


//-----------------------------------------------------------------------------
std::string Basename(const std::string& pathName)
{
  fs::path full_path(fs::initial_path<fs::path>());
  full_path = fs::system_complete(fs::path(pathName));
  return fs::basename(full_path);
}


//-----------------------------------------------------------------------------
fs::path CreateUniqueTempFileName(const std::string &prefix, const std::string &suffix) throw (niftk::IOException) {
  fs::path tmpFileName;
  std::string tmpDirName, fileNameTemplate;

#ifdef _WIN32
  tmpDirName = getenv("TMP");
#else
  tmpDirName = "/tmp";
#endif
  
  fileNameTemplate = (fs::path(tmpDirName)/fs::path(prefix + "XXXXXX" + suffix)).string();

#if defined HAVE_MKSTEMPS
  {
    char *p_namebuffer;
    int pathTmpLength;

    pathTmpLength = fileNameTemplate.length();
    p_namebuffer = new char[pathTmpLength + 1];
    std::copy(fileNameTemplate.begin(), fileNameTemplate.end(), p_namebuffer);
    p_namebuffer[pathTmpLength] = 0;

    if (mkstemps(p_namebuffer, suffix.length()) < 0) {
      throw niftk::IOException("Failed to create unique temp. file.");
    }

    tmpFileName = fs::path(p_namebuffer);
    delete[] p_namebuffer;

    return tmpFileName;
  }
#else
  {
    const int maxTries = 10;

    int currTry;


    /*
     * Custom implementation of mkstemps
     */
    for (currTry = 0; currTry < maxTries; currTry++) {
      std::string tmpPath;
      std::string::iterator i_char;

      assert(*(fileNameTemplate.end() - suffix.length() - 6) == 'X' && *(fileNameTemplate.end() - suffix.length() - 1) == 'X');
      tmpPath = fileNameTemplate;
      for (i_char = tmpPath.end() - suffix.length() - 6; i_char < tmpPath.end() - suffix.length(); i_char++) {
        assert(*i_char == 'X');
        switch (rand()%3) {
        case 0:
          *i_char = rand()%('z' - 'a') + 'a';
          break;

        case 1:
          *i_char = rand()%('Z' - 'A') + 'A';
          break;

        default:
          *i_char = rand()%('9' - '0') + '0';
        }
      }

      if (!fs::exists(tmpPath)) {
        std::ofstream(tmpPath.c_str());
        tmpFileName = fs::path(tmpPath);
        break;
      }
    }

    if (currTry == maxTries) {
      throw niftk::IOException("Failed to create unique temp. file.");
    }

    return tmpFileName;
  }
#endif
}


//-----------------------------------------------------------------------------
bool DirectoryExists(const std::string& directoryPath)
{
  fs::path full_path = ConvertToFullPath(directoryPath);
  return fs::is_directory(full_path);
}


//-----------------------------------------------------------------------------
bool CreateDirAndParents(const std::string& directoryPath)
{
  std::deque< fs::path > directoryTree;
  std::deque< fs::path >::iterator iterDirectoryTree;       

  fs::path full_path = ConvertToFullPath(directoryPath);
  fs::path branch = full_path;

  while ( ! branch.empty() )
  {
    directoryTree.push_front( branch );
    branch = branch.branch_path();
  }

  for ( iterDirectoryTree = directoryTree.begin(); 
	iterDirectoryTree < directoryTree.end(); 
	++iterDirectoryTree )
  {
    if ( ! fs::exists( *iterDirectoryTree ) )
    {
      if ( ! fs::create_directory( *iterDirectoryTree ) )
      {
        return false;
      }
    }
  }

  return true;
}


//-----------------------------------------------------------------------------
bool FileExists(const std::string& fileName)
{
  fs::path full_path = ConvertToFullPath(fileName);
  return fs::is_regular(full_path);
}


//-----------------------------------------------------------------------------
int FileSize(const std::string& fileName)
{
  fs::path full_path = ConvertToFullPath(fileName);
  return (int)fs::file_size(full_path);
}


//-----------------------------------------------------------------------------
bool FileIsEmpty(const std::string& fileName)
{
  return FileSize(fileName) == 0;
}


//-----------------------------------------------------------------------------
bool FilenameHasPrefixAndExtension(
    const std::string& filename,
    const std::string& prefix,
    const std::string& extension)
{
  bool result = false;
  
  size_t prefixIndex = filename.find(prefix);
  size_t extensionIndex = filename.rfind(extension);
  size_t dotIndex = filename.rfind(".");
  size_t extensionLength = extension.length();
  
  if (prefixIndex == 0
      && 
        (
           (extension.length() > 0 && extensionIndex == (filename.length() - extensionLength) && (extensionIndex - dotIndex) == 1)
        || (extension.length() == 0))
      )
    {
      result = true;
    }
  return result;
}


//-----------------------------------------------------------------------------
bool FilenameMatches(
    const std::string& filename,
    const std::string& prefix,
    const std::string& middle,
    const std::string& extension)
{

  bool result = false;
  std::string tmp;
  
  // If extension is empty, then you wouldnt expect the "." either.
  if (extension.length() == 0)
    {
      tmp = prefix + middle;
    }
  else
    {
      tmp = prefix + middle + "." + extension;
    }
    
  if (filename.compare(tmp) == 0)
    {
      result = true;  
    }
  
  return result;
}


//-----------------------------------------------------------------------------
std::string GetImagesDirectory()
{
  return ConcatenatePath(GetNIFTKHome(), "images"); 
}


//-----------------------------------------------------------------------------
std::vector<std::string> GetFilesInDirectory(const std::string& fullDirectoryName)
{
  if (!DirectoryExists(fullDirectoryName))
  {
    std::ostringstream message;
    message << "Directory " << fullDirectoryName << " does not exist!";
    throw std::logic_error(message.str());
  }

  std::vector<std::string> fileNames;
  fs::path fullDirectoryPath = ConvertToFullPath(fullDirectoryName);

  fs::directory_iterator end_itr; // default construction yields past-the-end
  for ( fs::directory_iterator itr( fullDirectoryPath );
        itr != end_itr;
        ++itr )
  {
    if (!fs::is_directory(itr->path()))
    {
      fs::path fullFilePath(fs::initial_path<fs::path>() );
      fullFilePath = fs::system_complete(itr->path());
      fileNames.push_back(fullFilePath.string());
    }
  }
  return fileNames;
}


//  -------------------------------------------------------------------------
void GetRecursiveFilesInDirectory( const std::string &directoryName, 
				   std::vector<std::string> &fileNames )
{
  fs::path full_path( directoryName );

  if (!DirectoryExists(directoryName))
  {
    throw std::logic_error("Directory does not exist!");
    return;
  }

  if ( fs::is_directory( full_path ) )
  {
    fs::directory_iterator end_iter;

    for ( fs::directory_iterator dir_itr( full_path );
          dir_itr != end_iter;
          ++dir_itr )
    {
      try
      {
        if ( fs::is_directory( dir_itr->status() ) )
        { 
          GetRecursiveFilesInDirectory( dir_itr->path().string(), fileNames );
        }
        else if ( fs::is_regular_file( dir_itr->status() ) )
        {
          fileNames.push_back( dir_itr->path().string() );
        }
      }
      catch ( const std::exception & ex )
      {
        std::cerr << dir_itr->path() << " " << ex.what() << std::endl;
      }
    }
  }
  else // must be a file
  {
    fileNames.push_back( full_path.string() );    
  }
}


//  -------------------------------------------------------------------------
bool NumericStringCompare( const std::string &string1, const std::string &string2) 
{
  fs::path path1 (string1);
  fs::path path2 (string2);
  int d1 = boost::lexical_cast<long long int>(path1.stem().string());
  int d2 = boost::lexical_cast<long long int>(path2.stem().string());
  return d1 < d2;
}


//  -------------------------------------------------------------------------
std::vector<std::string> FindVideoData( std::string directory) 
{
  boost::filesystem::recursive_directory_iterator end_itr;
  std::vector<std::string> returnStrings;

  boost::regex avifilter ( "(.+)(.avi)", boost::regex::icase);
  for ( boost::filesystem::recursive_directory_iterator it(directory);
          it != end_itr ; ++it)
  {
    if ( boost::regex_match (it->path().string().c_str(), avifilter))
    {
      returnStrings.push_back(it->path().string());
    }
  }
  //also look for 264 files, but put them further along the vector
  for ( boost::filesystem::recursive_directory_iterator it(directory);
          it != end_itr ; ++it)
  {
    if (  it->path().extension() == ".264" )
    {
      returnStrings.push_back(it->path().string());
    }
  }
  return returnStrings;
}


//  -------------------------------------------------------------------------
std::vector<std::string> FindFilesWithGivenExtension(const std::string& fullDirectoryName, const std::string& extension)
{
  std::vector<std::string> returnStrings;
  boost::filesystem::recursive_directory_iterator endItr;

  if (niftk::DirectoryExists(fullDirectoryName))
  {
    for ( boost::filesystem::recursive_directory_iterator it(niftk::ConvertToFullNativePath(fullDirectoryName)); it != endItr; ++it)
    {
      if ( it->path().extension() == extension)
      {
        returnStrings.push_back(it->path().string());
      }
    }
  }
  return returnStrings;
}


//  -------------------------------------------------------------------------
std::string ExtractImageFileSuffix( const std::string fileName )
{
  std::string suffix;
  std::string compSuffix;       // The .gz or .zip suffix if present

  suffix.clear();
  compSuffix.clear();

  if ( ( fileName.length() >= 3 ) && 
       ( ( fileName.substr( fileName.length() - 3 ) == std::string( ".gz" ) ) || 
         ( fileName.substr( fileName.length() - 3 ) == std::string( ".GZ" ) ) ) )
  {
    compSuffix = fileName.substr( fileName.length() - 3 );
  }

  else if ( ( fileName.length() >= 4 ) && 
            ( ( fileName.substr( fileName.length() - 4 ) == std::string( ".zip" ) ) || 
              ( fileName.substr( fileName.length() - 4 ) == std::string( ".zip" ) ) ) )
  {
    compSuffix = fileName.substr( fileName.length() - 4 );
  }

  if ( ( fileName.length() >= 4 + compSuffix.length() ) && 
       ( ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".dcm" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".DCM" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".nii" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".NII" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".bmp" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".BMP" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".tif" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".TIF" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".jpg" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".JPG" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".png" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".PNG" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".ima" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".IMA" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".img" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".IMG" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".hdr" ) ) ||
         ( fileName.substr( fileName.length() - compSuffix.length() - 4, 4 ) == std::string( ".HDR" ) ) ) )
  {
    suffix = fileName.substr( fileName.length() - compSuffix.length() - 4 );
  }

  else if ( ( fileName.length() >= 5 + compSuffix.length() ) && 
            ( ( fileName.substr( fileName.length() - compSuffix.length() - 5, 5 ) == std::string( ".tiff" ) ) || 
              ( fileName.substr( fileName.length() - compSuffix.length() - 5, 5 ) == std::string( ".TIFF" ) ) ||
              ( fileName.substr( fileName.length() - compSuffix.length() - 5, 5 ) == std::string( ".gipl" ) ) ||
              ( fileName.substr( fileName.length() - compSuffix.length() - 5, 5 ) == std::string( ".GIPL" ) ) ) )
  {
    suffix = fileName.substr( fileName.length() - compSuffix.length() - 5 );
  }

  else if ( ( fileName.length() >= 6 + compSuffix.length() ) && 
            ( ( fileName.substr( fileName.length() - compSuffix.length() - 6, 6 ) == std::string( ".dicom" ) ) ||
              ( fileName.substr( fileName.length() - compSuffix.length() - 6, 6 ) == std::string( ".DICOM" ) ) ) )
  {
    suffix =  fileName.substr( fileName.length() - compSuffix.length() - 6 );
  }

  return suffix;
}


//  -------------------------------------------------------------------------
std::string ExtractImageFileSuffix( const std::string fileName,
                                    std::string &fileNameWithoutSuffix )
{
  std::string suffix = niftk::ExtractImageFileSuffix( fileName );
  fileNameWithoutSuffix = fileName.substr( 0, fileName.length() - suffix.length() );

  return suffix;
}


//  -------------------------------------------------------------------------
std::string AddStringToImageFileSuffix( const std::string fileName,
                                        std::string stringToAdd )
{
  std::string fileNameWithoutSuffix;
  std::string suffix = ExtractImageFileSuffix( fileName,
                                               fileNameWithoutSuffix );
  return fileNameWithoutSuffix + stringToAdd + suffix;
}


//  -------------------------------------------------------------------------
std::string ModifyImageFileSuffix( const std::string fileName,
                                   std::string newSuffix )
{
  std::string fileNameWithoutSuffix;
  niftk::ExtractImageFileSuffix( fileName, fileNameWithoutSuffix );
  return fileNameWithoutSuffix + newSuffix;
}



} // end namespace
