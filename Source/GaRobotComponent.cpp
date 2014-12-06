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
#include "GaWeaponComponent.h"
#include "GaWorldComponent.h"

#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnDebugRenderComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "System/Os/OsCore.h"

#include "Base/BcProfiler.h"
#include "Base/BcMath.h"
#include "Base/BcRandom.h"

#include <limits>
#include <cmath>

//////////////////////////////////////////////////////////////////////////
// Program function map.
std::map< std::string, GaRobotComponent::ProgramFunction > GaRobotComponent::ProgramFunctionMap_ =
{
	/**
	 * Condition always: Always perform this operation.
	 * @param Distance to check against. ( < Distance )
	 * @return If we are this near to any enemy.
	 */
	{
		"cond_always",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			return BcTrue;
		}
	},

	/**
	 * Condition Near: Are we near an enemy?
	 * @param Distance to check against. ( < Distance )
	 * @return If we are this near to any enemy.
	 */
	{
		"cond_near_enemy",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			auto Robots = ThisRobot->getRobots( 1 - ThisRobot->Team_ );
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();

			BcBool RetVal = BcFalse;
			for( auto Robot : Robots )
			{
				auto RobotPosition = Robot->getParentEntity()->getLocalPosition();
				if( ( RobotPosition - LocalPosition ).magnitudeSquared() < BcF32( Distance * Distance ) )
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
		"cond_far_enemy",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			auto Robots = ThisRobot->getRobots( 1 - ThisRobot->Team_ );
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();

			BcBool RetVal = BcFalse;
			for( auto Robot : Robots )
			{
				auto RobotPosition = Robot->getParentEntity()->getLocalPosition();
				if( ( RobotPosition - LocalPosition ).magnitudeSquared() < BcF32( Distance * Distance ) )
				{
					RetVal = BcTrue;
					break;
				}
			}

			return !RetVal;
		}
	},

	/**
	 * Condition Near: Are we near start?
	 * @param Distance to check against. ( < Distance )
	 * @return If we are this near to our start.
	 */
	{
		"cond_near_start",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();

			BcBool RetVal = BcFalse;
			if( ( ThisRobot->StartPosition_ - LocalPosition ).magnitudeSquared() < BcF32( Distance * Distance ) )
			{
				RetVal = BcTrue;
			}

			return RetVal;
		}
	},

	/**
	 * Condition Far: Are we far start?
	 * @param Distance to check against. ( > Distance )
	 * @return If we are this far to our start.
	 */
	{
		"cond_far_start",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();

			BcBool RetVal = BcFalse;
			if( ( ThisRobot->StartPosition_ - LocalPosition ).magnitudeSquared() < BcF32( Distance * Distance ) )
			{
				RetVal = BcTrue;
			}

			return !RetVal;
		}
	},

	/**
	 * Condition: Is attack incoming?
	 * Will pick a random position outside of an incoming attack's range.
	 */
	{
		"cond_incoming_attack",
		[]( GaRobotComponent* ThisRobot, BcU32 Radius )->BcU32
		{
			auto Weapons = ThisRobot->getWeapons( 
				ThisRobot->getParentEntity()->getLocalPosition(), BcF32( Radius ) );
			return Weapons.size() > 0;
		}
	},

	/**
	 * Condition: Is health less than?
	 */
	{
		"cond_health_less",
		[]( GaRobotComponent* ThisRobot, BcU32 Health )->BcU32
		{
			return ThisRobot->Health_ < BcF32( Health );
		}
	},

	/**
	 * Condition: Is health greater than?
	 */
	{
		"cond_health_greater",
		[]( GaRobotComponent* ThisRobot, BcU32 Health )->BcU32
		{
			return ThisRobot->Health_ > BcF32( Health );
		}
	},

	/**
	 * Condition: Is health less than?
	 */
	{
		"cond_energy_less",
		[]( GaRobotComponent* ThisRobot, BcU32 Energy )->BcU32
		{
			return ThisRobot->Energy_ < BcF32( Energy );
		}
	},

	/**
	 * Condition: Is health greater than?
	 */
	{
		"cond_energy_greater",
		[]( GaRobotComponent* ThisRobot, BcU32 Energy )->BcU32
		{
			return ThisRobot->Energy_ > BcF32( Energy );
		}
	},

	/**
	 * Set state.
	 */
	{
		"op_set_state",
		[]( GaRobotComponent* ThisRobot, BcU32 State )->BcU32
		{
			return State;
		}
	},

	/**
	 * Move: Nearest target. 
	 * Will pick a point between us and nearest enemy robot at specified distance.
	 */
	{
		"op_target_enemy",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
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
			ThisRobot->TargetPosition_ = RobotPosition - ( ( RobotPosition - LocalPosition ).normal() * BcF32( Distance ) );
			return BcErrorCode;
		}
	},

	/**
	 * Move: Start. 
	 * Will pick a point between us and start at specified distance.
	 */
	{
		"op_target_start",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();
			auto StartPosition = ThisRobot->StartPosition_;
			ThisRobot->TargetPosition_ = StartPosition - ( ( StartPosition - LocalPosition ).normal() * BcF32( Distance ) );
			return BcErrorCode;
		}
	},

	/**
	 * Move: Avoid incoming attack.
	 * Will pick a random position outside of an incoming attack's range.
	 */
	{
		"op_avoid_attack",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			auto Weapons = ThisRobot->getWeapons( 
				ThisRobot->getParentEntity()->getLocalPosition(), BcF32( Distance ) );

			if( Weapons.size() == 0 )
			{
				return BcErrorCode;
			}

			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();

#if 0
			// Work out nearest to avoid.
			GaWeaponComponent* NearestWeapon;
			BcF32 NearestDistance  = std::numeric_limits< BcF32 >::max();
			for( auto* Weapon : Weapons )
			{
				auto WeaponPosition = Weapon->getParentEntity()->getLocalPosition();
				BcF32 Distance = ( WeaponPosition - LocalPosition ).magnitude();
				if( Distance < NearestDistance )
				{
					NearestWeapon = Weapon;
				}
			}
			MaVec3d AvoidPosition = NearestWeapon->TargetPosition_;
#else
			MaVec3d AvoidPosition;
			for( auto* Weapon : Weapons )
			{
				auto WeaponPosition = Weapon->getParentEntity()->getLocalPosition();
				AvoidPosition += WeaponPosition;;
			}
			AvoidPosition /= BcF32( Weapons.size() );
#endif
			// Move nearest point away.
			ThisRobot->TargetPosition_ = LocalPosition + 
				( LocalPosition - AvoidPosition ).normal() * Distance;
			return BcErrorCode;
		}
	},

	/**
	 * Attack weapon a.
	 * Radius to spray.
	 */
	{
		"op_attack_a",
		[]( GaRobotComponent* ThisRobot, BcU32 Radius )->BcU32
		{
			ThisRobot->fireWeaponA( Radius );
			return BcErrorCode;
		}
	},

	/**
	 * Attack weapon b.
	 * Radius to spray.
	 */
	{
		"op_attack_b",
		[]( GaRobotComponent* ThisRobot, BcU32 Radius )->BcU32
		{
			ThisRobot->fireWeaponB( Radius );
			return BcErrorCode;
		}
	},
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
		new ReField( "StartPosition_", &GaRobotComponent::StartPosition_ ),
		new ReField( "TargetDistance_", &GaRobotComponent::TargetDistance_ ),
		new ReField( "TargetPosition_", &GaRobotComponent::TargetPosition_ ),
		new ReField( "MaxVelocity_", &GaRobotComponent::MaxVelocity_ ),
		new ReField( "Velocity_", &GaRobotComponent::Velocity_ ),
		new ReField( "Health_", &GaRobotComponent::Health_ ),
		new ReField( "Energy_", &GaRobotComponent::Energy_ ),
		new ReField( "EnergyChargeRate_", &GaRobotComponent::EnergyChargeRate_ ),
		new ReField( "WeaponACoolDown_", &GaRobotComponent::WeaponACoolDown_ ),
		new ReField( "WeaponACost_", &GaRobotComponent::WeaponACost_ ),
		new ReField( "WeaponATimer_", &GaRobotComponent::WeaponATimer_ ),
		new ReField( "WeaponBCoolDown_", &GaRobotComponent::WeaponBCoolDown_ ),
		new ReField( "WeaponBCost_", &GaRobotComponent::WeaponBCost_ ),
		new ReField( "WeaponBTimer_", &GaRobotComponent::WeaponBTimer_ ),
		new ReField( "Program_", &GaRobotComponent::Program_ ),
		new ReField( "CurrentState_", &GaRobotComponent::CurrentState_ ),
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

	Health_ = 100.0f;
	Energy_ = 0.0f;
	EnergyChargeRate_ = 5.0f;

	WeaponACoolDown_ = 0.1f;
	WeaponACost_ = 2.0f;
	WeaponATimer_ = 0.0f;
	WeaponBCoolDown_ = 4.0f;
	WeaponBCost_ = 25.0f;
	WeaponBTimer_ = 0.0f;
	
	// Test program.
	Program_.push_back( GaRobotOperation( 0, "cond_far_enemy", 8, "op_target_enemy", 8 ) );
	Program_.push_back( GaRobotOperation( 0, "cond_energy_greater", 25, "op_set_state", 2 ) );
	Program_.push_back( GaRobotOperation( 0, "cond_far_start", 24, "op_set_state", 1 ) );
	Program_.push_back( GaRobotOperation( 0, "cond_always", 2, "op_avoid_attack", 32 ) );

	Program_.push_back( GaRobotOperation( 1, "cond_always", 0, "op_target_start", 0 ) );
	Program_.push_back( GaRobotOperation( 1, "cond_near_start", 2, "op_set_state", 0 ) );
	Program_.push_back( GaRobotOperation( 1, "cond_always", 2, "op_avoid_attack", 32 ) );

	Program_.push_back( GaRobotOperation( 2, "cond_always", 0, "op_avoid_attack", 32 ) );
	Program_.push_back( GaRobotOperation( 2, "cond_always", 0, "op_attack_a", 2 ) );
	Program_.push_back( GaRobotOperation( 2, "cond_energy_less", 5, "op_set_state", 0 ) );

	CurrentState_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaRobotComponent::update( BcF32 Tick )
{
	// Handle robot program.
	BcBool ExecutedCode = BcFalse;
	for( BcU32 Idx = 0; Idx < Program_.size(); ++Idx )
	{
		const auto& Op = Program_[ Idx ];
		if( Op.State_ == CurrentState_ )
		{
			auto Condition = ProgramFunctionMap_[ Op.Condition_ ];
			if( Condition == nullptr )
			{
				BcPrintf( "No condition \"%s\"\n", Op.Condition_.c_str() );
			}
			else if( Condition( this, Op.ConditionVar_ ) )
			{
				auto Operation = ProgramFunctionMap_[ Op.Operation_ ];
				if( Operation == nullptr )
				{
					BcPrintf( "No operation \"%s\"\n", Op.Operation_.c_str() );
				}
				else
				{
					auto RetVal = Operation( this, Op.OperationVar_ );
					if( RetVal != BcErrorCode )
					{
						CurrentState_ = RetVal;
						break;
					}
				}
			}
			ExecutedCode = BcTrue;
		}
	}

	// Did we fail to run code? If so, reset to state 0.
	if( ExecutedCode == BcFalse )
	{
		CurrentState_ = 0;
	}

	// Grab entity + position.
	auto Entity = getParentEntity();
	auto LocalPosition = Entity->getLocalPosition();

	// Move if we need to move towards our target position.
	if( ( TargetPosition_ - LocalPosition ).magnitudeSquared() > ( TargetDistance_ * TargetDistance_ ) )
	{
		if( MoveTimer_ <= 0.0f )
		{
			Velocity_ =  ( TargetPosition_ - LocalPosition ).normal() * MaxVelocity_;
		}
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

	// Slow down velocity.
	BcF32 SlowDownTick = BcClamp( Tick * 10.0f, 0.0f, 1.0f );
	Velocity_ -= ( Velocity_ * SlowDownTick );

	// Set local position.
	Entity->setLocalPosition( LocalPosition );

	// Handle health + energy.
	Health_ = BcClamp( Health_, 0.0f, 100.0f );
	Energy_ = BcClamp( Energy_ + ( EnergyChargeRate_ * Tick ), 0.0f, 100.0f );

	// Weapon timers.
	WeaponATimer_ = BcMax( WeaponATimer_ - Tick, -1.0f );
	WeaponBTimer_ = BcMax( WeaponBTimer_ - Tick, -1.0f );

	MoveTimer_ = BcMax( MoveTimer_ - Tick, -1.0f );

	// Health/energy bars.
	OsClient* Client = OsCore::pImpl()->getClient( 0 );
	BcF32 Width = BcF32( Client->getWidth() ) * 0.5f;
	BcF32 Height = BcF32( Client->getHeight() ) * 0.5f;
	MaMat4d Projection;
	Projection.orthoProjection( -Width, Width, Height, -Height, -1.0f, 1.0f );
	Canvas_->pushMatrix( Projection );

	Canvas_->setMaterialComponent( Material_ );

	auto ScreenPos = View_->getScreenPosition( getParentEntity()->getWorldPosition() );
	ScreenPos -= MaVec2d( 0.0f, Height / 8.0f );
	auto TLPos = ScreenPos - MaVec2d( Width / 16.0f, Height / 64.0f );
	auto BRPos = ScreenPos + MaVec2d( Width / 16.0f, Height / 64.0f );

	// Draw background.
	Canvas_->drawBox( TLPos, BRPos, RsColour::BLACK, 0 );

	// Draw inner bars.
	TLPos += MaVec2d( 1.0f, 1.0f );
	BRPos -= MaVec2d( 1.0f, 1.0f );

	auto HealthTL = MaVec2d(
		TLPos.x(),
		TLPos.y() );	
	auto HealthBR = MaVec2d( 
		TLPos.x() + ( BRPos.x() - TLPos.x() ) * ( Health_ / 100.0f ),
		( TLPos.y() + BRPos.y() ) * 0.5f );

	auto EnergyTL = MaVec2d(
		TLPos.x(),
		( TLPos.y() + BRPos.y() ) * 0.5f );
	auto EnergyBR = MaVec2d( 
		TLPos.x() + ( BRPos.x() - TLPos.x() ) * ( Energy_ / 100.0f ),
		BRPos.y() );

	Canvas_->drawBox( HealthTL, HealthBR, RsColour::GREEN, 0 );
	Canvas_->drawBox( EnergyTL, EnergyBR, RsColour::BLUE, 0 );

	Canvas_->popMatrix( );

	ScnDebugRenderComponent::pImpl()->drawLine(
		LocalPosition,
		TargetPosition_,
		RsColour::WHITE,
		0 );

	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaRobotComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	StartPosition_ = Parent->getLocalPosition();
	TargetPosition_ = Parent->getLocalPosition();

	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();
	Material_ = Parent->getComponentAnyParent( "DefaultCanvasMaterial_0" );
	View_ = ScnCore::pImpl()->findEntity( "CameraEntity_0" )->getComponentByType< ScnViewComponent >();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaRobotComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// fireWeaponA
void GaRobotComponent::fireWeaponA( BcF32 Radius )
{
	if( WeaponATimer_ < 0.0f && Energy_ > WeaponACost_ )
	{
		Energy_ -= WeaponACost_;
		WeaponATimer_ = WeaponACoolDown_;
		MoveTimer_ = WeaponACoolDown_ * 0.1f;

		// Spawn entity.
		ScnEntitySpawnParams EntityParams = 
		{
			"default", BcName( "WeaponEntity", 0 ), BcName( "WeaponEntity", 0 ),
			getParentEntity()->getLocalMatrix(),
			getParentEntity()->getParentEntity()
		};

		auto Entity = ScnCore::pImpl()->spawnEntity( EntityParams );
		BcAssert( Entity != nullptr );
		auto Robots = getRobots( 1 - Team_ );
		auto WeaponComponent = Entity->getComponentByType< GaWeaponComponent >();
		WeaponComponent->TargetPosition_ = Robots[ 0 ]->getParentEntity()->getLocalPosition();
		// Randomise target position slightly.
		WeaponComponent->TargetPosition_ += MaVec3d( 
			BcRandom::Global.randRealRange( -1.0f, 1.0f ),
			0.0f,
			BcRandom::Global.randRealRange( -1.0f, 1.0f ) ).normal() * Radius;
	}
}

//////////////////////////////////////////////////////////////////////////
// fireWeaponB
void GaRobotComponent::fireWeaponB( BcF32 Radius )
{
	//
}
	
//////////////////////////////////////////////////////////////////////////
// takeDamage
void GaRobotComponent::takeDamage( BcF32 Damage )
{
	Health_ -= Damage;
}

//////////////////////////////////////////////////////////////////////////
// getRobots
std::vector< class GaRobotComponent* > GaRobotComponent::getRobots( BcU32 Team )
{
	auto WorldComponent = getParentEntity()->getComponentAnyParentByType< GaWorldComponent >();
	return std::move( WorldComponent->getRobots( Team ) );
}

//////////////////////////////////////////////////////////////////////////
// getWeapons
std::vector< class GaWeaponComponent* > GaRobotComponent::getWeapons( MaVec3d Position, BcF32 Radius )
{
	auto WorldComponent = getParentEntity()->getComponentAnyParentByType< GaWorldComponent >();
	return std::move( WorldComponent->getWeapons( Position, Radius ) );
}
