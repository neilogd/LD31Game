/**************************************************************************
*
* File:		GaTestSelectionComponent.h
* Author:	Neil Richardson 
* Ver/Date:
* Description:
*		Test selection component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaTestSelectionComponent_H__
#define __GaTestSelectionComponent_H__

#include "Psybrus.h"

#include "System/Os/OsEvents.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/Rendering/ScnFont.h"
#include "System/Debug/DsCore.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef ReObjectRef< class GaTestSelectionComponent > GaTestSelectionComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaTestSelectionComponent
class GaTestSelectionComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( GaTestSelectionComponent, ScnComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcF32 Tick );

	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
	eEvtReturn							onKeyPress( EvtID ID, const OsEventInputKeyboard& Event );

private:
	struct TMenuEntry
	{
		ScnEntityRef EntityToSpawn_;
	};

	std::vector< TMenuEntry >			Options_;
	BcU32								SelectedEntry_;
	ScnEntityRef						PreviousSpawned_;
	ScnFontComponentRef					FontComponent_;
	ScnCanvasComponentRef				Canvas_;

	MaMat4d								Projection_;

	void								LoadEntity(int Entity);

	BcU32 CreateEntity1_;
	BcU32 CreateEntity2_;
};

#endif

