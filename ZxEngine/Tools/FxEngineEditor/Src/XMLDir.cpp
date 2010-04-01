/* __________________________________________________________________________
	
	FxEngine Framework. 
	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.

	This file is part of FxEngine Editor.
    FxEngine Editor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FxEngine Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FxEngine Editor. If not, see <http://www.gnu.org/licenses/>.

   ____________________________________________http://www.SMProcess.com______ */
#include <strstream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include <boost/lexical_cast.hpp>

#include "XMLDir.h"

template < class T >
  struct check_string
  {
    static Void from_string( const std::string & src, T & dest )
    {
      std::istringstream iss( src );
      
      // Conversion vers Dest
      iss >> dest;
    }
    
  }; 
  
  template < >
  struct check_string < std::string >
  {
    static Void from_string( const std::string & src, std::string & dest )
    {
      dest = src;
    }
  }; 

  
  template < class T >
  Void from_string( const std::string & src, T & dest )
  {
    check_string<T>::from_string( src, dest );
  }


template<typename T>
Bool getValue(std::string& str, T &Dest )
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
      ////
      //istringstream iss( strResult );
      //// tenter la conversion vers Dest
      //from_string(strResult,Dest);
      Dest = boost::lexical_cast<T>(strResult);

      return /*iss >> Dest != */0;     
    }
    return 1;
}

CXMLPathManager::CXMLPathManager()
{
}

CXMLPathManager::~CXMLPathManager()
{
}

Int32 CXMLPathManager::ReadObject(std::ifstream& fd, VectorOfPath& PathVector)
{
    if(fd.is_open() == FALSE)
        return FX_ERROR;

    /*! Get line of xml and create the path objects */
    std::string strLine = "";
    std::string strPath;
        
    while (getline(fd, strLine))
	{
        if(strLine.find(DIR_TAG_ELEMENT)  < strLine.size())
		{            
            getValue(strLine, strPath);
            PathVector.push_back(strPath);
		}
        if(strLine.find(DIR_TAG_OBJECT_END)  < strLine.size())
            return FX_OK;
    }
    return FX_OK;
}

Int32 CXMLPathManager::WriteObject(std::ofstream& fd, VectorOfPath& PathVector)
{
    if(fd.is_open() == FALSE)
        return FX_ERROR;

    VectorOfPathItr Itr;
    for(Itr = PathVector.begin(); Itr != PathVector.end(); Itr++)
    {
        fd << DIR_TAG_ELEMENT << "id=\"" << (*Itr) << "\"/>" << endl;
    }
    return FX_OK;
 }
