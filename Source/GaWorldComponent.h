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
	enum class HotspotType
	{
		// Per op.
		STATE,
		CONDITION,
		CONDITION_VAR,
		OPERATION,
		OPERATION_VAR,
		ADD_OP,
		DEL_OP,

		// Selection.
		STATE_SELECTION,
		CONDITION_SELECTION,
		CONDITION_VAR_SELECTION,
		OPERATION_SELECTION,
		OPERATION_VAR_SELECTION,

		//
		STATE_MAIN,

		// 
		START,
		RESET,

		SAMPLE1,
		SAMPLE2,

		//
		INVALID
	};

	enum class GuiState
	{
		MAIN,
		SELECTION_STATE,
		SELECTION,
		SELECTION_VAR,
	};


	struct Hotspot
	{
		MaVec2d Position_;
		MaVec2d Extents_;
		BcU32 ID_;
		HotspotType Type_;
	};

public:
	DECLARE_RESOURCE( GaWorldComponent, ScnComponent );

	void initialise( const Json::Value& Object );

	virtual void update( BcF32 Tick );

	void onClick( const Hotspot& ClickedHotspot, MaVec2d MousePosition );
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );


	
	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseMove( EvtID ID, const OsEventInputMouse& Event );

	std::vector< class GaRobotComponent* > getRobots( BcU32 Team );
	std::vector< class GaWeaponComponent* > getWeapons( MaVec3d Position, BcF32 Radius );

	void startNewGame();
	void reset();

	BcBool isGamePlaying();

public:
	std::vector< GaRobotOperation > Program_;
	std::vector< GaRobotOperation > CurrentEnemyAI_;

	ScnCanvasComponentRef Canvas_;
	ScnMaterialComponentRef Material_;
	ScnViewComponentRef View_;
	ScnFontComponentRef Font_;

	std::vector< OsEventInputMouse > MouseEvents_;
	static OsEventInputMouse MouseMoveEvent_;

	// Main/all gui stuff.
	GuiState GuiState_;
	MaVec2d SelectionPosition_;

	// For the selection states.
	BcU32 SelectedID_;
	HotspotType HotspotType_;

	Hotspot LastHighlightedHotspot_;

	GaRobotComponent* PlayerRobot_;
	GaRobotComponent* EnemyRobot_;

	BcBool HandledWin_;
	std::string GameOverMessage_;

	MaVec3d PanelPosition_;
	MaVec3d TargetPanelPosition_;

	BcU32 Wins_;
	BcU32 Loses_;
};

#endif

