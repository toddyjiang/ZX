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

#include "XMLOptions.h"

template < class T >
  struct check_string
  {
    static Void from_string( const std::string & src, T & dest )
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

CXMLOptionsManager::CXMLOptionsManager()
{
}

CXMLOptionsManager::~CXMLOptionsManager()
{
}

Int32 CXMLOptionsManager::ReadObject(std::ifstream& fd, Bool& isLockOnStart)
{
    if(fd.is_open() == FALSE)
        return FX_ERROR;

    /*! Get line of xml and create the path objects */
    std::string strLine = "";
            
    getline(fd, strLine);
    if(strLine.find(OPTION_TAG_ELEMENT_LOCK)  < strLine.size())
	{            
        getValue(strLine, isLockOnStart);
	}
       
    return FX_OK;
}

Int32 CXMLOptionsManager::WriteObject(std::ofstream& fd, Bool& isLockOnStart)
{
    if(fd.is_open() == FALSE)
        return FX_ERROR;

    fd << OPTION_TAG_ELEMENT_LOCK << "id=\"" << isLockOnStart << "\"/>" << endl;
    
    return FX_OK;
 }
