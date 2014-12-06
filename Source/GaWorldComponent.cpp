/**************************************************************************
*
* File:		GaWorldComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaWorldComponent.h"
#include "GaRobotComponent.h"
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
DEFINE_RESOURCE( GaWorldComponent );

void GaWorldComponent::StaticRegisterClass()
{
	ReRegisterClass< GaWorldComponent, Super >()
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaWorldComponent::initialise( const Json::Value& Object )
{

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWorldComponent::update( BcF32 Tick )
{
	Super::update( Tick );

	// Draw floor.
	ScnDebugRenderComponent::pImpl()->drawGrid( 
		MaVec3d( 0.0f, 0.0f, 0.0f ),
		MaVec3d( 500.0f, 0.0f, 500.0f ),
		1.0f,
		10.0f,
		0 );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWorldComponent::onAttach( ScnEntityWeakRef Parent )
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
void GaWorldComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}

//////////////////////////////////////////////////////////////////////////
// getRobots
std::vector< class GaRobotComponent* > GaWorldComponent::getRobots( BcU32 Team )
{
	std::vector< GaRobotComponent* > Robots;
	auto WorldEntity = getParentEntity();

	for( BcU32 Idx = 0; Idx < WorldEntity->getNoofComponents(); ++Idx )
	{
		auto Component = WorldEntity->getComponent( Idx );
		if( Component->isTypeOf< ScnEntity >() )
		{
			ScnEntityRef Entity( Component );
			auto RobotComponent = Entity->getComponentByType< GaRobotComponent >();
			if( RobotComponent != nullptr && ( RobotComponent->Team_ == Team || Team == BcErrorCode ) )
			{
				Robots.push_back( RobotComponent );
			}
		}
	}

	return std::move( Robots );
}

//////////////////////////////////////////////////////////////////////////
// getWeapons
std::vector< class GaWeaponComponent* > GaWorldComponent::getWeapons( MaVec3d Position, BcF32 Radius )
{
	std::vector< GaWeaponComponent* > Weapons;
	auto WorldEntity = getParentEntity();

	for( BcU32 Idx = 0; Idx < WorldEntity->getNoofComponents(); ++Idx )
	{
		auto Component = WorldEntity->getComponent( Idx );
		if( Component->isTypeOf< ScnEntity >() )
		{
			ScnEntityRef Entity( Component );
			auto WeaponComponent = Entity->getComponentByType< GaWeaponComponent >();
			if( WeaponComponent != nullptr )
			{
				auto WeaponTargetPosition = WeaponComponent->TargetPosition_;
				if( ( WeaponTargetPosition - Position ).magnitude() < Radius )
				{
					Weapons.push_back( WeaponComponent );
				}
			}
		}
	}

	return std::move( Weapons );
}
