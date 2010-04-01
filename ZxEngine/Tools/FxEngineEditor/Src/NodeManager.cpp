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
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <list>
#include <map>
#include <algorithm>

using namespace std;

#include "Resource.h"
#include "EditorDefs.h"

#include "FxEngine.h"

#include "Node.h"

#include "NodeManager.h"

/*----------------------------------------------------------------------*//*!
				 Construction/Destruction
*//*-----------------------------------------------------------------------*/
CNodeManager* CNodeManager::_instance = NULL;

CNodeManager* CNodeManager::Instance () {
  if (_instance == 0) {
      _instance = new CNodeManager;
  }
  return _instance;
}

CNodeManager::CNodeManager()
{

}

CNodeManager::~CNodeManager()
{
	/*!  */
	NodeMap::iterator Itmap;
	ListOfNodeIter Itlist;
	for ( Itmap = _NodeMap.begin( ); Itmap != _NodeMap.end( ); Itmap++ )
	{
		ListOfNode* pListOfNode = Itmap->second;
		pListOfNode->clear();
		SAFE_DELETE_OBJECT(pListOfNode);
	}
	_NodeMap.clear();

	_instance = NULL;
}

/*----------------------------------------------------------------------*//*!
	Add().

	The Add method adds a new point to the point manager.

    @param pNode:		Pointer to a variable that contains the
							new point to add.

	@return	None.
*//*-----------------------------------------------------------------------*/
Void CNodeManager::Add(CNode* pNode)
{
	ListOfNode* ListOfLink = NULL;
	ListOfLink = new ListOfNode;

	_NodeMap.insert(std::make_pair(pNode, ListOfLink));
	return;
}

/*----------------------------------------------------------------------*//*!
	Remove().

	The Remove method removes a point from the point manager.
	All linked points are removed.

    @param pNode:		Pointer to a variable that contains the
							new point to remove.

	@return	None.
*//*-----------------------------------------------------------------------*/
Void CNodeManager::Remove(CNode* pNode)
{
	NodeMap::iterator Itmap;
	Itmap = _NodeMap.find( pNode );
	if( Itmap == _NodeMap.end() )
		return;

	ListOfNode* pListOfNode = Itmap->second;

	/*! Clear all point linked */
	pListOfNode->clear();
	SAFE_DELETE_OBJECT(pListOfNode);			/*!< Delete list of linked points */

	_NodeMap.erase(Itmap);

	return;
}

CNode* CNodeManager::GetNode(wxPoint& point)
{
	NodeMap::iterator Itmap;
	for ( Itmap = _NodeMap.begin( ); Itmap != _NodeMap.end( ); Itmap++ )
	{
        if((Itmap->first)->IsFx())
		    if((Itmap->first)->PointInRegion(point))
			    return (Itmap->first);
	}
	return NULL;
}

/*----------------------------------------------------------------------*//*!
	Move().

	The Move method moves a linkpoint.
	All linked points are moved too.

    @param pNode:		Pointer to a variable that contains the
							new point to move.
	@param point:			reference to the new point coordinates.

	@return	None.
*//*-----------------------------------------------------------------------*/
Void CNodeManager::Move(CNode* pNode, wxPoint& point)
{
	/*! Move current point */
	pNode->Move(&point);

	/*! Move all linked points to it */
	NodeMap::iterator Itmap;
	Itmap = _NodeMap.find( pNode );
	if( Itmap == _NodeMap.end() )
		return;

	ListOfNode* pListOfNode = Itmap->second;

	/*!< Move points */
	ListOfNodeIter Itlist;
#ifdef WIN32
	std::for_each(pListOfNode->begin(), pListOfNode->end(),
		std::bind2nd(std::mem_fun1(&CNode::Move), &point));
#else
    std::for_each(pListOfNode->begin(), pListOfNode->end(),
		std::bind2nd(std::mem_fun(&CNode::Move), &point));
#endif
	return;
}

/*----------------------------------------------------------------------*//*!
	Link().

	The Nodes try to link all points between them in point manager .

	@return	None.
*//*-----------------------------------------------------------------------*/
Void CNodeManager::Link()
{
	/*! Link point if it is in reg */
	NodeMap::iterator Itmap;
	for ( Itmap = _NodeMap.begin( ); Itmap != _NodeMap.end( ); Itmap++ )
	{
        /*! Try link only begin and End point of line */
		if(Itmap->first->IsEnd())
		    Link(Itmap->first);
	}

	return;
}

/*----------------------------------------------------------------------*//*!
	Link().

	The Link try to link a point with all others point in the point manager.

    @param pNode:		Pointer to a variable that contains the
							point to link.

	@return	None.
*//*-----------------------------------------------------------------------*/
Bool CNodeManager::Link(CNode* pNode, Bool ShouldBothDir)
{
	/*! Link point if it is in reg */
	NodeMap::iterator Itmap;
	for ( Itmap = _NodeMap.begin( ); Itmap != _NodeMap.end( ); Itmap++ )
	{
		if( Itmap->first != pNode)
			if( (Itmap->first)->NodeInRegion(pNode) )
			{
				/*! Only one linked point */
				ListOfNode* pListOfNode = Itmap->second;
				if(pListOfNode->size() > 0)
				{
					/*! Is the same link */
					if(*(pListOfNode->begin()) == pNode)
						return TRUE;
					return FALSE;
				}
				if(Itmap->first->IsFx())
					pNode->SetType(Itmap->first->GetType());

                if(ShouldBothDir)
				{
					/*! Add link between us */
					Link(pNode, (Itmap->first));
				}
				pListOfNode->push_back(pNode);
			}
	}

	return TRUE;
}

/*----------------------------------------------------------------------*//*!
	Link().

	The Link try to link a point with an other point.

    @param pNode:		    Pointer to a variable that contains the
							    point to link.
	@param pNodeToAdd:		Pointer to a variable that contains the
							    point to link.

	@return	None.
*//*-----------------------------------------------------------------------*/
Bool CNodeManager::Link(CNode* pNode1, CNode* pNode2)
{
	if(pNode2->NodeInRegion(pNode1))
	{
		NodeMap::iterator Itmap;
		Itmap = _NodeMap.find( pNode1 );
		if( Itmap != _NodeMap.end() )
		{
			ListOfNode* pListOfNode = Itmap->second;
			/*! Only one linked point */
			if(pListOfNode->size() > 0)
				return FALSE;
			pListOfNode->push_back(pNode2);

			/*! Set the same point */
			wxPoint Point;
			pNode2->GetPoint(&Point);
			pNode1->Move(&Point);

		}
	}
	return TRUE;
}

/*----------------------------------------------------------------------*//*!
	UnLink().

	The UnLink try to remove a linked point in all other points.

    @param pNode:		    Pointer to a variable that contains the
							    point to remove.

	@return	None.
*//*-----------------------------------------------------------------------*/
Void CNodeManager::UnLink(CNode* pNode)
{
	NodeMap::iterator Itmap;
	/*! Find all Node linked and remove it */
	for ( Itmap = _NodeMap.begin( ); Itmap != _NodeMap.end( ); Itmap++ )
	{
		ListOfNode* pListOfNode = Itmap->second;

		ListOfNodeIter It;
		It = std::find(pListOfNode->begin(), pListOfNode->end(), pNode);
		if( It != pListOfNode->end() )
		{
			pListOfNode->erase(It);
		}
	}

	return;
}

Void CNodeManager::UnLinkList(CNode* pNode)
{
	NodeMap::iterator Itmap;
	Itmap = _NodeMap.find( pNode );
	if( Itmap == _NodeMap.end() )
		return;

	ListOfNode* pListOfNode = Itmap->second;
	pListOfNode->clear();

	return;
}

CNode* CNodeManager::GetFirstLink(CNode* pNode)
{
	NodeMap::iterator Itmap;
	Itmap = _NodeMap.find( pNode );
	if( Itmap == _NodeMap.end() )
		return NULL;

	ListOfNode* pListOfNode = Itmap->second;

	if(pListOfNode->size() > 0)
		return (*(pListOfNode->begin()));

	return NULL;
}
