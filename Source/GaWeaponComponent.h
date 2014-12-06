/**************************************************************************
*
* File:		GaWeaponComponent.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaWeaponComponent_H__
#define __GaWeaponComponent_H__

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef ReObjectRef< class GaWeaponComponent > GaWeaponComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaWeaponComponent
class GaWeaponComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( GaWeaponComponent, ScnComponent );

	void initialise( const Json::Value& Object );

	virtual void update( BcF32 Tick );
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );
	
public:
	MaVec3d TargetPosition_;
	MaVec3d	Velocity_;
	BcF32 MaxVelocity_;
	BcF32 Damage_;
	BcF32 Radius_;
};

#endif

