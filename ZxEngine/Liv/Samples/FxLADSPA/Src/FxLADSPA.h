/* __________________________________________________________________________

	Fx sample code. 

	This material has been supplied as part of the FxEngine Framework.
	Under copyright laws, this material may not be duplicated in whole
	or in part, except for personal use, without the express written consent
	of SMProcess. Refer to the license agreement contained with this Framework
	before using any part of this material.

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	PURPOSE.

	Email:  info@SMProcess.com

	Copyrights (c) 2009 SMProcess, all rights reserved.

   __________________________________________________________________________
*//*!
   @file  FxLADSPA.h
   @brief The Fx LADSPA allows to load and to use LADSPA plugins. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/
#include "IFxBase.h"
using namespace FEF; //!< Using FxEngineFramework

#include "LADSPAWrapper.h" //!< LADSPA wrapper

#pragma warning(disable : 4995)	//!< stdio.h
#pragma warning(disable : 4996) //!< stdlib.h

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
#define FX_VERSION        ("1.1.0.0")

class CFxLADSPA:public IFxBase
{
public:
	CFxLADSPA(void);
	~CFxLADSPA();

public:
	//! The IFxBase classe implementation
	virtual Int32 GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor);
	virtual Int32 InitFx(IFx **ppFx);
	virtual Int32 UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter);
	virtual Int32 DisplayFxPropertyPage(Pvoid pvWndParent);
    virtual Int32 StartFx();
    virtual Int32 RunFx();
	virtual Int32 StopFx();
	virtual Int32 ReleaseFx();
    
private:
	FX_DESCRIPTOR   *_pFxDescriptor;  /*!< Contains the Fx description */
	IFx *_pFx;					/*!< The IFx interface */
	IFxParam        *_pFxParam; /*!< The IFxParam interface on Fx */
	IFxState*		_pFxState;	/*!< The IFxState interface on Fx */
	FX_STATE		_FxState;		/*!< Contains the Fx state */

private:
	CLADSPAWrapper* _pWrapper;  /*!< The Fx wrapper */
};

