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
#ifndef _LINK_POINT_MNG_H
#define _LINK_POINT_MNG_H

#pragma warning(disable : 4786)

//! define the container of points
typedef std::list<CNode*>    ListOfNode;
typedef ListOfNode::iterator		 ListOfNodeIter;

typedef std::map<CNode*, ListOfNode*> NodeMap;

class CNodeManager  
{
public:
  static CNodeManager* Instance();

	//! existing interface goes here
protected:
	CNodeManager();
private:
	static CNodeManager* _instance;
	NodeMap _NodeMap;

public:
	virtual ~CNodeManager();

public:
	Void Add(CNode* pNode);
	Void Remove(CNode* pNode);
	Void UnLinkList(CNode* pNode);
	CNode* GetNode(wxPoint& point);
	Void Move(CNode* pNode, wxPoint& point);
	Void Link();
	Bool Link(CNode* pNode, Bool ShouldBothDir = TRUE);
	Bool Link(CNode* pNode, CNode* pNodeToAdd);
	Void UnLink(CNode* pNode);
	CNode* GetFirstLink(CNode* pNode);

};
#endif

