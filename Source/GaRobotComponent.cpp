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
	 * Condition never: Never perform this operation.
	 */
	{
		"cond_never",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			return BcFalse;
		}
	},

	/**
	 * Condition always: Always perform this operation.
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
	 * Will pick a point around nearest enemy robot at specified distance.
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

			if( NearestRobot != nullptr )
			{
				auto RobotPosition = NearestRobot->getParentEntity()->getLocalPosition();

				BcF32 RandomDelta = ThisRobot->MoveAngle_;
				ThisRobot->MoveAngle_ += BcPIDIV4;
				MaVec3d Offset( BcCos( RandomDelta ), 0.0f, -BcSin( RandomDelta ) );

				ThisRobot->TargetPosition_ = RobotPosition - ( Offset * BcF32( Distance ) );
			}
			return BcErrorCode;
		}
	},

	/**
	 * Move: Start. 
	 * Will pick a point around start at specified distance.
	 */
	{
		"op_target_start",
		[]( GaRobotComponent* ThisRobot, BcU32 Distance )->BcU32
		{
			auto LocalPosition = ThisRobot->getParentEntity()->getLocalPosition();
			auto StartPosition = ThisRobot->StartPosition_;

			BcF32 RandomDelta = ThisRobot->MoveAngle_;
			ThisRobot->MoveAngle_ += BcPIDIV4;
			MaVec3d Offset( BcCos( RandomDelta ), 0.0f, -BcSin( RandomDelta ) );

			ThisRobot->TargetPosition_ = StartPosition - ( Offset * BcF32( Distance ) );
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
REFLECTION_DEFINE_BASIC( GaRobotCommandEntry );
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
		new ReField( "MoveTimer_", &GaRobotComponent::MoveTimer_ ),
		new ReField( "Program_", &GaRobotComponent::Program_ ),
		new ReField( "CurrentState_", &GaRobotComponent::CurrentState_ ),
		new ReField( "CurrentOp_", &GaRobotComponent::CurrentOp_ ),
		new ReField( "NextOp_", &GaRobotComponent::NextOp_ ),
		new ReField( "CurrentOpTimer_", &GaRobotComponent::CurrentOpTimer_ ),
		new ReField( "CurrentOpTime_", &GaRobotComponent::CurrentOpTime_ ),
		new ReField( "MoveAngle_", &GaRobotComponent::MoveAngle_ ),
	};
	
	ReRegisterClass< GaRobotComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( 1 ) );
}

void GaRobotCommandEntry::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Name_", &GaRobotCommandEntry::Name_ ),
		new ReField( "Shorthand_", &GaRobotCommandEntry::Shorthand_ ),
		new ReField( "Doc_", &GaRobotCommandEntry::Doc_ ),
		new ReField( "VarOptions_", &GaRobotCommandEntry::VarOptions_ ),
	};
	
	ReRegisterClass< GaRobotCommandEntry >( Fields );
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
	WeaponACost_ = 10.0f;
	WeaponATimer_ = 0.0f;
	WeaponBCoolDown_ = 4.0f;
	WeaponBCost_ = 50.0f;
	WeaponBTimer_ = 0.0f;
	MoveTimer_ = 0.0f;
	
	CurrentState_ = 0;
	CurrentOp_ = 0;
	NextOp_ = 0;
	CurrentOpTimer_ = 0.0f;
	CurrentOpTime_ = 0.1f;

	MoveAngle_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaRobotComponent::update( BcF32 Tick )
{
	if( Health_ <= 0.0f )
	{
		return;
	}

	CurrentOpTimer_ -= Tick;
	if( CurrentOpTimer_ < 0.0f )
	{
		CurrentOpTimer_ += CurrentOpTime_;

		// Handle robot program.
		BcBool ExecutedCode = BcFalse;
		if( Program_.size() > 0 )
		{
			CurrentOp_ = NextOp_;
			const auto& Op = Program_[ CurrentOp_ ];
			if( Op.State_ == CurrentState_ )
			{
				auto Condition = ProgramFunctionMap_[ Op.Condition_ ];
				if( Condition != nullptr )
				{
					if( Condition( this, Op.ConditionVar_ ) )
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
							}
						}
					}
				}
				ExecutedCode = BcTrue;
			}

			// Advance to next valid op.
			if( ExecutedCode )
			{
				for( BcU32 Idx = 0; Idx < Program_.size(); ++Idx )
				{
					NextOp_ = ( NextOp_ + 1 ) % Program_.size();
					if( Program_[ NextOp_ ].State_ == CurrentState_ )
					{
						break;
					}
				}
			}

			// Did we fail to run code? If so, reset to op 0 and the state of op 0.
			if( ExecutedCode == BcFalse )
			{
				NextOp_ = 0;
				CurrentState_ = Program_[ NextOp_ ].State_;
			}
		}
	}

	// Grab entity + position.
	auto Entity = getParentEntity();
	auto LocalPosition = Entity->getLocalPosition();

	// Move if we need to move towards our target position.
	if( ( TargetPosition_ - LocalPosition ).magnitudeSquared() > ( TargetDistance_ * TargetDistance_ ) )
	{
		if( MoveTimer_ <= 0.0f )
		{
			Velocity_ +=  ( TargetPosition_ - LocalPosition ).normal() * MaxVelocity_;
		}
	}
	else
	{
		BcF32 SlowDownTick = BcClamp( Tick * 50.0f, 0.0f, 1.0f );
		Velocity_ -= ( Velocity_ * SlowDownTick );
	}

	// TODO LATER: Do rotation.
	if( Velocity_.magnitudeSquared() > 0.1f )
	{
		auto Angle = std::atan2( Velocity_.z(), Velocity_.x() ) + BcPIDIV2;

		MaMat4d RotMat;
		RotMat.rotation( MaVec3d( 0.0f, Angle, 0.0f ) );
		Base_->setLocalMatrix( RotMat );
	}

	// TODO LATER: Do rotation.
	auto Robots = getRobots( 1 - Team_ );
	if( Robots.size() > 0 )
	{
		auto Robot = Robots[ 0 ];
		auto RobotPosition = Robot->getParentEntity()->getLocalPosition();
		auto VectorTo = RobotPosition - LocalPosition;

		// Push out of away.
		if( VectorTo.magnitude() < 3.0f )
		{
			BcF32 Factor = ( 3.0f - VectorTo.magnitude() ) / 3.0f;
			BcF32 InvFactor = 1.0f - Factor;

			Velocity_ = ( -( VectorTo.normal() * MaxVelocity_ ) * Factor * 3.0f ) + ( Velocity_ * InvFactor );
		}

		// Face turret.
		auto Angle = std::atan2( VectorTo.z(), VectorTo.x() ) + BcPIDIV2;

		MaMat4d RotMat;
		RotMat.rotation( MaVec3d( 0.0f, Angle, 0.0f ) );
		Turret_->setLocalMatrix( RotMat );
	}

	LocalPosition += Velocity_ * Tick;

	// Slow down velocity.
	BcF32 SlowDownTick = BcClamp( Tick * 10.0f, 0.0f, 1.0f );
	Velocity_ -= ( Velocity_ * SlowDownTick );

	if( Velocity_.magnitude() > MaxVelocity_ )
	{
		Velocity_ = Velocity_.normal() * MaxVelocity_;
	}

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

	auto spawnPart = [ & ]( const BcName PartName )
	{
		ScnEntitySpawnParams EntityParams = 
		{
			"default", PartName, BcName( PartName.getValue(), getParentEntity()->getName().getID() ),
			MaMat4d(),
			getParentEntity()
		};
		return ScnCore::pImpl()->spawnEntity( EntityParams );
	};

	Base_ = spawnPart( "RobotBase" );
	Turret_ = spawnPart( "RobotTurret" );

	MoveAngle_ = getName().getID() == 0 ? 0.0f : BcPI;
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
		auto Robots = getRobots( 1 - Team_ );
		if( Robots.size() > 0 )
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
			auto WeaponComponent = Entity->getComponentByType< GaWeaponComponent >();
			WeaponComponent->TargetPosition_ = Robots[ 0 ]->getParentEntity()->getLocalPosition();
			// Randomise target position slightly.
			WeaponComponent->TargetPosition_ += MaVec3d( 
				BcRandom::Global.randRealRange( -1.0f, 1.0f ),
				0.0f,
				BcRandom::Global.randRealRange( -1.0f, 1.0f ) ).normal() * Radius;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// fireWeaponB
void GaRobotComponent::fireWeaponB( BcF32 Radius )
{
	if( WeaponATimer_ < 0.0f && Energy_ > WeaponACost_ )
	{
		auto Robots = getRobots( 1 - Team_ );
		if( Robots.size() > 0 )
		{
			Energy_ -= WeaponBCost_;
			WeaponBTimer_ = WeaponBCoolDown_;
			MoveTimer_ = WeaponBCoolDown_ * 0.1f;

			// Spawn entity.
			ScnEntitySpawnParams EntityParams = 
			{
				"default", BcName( "WeaponEntity", 1 ), BcName( "WeaponEntity", 1 ),
				getParentEntity()->getLocalMatrix(),
				getParentEntity()->getParentEntity()
			};

			auto Entity = ScnCore::pImpl()->spawnEntity( EntityParams );
			BcAssert( Entity != nullptr );
			auto WeaponComponent = Entity->getComponentByType< GaWeaponComponent >();
			WeaponComponent->TargetPosition_ = Robots[ 0 ]->getParentEntity()->getLocalPosition();
			// Randomise target position slightly.
			WeaponComponent->TargetPosition_ += MaVec3d( 
				BcRandom::Global.randRealRange( -1.0f, 1.0f ),
				0.0f,
				BcRandom::Global.randRealRange( -1.0f, 1.0f ) ).normal() * Radius;
		}
	}
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

//////////////////////////////////////////////////////////////////////////
// setProgram
void GaRobotComponent::setProgram( std::vector< GaRobotOperation > Program )
{
	Program_ = Program;
	CurrentState_ = 0;
	NextOp_ = 0;
	CurrentOp_ = 0;

	if( Program_.size() > 0 )
	{
		CurrentState_ = Program_[ 0 ].State_;
	}
}
