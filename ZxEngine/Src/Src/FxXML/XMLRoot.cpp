/* __________________________________________________________________________
	
	FxEngine Framework. 
	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.

	This file is part of FxEngine Framework.
    The FxEngine Framework library is free software; you can redistribute it
	and/or modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library.
	if not,  If not, see <http://www.gnu.org/licenses/>.

   ____________________________________________http://www.SMProcess.com______ */
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <boost/lexical_cast.hpp>

#include "XMLRoot.h"

namespace FEF {

#define FEF_ROOT "<FEF>"
#define FEF_ROOT_END "</FEF>"
#define VERSION_TAG_OBJECT "<XMLVersion>"
#define VERSION_TAG_OBJECT_END "</XMLVersion>"

#define MAJOR_TAG_ELEMENT "<major "
#define MINOR_TAG_ELEMENT "<minor "

#define MAJOR_VERSION 1
#define MINOR_VERSION 3

template < class T >
  struct check_string
  {
    static void from_string( const std::string & src, T & dest )
    {
      //
      std::istringstream iss( src );
      // Conversion vers Dest
      iss >> dest;
    }
  }; 

  template < >
  struct check_string < std::string >
  {
    static void from_string( const std::string & src, std::string & dest )
    {
      dest = src;
    }
  }; 

  template < class T >
  void from_string( const std::string & src, T & dest )
  {
    check_string<T>::from_string( src, dest );
  }

template<typename T>
bool getValue(std::string& str, T &Dest )
{
    size_t pfrom, pto;
    pfrom = 0;
    pto   = 0;
    std::string  strTag = "\"";
    pfrom = str.find(strTag,pfrom);
    pto   = str.find(strTag  ,pfrom+1);

    std::string strResult;
    if (pfrom >= 0 && pto > 0 && pfrom!=pto)
    {
      strResult = str.substr(pfrom+strTag.size(),pto-pfrom-strTag.size());
      str = str.erase(pfrom,pto-pfrom+1);
      Dest = boost::lexical_cast<T>(strResult);
      return /*iss >> Dest != */0;     
    }
    return 1;
}


CXMLRoot::CXMLRoot(const std::string strFilePath) : 
_strFilePath(strFilePath)
{

}

CXMLRoot::~CXMLRoot()
{
}

Int32 CXMLRoot::ReadXML(MapFxIdToFxXML& FxIdToFxXMLMap, MapIdToConnectionXML& IdToConnectionXMLMap)
{
    std::ifstream fd(_strFilePath.c_str());
    if(fd.is_open() == FALSE)
        return FX_INVALPARAM;

    /*! Get line of xml and create the FEF objects */
    std::string strLine = "";
    
    while (getline(fd, strLine))
	{
        if(strLine.find(MAJOR_TAG_ELEMENT) < strLine.size())
        {
            getValue(strLine, _wMajor);
        }
        else if(strLine.find(MINOR_TAG_ELEMENT) < strLine.size())
        {
            getValue(strLine, _wMinor);
        }
        else if(strLine.find(FX_TAG_OBJECT) < strLine.size())
        {
            CXMLFxManager FxManagerXML;
            FxManagerXML.ReadObject(fd, FxIdToFxXMLMap);
        }
        else if(strLine.find(CONNECTION_TAG_OBJECT) < strLine.size())
        {
            CXMLConnectionManager ConnectionManagerXML;
			ConnectionManagerXML.ReadObject(fd, IdToConnectionXMLMap);
        }
    }

    fd.close();

    return FX_OK;
}

Int32 CXMLRoot::WriteXML(MapFxIdToFxXML& FxIdToFxXMLMap, MapIdToConnectionXML& IdToConnectionXMLMap)
{
    std::ofstream fd(_strFilePath.c_str());
    if(fd.is_open() == FALSE)
        return FX_INVALPARAM;

    if(FxIdToFxXMLMap.empty() == TRUE)
        return FX_INVALPARAM;

    CXMLFxManager FxManagerXML;
    CXMLConnectionManager ConnectionManagerXML;

    fd << FEF_ROOT << endl;
    {
        fd << VERSION_TAG_OBJECT << endl;
        {
            fd << MAJOR_TAG_ELEMENT << " id=\"" << MAJOR_VERSION << "\"/>" << endl;
            fd << MINOR_TAG_ELEMENT << " id=\"" << MINOR_VERSION << "\"/>" << endl;
        }
        fd << VERSION_TAG_OBJECT_END << endl;

        fd << FX_TAG_OBJECT << endl;
        {
            FxManagerXML.WriteObject(fd, FxIdToFxXMLMap);
        }
        fd << FX_TAG_OBJECT_END << endl;

        fd << CONNECTION_TAG_OBJECT << endl;
        {
            ConnectionManagerXML.WriteObject(fd, IdToConnectionXMLMap);
        }
        fd << CONNECTION_TAG_OBJECT_END << endl;
    }
    fd << FEF_ROOT_END << endl;

    fd.close();

    return FX_OK;
}

 } //namespace FEF
