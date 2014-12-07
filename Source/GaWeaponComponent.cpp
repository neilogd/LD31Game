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
#include "GaRobotComponent.h"
#include "GaWorldComponent.h"

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
	ReField* Fields[] = 
	{
		new ReField( "TargetPosition_", &GaWeaponComponent::TargetPosition_ ),
		new ReField( "Velocity_", &GaWeaponComponent::Velocity_ ),
		new ReField( "MaxVelocity_", &GaWeaponComponent::MaxVelocity_ ),
		new ReField( "Damage_", &GaWeaponComponent::Damage_ ),
		new ReField( "Radius_", &GaWeaponComponent::Radius_ ),
	};
	
	ReRegisterClass< GaWeaponComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaWeaponComponent::initialise( const Json::Value& Object )
{
	MaxVelocity_ = BcF32( Object[ "velocity" ].asDouble() );
	Damage_ = BcF32( Object[ "damage" ].asDouble() );
	Radius_ = BcF32( Object[ "radius" ].asDouble() );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWeaponComponent::update( BcF32 Tick )
{
	// Grab entity + position.
	auto Entity = getParentEntity();
	auto LocalPosition = Entity->getLocalPosition();

	// Move distance;
	BcF32 MoveDistance = MaxVelocity_ * Tick;

	// Move if we need to move towards our target position.
	if( ( TargetPosition_ - LocalPosition ).magnitudeSquared() > ( MoveDistance * MoveDistance ) )
	{
		Velocity_ = ( TargetPosition_ - LocalPosition ).normal() * MaxVelocity_;
	}
	else
	{
		Velocity_ = MaVec3d( 0.0f, 0.0f, 0.0f );
		LocalPosition = TargetPosition_;

		auto WorldComponent = getParentEntity()->getComponentAnyParentByType< GaWorldComponent >();
		auto Robots = WorldComponent->getRobots( BcErrorCode );
		for( auto* Robot : Robots )
		{
			auto RobotPosition = Robot->getParentEntity()->getLocalPosition();
			if( ( LocalPosition - RobotPosition ).magnitudeSquared() < ( Radius_ * Radius_ ) )
			{
				Robot->takeDamage( Damage_ );
			}
		}

		ScnCore::pImpl()->removeEntity( getParentEntity() );
	}

	LocalPosition += Velocity_ * Tick;

	// Set local position.
	Entity->setLocalPosition( LocalPosition );

	// Draw a debug ball..
	ScnDebugRenderComponent::pImpl()->drawEllipsoid( 
		getParentEntity()->getWorldPosition(),
		MaVec3d( 1.0f, 1.0f, 1.0f ) * Radius_,
		RsColour::GREEN,
		0 );

	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWeaponComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWeaponComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}
