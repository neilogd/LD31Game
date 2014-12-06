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
#include "System/Os/OsEvents.h"

#include "GaRobotComponent.h"

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

	void initialise( const Json::Value& Object );

	virtual void update( BcF32 Tick );
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );
	
	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseMove( EvtID ID, const OsEventInputMouse& Event );

	std::vector< class GaRobotComponent* > getRobots( BcU32 Team );
	std::vector< class GaWeaponComponent* > getWeapons( MaVec3d Position, BcF32 Radius );

public:
	std::vector< GaRobotOperation > Program_;

	ScnCanvasComponentRef Canvas_;
	ScnMaterialComponentRef Material_;
	ScnViewComponentRef View_;
	ScnFontComponentRef Font_;

	OsEventInputMouse MouseMoveEvent_;
	std::vector< OsEventInputMouse > MouseEvents_;

	enum class HotspotType
	{
		// Per op.
		CONDITION,
		CONDITION_VAR,
		OPERATION,
		OPERATION_VAR,
		ADD_OP,
		DEL_OP,
	};

	struct Hotspot
	{
		MaVec2d TL_;
		MaVec2d BR_;
		BcU32 ID_;
		HotspotType Type_;
	};
};

#endif

