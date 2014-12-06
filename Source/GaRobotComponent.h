/**************************************************************************
*
* File:		GaRobotComponent.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaRobotComponent_H__
#define __GaRobotComponent_H__

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef ReObjectRef< class GaRobotComponent > GaRobotComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaRobotComponent
class GaRobotComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( GaRobotComponent, ScnComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcF32 Tick );
	
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:

};

#endif

