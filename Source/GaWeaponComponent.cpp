/**************************************************************************
*
* File:		GaWeaponComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaWeaponComponent.h"

#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnDebugRenderComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "Base/BcProfiler.h"
#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaWeaponComponent );

void GaWeaponComponent::StaticRegisterClass()
{
	ReRegisterClass< GaWeaponComponent, Super >()
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaWeaponComponent::initialise( const Json::Value& Object )
{

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWeaponComponent::update( BcF32 Tick )
{
	Super::update( Tick );

	



	// Draw a debug ball..
	ScnDebugRenderComponent::pImpl()->drawEllipsoid( 
		getParentEntity()->getWorldPosition(),
		MaVec3d( 1.0f, 1.0f, 1.0f ),
		RsColour::GREEN,
		0 );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWeaponComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	BcU32 Idx = 0;

	auto spawnRobot = [ this, &Idx ]( MaVec3d Position, MaVec3d Rotation )
	{
		ScnEntitySpawnParams EntityParams = 
		{
			"default", BcName( "RobotEntity", Idx % 2 ), BcName( "RobotEntity", Idx ),
			MaMat4d(),
			getParentEntity()
		};

		++Idx;

		EntityParams.Transform_.rotation( Rotation );
		EntityParams.Transform_.translation( Position );
		ScnCore::pImpl()->spawnEntity( EntityParams );
	};

	spawnRobot( MaVec3d( -32.0f, 0.0f, 0.0f ), MaVec3d( 0.0f, 0.0f, 0.0f ) );
	spawnRobot( MaVec3d( 32.0f, 0.0f, 0.0f ), MaVec3d( 0.0f, BcPI, 0.0f ) );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWeaponComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}
