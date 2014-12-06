/**************************************************************************
*
* File:		GaWorldComponent.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaWorldComponent_H__
#define __GaWorldComponent_H__

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef ReObjectRef< class GaWorldComponent > GaWorldComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaWorldComponent
class GaWorldComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( GaWorldComponent, ScnComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcF32 Tick );
	
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:

};

#endif

