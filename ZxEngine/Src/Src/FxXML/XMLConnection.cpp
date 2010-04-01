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

#include "XMLConnection.h"

namespace FEF {

template < class T >
  struct check_string
  {
    static void from_string( const std::string & src, T & dest )
    {
		std::istringstream iss( src );
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
      return 0;     
    }
    return 1;
}

CXMLConnectionManager::CXMLConnectionManager(void)
{
}

CXMLConnectionManager::~CXMLConnectionManager()
{
}

Int32 CXMLConnectionManager::ReadObject(std::ifstream& fd, MapIdToConnectionXML& IdToConnectionXMLMap)
{
    if(fd.is_open() == FALSE)
        return FX_ERROR;

    /*! Get line of xml and create the FEF objects */
    std::string strLine = "";
        
    while (getline(fd, strLine))
	{
        if(strLine.find(CONNECTION_TAG_OBJECT_CHILD)  < strLine.size())
        {            
			CXMLConnection* pFxConnection = new CXMLConnection(fd, strLine);
			IdToConnectionXMLMap.insert(make_pair(pFxConnection->GetId(), pFxConnection));
        }
        if(strLine.find(CONNECTION_TAG_OBJECT_END) < strLine.size())
            return FX_OK;
    }
    return FX_OK;
}

Int32 CXMLConnectionManager::WriteObject(std::ofstream& fd, MapIdToConnectionXML& IdToConnectionXMLMap)
{
    if(fd.is_open() == FALSE)
        return FX_ERROR;

    MapIdToConnectionXMLItr Itr;
    VectorOfFxLinkPointItr ItrVec;
    for(Itr = IdToConnectionXMLMap.begin(); Itr != IdToConnectionXMLMap.end(); Itr++)
    {
        VectorOfFxLinkPoint FxLinkPoint = (*Itr).second->GetVectorOfFxLinkPoint();
        fd << CONNECTION_TAG_OBJECT_CHILD << "id=\"" << (*Itr).first << "\">" << endl;
        {
            fd << FX_TAG_ELEMENT << "id=\"" << (*Itr).second->GetFxId(0) << "\"" << " pin=\"" << (*Itr).second->GetFxPinId(0) << "\"/>" << endl;
            fd << FX_TAG_ELEMENT << "id=\"" << (*Itr).second->GetFxId(1) << "\"" << " pin=\"" << (*Itr).second->GetFxPinId(1) << "\"/>" << endl;
            fd << MEDIA_TAG_ELEMENT << "main=\"" << (*Itr).second->GetMainMedia() << "\"" << " sub=\"" << (*Itr).second->GetSubMedia() << "\"/>" << endl;
            for(ItrVec = FxLinkPoint.begin(); ItrVec != FxLinkPoint.end(); ItrVec++)
            {
                fd << FXLINKPOINT_TAG_ELEMENT << "x=\"" << (*ItrVec)->dwX << "\"" << " y=\"" << (*ItrVec)->dwY << "\"/>" << endl;
            }
        }
        fd << CONNECTION_TAG_OBJECT_CHILD_END << endl; 
    }
            
    return FX_OK;
}

CXMLConnection::CXMLConnection(void)
{
}

CXMLConnection::CXMLConnection(std::ifstream& fd, std::string& strLine):
_wId(0),
_MainMedia(MAIN_TYPE_UNDEFINED),
_SubMedia(SUB_TYPE_UNDEFINED)
{
	Uint16* pwFxId = &_wFxId[0];
	Uint16* pwFxPinId = &_wFxPinId[0];

	getValue(strLine, _wId);
	if(fd.is_open() == TRUE)
	{
       do{
			if(strLine.find(FX_TAG_ELEMENT)  < strLine.size())
			{            
	            getValue(strLine, *pwFxId++);
				getValue(strLine, *pwFxPinId++);
			}
			if(strLine.find(MEDIA_TAG_ELEMENT)  < strLine.size())
			{            
                Uint16 wMedia;
	            getValue(strLine, wMedia);
                _MainMedia = (FX_MAIN_MEDIA_TYPE)wMedia;
				getValue(strLine, wMedia);
                _SubMedia = (FX_SUB_MEDIA_TYPE)wMedia;
			}
            if(strLine.find(FXLINKPOINT_TAG_ELEMENT)  < strLine.size())
			{            
                FXLINKPOINT* pFxLinkPoint = new FXLINKPOINT;
                getValue(strLine, pFxLinkPoint->dwX);
                getValue(strLine, pFxLinkPoint->dwY);
                _FxLinkPointVector.push_back(pFxLinkPoint);
            }
			if(strLine.find(CONNECTION_TAG_OBJECT_CHILD_END) < strLine.size())
				break;
		} while(getline(fd, strLine));
	}
}

 } //namespace FEF
