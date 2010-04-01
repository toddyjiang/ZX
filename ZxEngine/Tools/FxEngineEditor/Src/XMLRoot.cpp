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
#include <iostream>
#include <fstream>
#include <string>

#include "boost/lexical_cast.hpp"

#include "XMLRoot.h"
#include "XMLOptions.h"

#define FEF_ROOT "<FxEngineEditor>"
#define FEF_ROOT_END "</FxEngineEditor>"
#define VERSION_TAG_OBJECT "<XMLVersion>"
#define VERSION_TAG_OBJECT_END "</XMLVersion>"

#define MAJOR_TAG_ELEMENT "<major "
#define MINOR_TAG_ELEMENT "<minor "

#define MAJOR_VERSION 1
#define MINOR_VERSION 2

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
      //
      std::istrstream iss( strResult.c_str() );
      // tenter la conversion vers Dest
      Dest = boost::lexical_cast<T>(strResult);
      return /*iss >> Dest !=*/ 0;     
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

Int32 CXMLRoot::ReadXML(VectorOfPath& PathVector, Bool& LockOnStart)
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
        else if(strLine.find(DIR_TAG_OBJECT) < strLine.size())
        {
            CXMLPathManager PathManagerXML;
            PathManagerXML.ReadObject(fd, PathVector);
        }
        else if(strLine.find(OPTION_TAG_OBJECT) < strLine.size())
        {
            CXMLOptionsManager OptionsManagerXML;
			OptionsManagerXML.ReadObject(fd, LockOnStart);
        }
    }

    fd.close();

    return FX_OK;
}

Int32 CXMLRoot::WriteXML(VectorOfPath& PathVector, Bool& LockOnStart)
{
    std::ofstream fd(_strFilePath.c_str());
    if(fd.is_open() == FALSE)
        return FX_INVALPARAM;

    if(PathVector.size() == 0)
        return FX_INVALPARAM;

    CXMLPathManager PathManagerXML;
    CXMLOptionsManager OptionsManagerXML;
    
    fd << FEF_ROOT << endl;
    {
        fd << VERSION_TAG_OBJECT << endl;
        {
            fd << MAJOR_TAG_ELEMENT << " id=\"" << MAJOR_VERSION << "\"/>" << endl;
            fd << MINOR_TAG_ELEMENT << " id=\"" << MINOR_VERSION << "\"/>" << endl;
        }
        fd << VERSION_TAG_OBJECT_END << endl;

        fd << DIR_TAG_OBJECT << endl;
        {
            PathManagerXML.WriteObject(fd, PathVector);
        }
        fd << DIR_TAG_OBJECT_END << endl;

        fd << OPTION_TAG_OBJECT << endl;
        {
            OptionsManagerXML.WriteObject(fd, LockOnStart);
        }
        fd << OPTION_TAG_OBJECT_END << endl;
    }
    fd << FEF_ROOT_END << endl;

    fd.close();

    return FX_OK;
}

