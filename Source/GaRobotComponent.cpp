/**************************************************************************
*
* File:		GaRobotComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaRobotComponent.h"
#include "GaWorldComponent.h"

#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnDebugRenderComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "Base/BcProfiler.h"

#include <limits>
#include <cmath>

//////////////////////////////////////////////////////////////////////////
// Program function map.
std::map< std::string, GaRobotComponent::ProgramFunction > GaRobotComponent::ProgramFunctionMap_ =
{
	/**
	 * Condition Near: Are we near an enemy?
	 * @param Distance to check against. ( < Distance )
	 * @return If we are this near to any enemy.
	 */
	{
		"near",
		[]( GaRobotComponent* ThisRobot, BcF32 Distance )->BcBool
		{
			auto Robots = ThisRobot->getRobots( 1 - ThisRobot->Team_ );
			BcF32 DistSquared = std::numeric_limits< BcF32 >::max();
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();

			BcBool RetVal = BcFalse;
			for( auto Robot : Robots )
			{
				auto RobotPosition = Robot->getParentEntity()->getLocalPosition();
				if( ( RobotPosition - LocalPosition ).magnitudeSquared() < ( Distance * Distance ) )
				{
					RetVal = BcTrue;
				}
			}

			return RetVal;
		}
	},

	/**
	 * Condition Far: Are we far from an enemy?
	 * @param Distance to check against. ( > Distance )
	 * @return If we are farther away from all enenmies than specified distance.
	 */
	{
		"far",
		[]( GaRobotComponent* ThisRobot, BcF32 Distance )->BcBool
		{
			auto Robots = ThisRobot->getRobots( 1 - ThisRobot->Team_ );
			BcF32 DistSquared = std::numeric_limits< BcF32 >::max();
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();

			BcBool RetVal = BcTrue;
			for( auto Robot : Robots )
			{
				auto RobotPosition = Robot->getParentEntity()->getLocalPosition();
				if( ( RobotPosition - LocalPosition ).magnitudeSquared() < ( Distance * Distance ) )
				{
					RetVal = BcFalse;
					break;
				}
			}

			return RetVal;
		}
	},

	/**
	 * Move: Nearest target. 
	 * Will pick a point between us and nearest enemy robot at specified distance.
	 */
	{
		"move",
		[]( GaRobotComponent* ThisRobot, BcF32 Distance )->BcBool
		{
			auto Robots = ThisRobot->getRobots( 1 - ThisRobot->Team_ );
			BcF32 NearestDistSquared = std::numeric_limits< BcF32 >::max();
			GaRobotComponentRef NearestRobot = nullptr;
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();
			for( auto Robot : Robots )
			{
				auto RobotPosition = Robot->getParentEntity()->getLocalPosition();
				auto DistanceSquared = ( RobotPosition - LocalPosition ).magnitudeSquared();
				if( DistanceSquared < NearestDistSquared )
				{
					NearestDistSquared = DistanceSquared;
					NearestRobot = Robot;
				}
			}

			BcAssert( NearestRobot != nullptr );

			auto RobotPosition = NearestRobot->getParentEntity()->getLocalPosition();
			ThisRobot->TargetPosition_ = RobotPosition - ( ( RobotPosition - LocalPosition ).normal() * Distance );
			return BcTrue;
		}
	}
};


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaRobotComponent );
REFLECTION_DEFINE_BASIC( GaRobotOperation );

void GaRobotComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Team_", &GaRobotComponent::Team_ ),
		new ReField( "TargetDistance_", &GaRobotComponent::TargetDistance_ ),
		new ReField( "TargetPosition_", &GaRobotComponent::TargetPosition_ ),
		new ReField( "MaxVelocity_", &GaRobotComponent::MaxVelocity_ ),
		new ReField( "Velocity_", &GaRobotComponent::Velocity_ ),
		new ReField( "Program_", &GaRobotComponent::Program_ ),
	};
	
	ReRegisterClass< GaRobotComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

void GaRobotOperation::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Condition_", &GaRobotOperation::Condition_ ),
		new ReField( "ConditionVar_", &GaRobotOperation::ConditionVar_ ),
		new ReField( "Operation_", &GaRobotOperation::Operation_ ),
		new ReField( "OperationVar_", &GaRobotOperation::OperationVar_ ),
	};
	
	ReRegisterClass< GaRobotOperation >( Fields );
}

//////////////////////////////////////////////////////////////////////////b
// initialise
void GaRobotComponent::initialise( const Json::Value& Object )
{
	Team_ = Object[ "team" ].asUInt();

	TargetDistance_ = 1.0f;
	TargetPosition_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	MaxVelocity_ = 4.0f;

	// Test program!
	// If we are further than 8 units, move to within 8 units of it.
	Program_.push_back( GaRobotOperation( "far", 8.0f, "move", 8.0f ) );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaRobotComponent::update( BcF32 Tick )
{
	// Handle robot program.
	for( auto& Op : Program_ )
	{
		auto Condition = ProgramFunctionMap_[ Op.Condition_ ];
		if( Condition( this, Op.ConditionVar_ ) )
		{
			auto Operation = ProgramFunctionMap_[ Op.Operation_ ];
			Operation( this, Op.OperationVar_ );
		}
	}

	// Grab entity + position.
	auto Entity = getParentEntity();
	auto LocalPosition = Entity->getLocalPosition();

	// Move if we need to move towards our target position.
	if( ( TargetPosition_ - LocalPosition ).magnitudeSquared() > ( TargetDistance_ * TargetDistance_ ) )
	{
		Velocity_ = ( TargetPosition_ - LocalPosition ).normal() * MaxVelocity_;
	}
	else
	{
		Velocity_ = MaVec3d( 0.0f, 0.0f, 0.0f );
	}

	LocalPosition += Velocity_ * Tick;

	// TODO LATER: Do rotation.
	if( Velocity_.magnitudeSquared() > 0.0f )
	{
		std::atan2( Velocity_.z(), Velocity_.x() );
	}

	// Set local position.
	Entity->setLocalPosition( LocalPosition );


	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaRobotComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	//TargetPosition_ = Parent->getLocalPosition();

}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaRobotComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getRobots
std::vector< GaRobotComponentRef > GaRobotComponent::getRobots( BcU32 Team )
{
	std::vector< GaRobotComponentRef > Robots;

	// TODO: Cache it or something?

	auto WorldComponent = getParentEntity()->getComponentAnyParentByType< GaWorldComponent >();
	auto WorldEntity = WorldComponent->getParentEntity();

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
