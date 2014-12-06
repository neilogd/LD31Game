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
#include "System/Os/OsCore.h"

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


	// Render some huddy stuff.
	OsClient* Client = OsCore::pImpl()->getClient( 0 );
	BcF32 Width = 1280;//BcF32( Client->getWidth() ) * 0.5f;
	BcF32 Height = 720;//BcF32( Client->getHeight() ) * 0.5f;
	MaMat4d Projection;
	Projection.orthoProjection( -Width, Width, Height, -Height, -1.0f, 1.0f );
	Canvas_->pushMatrix( Projection );

	MaVec2d MousePos( MouseMoveEvent_.NormalisedX_ * Width, MouseMoveEvent_.NormalisedY_ * Height );
	Canvas_->setMaterialComponent( Material_ );
	Canvas_->drawSpriteCentered( MousePos, MaVec2d( 32, 32 ), 0, RsColour( 1.0f, 1.0f, 1.0f, 1.0f ), 100 );

	MaVec2d Extents( 512.0f, 64.0f );

	MaMat4d PanelOffset;

	//PanelOffset.translation( MaVec3d( -( Width - Extents.x() ), -Height * 0.9f, 0.0f ) );
	Canvas_->pushMatrix( PanelOffset );

	// Draw background.	
	{
		MaVec2d Position( 0.0f, 0.0f );
		
		for( BcU32 Idx = 0; Idx < Program_.size(); ++Idx )
		{
			// Background.
			MaVec2d TLCorner = -Extents + Position;
			MaVec2d BRCorner = Extents + Position;

			Canvas_->drawSprite( TLCorner, Extents, 2, RsColour( 1.0f, 1.0f, 1.0f, 0.6f ), 10 );

			// Foreground.
			//Canvas_->drawSprite( -Extents + Position, Extents + Position, RsColour( 1.0f, 1.0f, 1.0f, 0.6f ), 10 );

			Position += MaVec2d( 0.0f, Extents.y()  );
		}
	}

	{
		MaVec2d Position( 0.0f, 0.0f );
		Font_->setAlphaTestStepping( MaVec2d( 0.4f, 0.42f ) );
		for( BcU32 Idx = 0; Idx < Program_.size(); ++Idx )
		{
			auto& Op = Program_[ Idx ];
			BcChar Buffer[1024];
			BcSPrintf( Buffer, "%u: %s(%u) ? %s(%u)", 
				Op.State_,
				Op.Condition_.c_str(),
				Op.ConditionVar_,
				Op.Operation_.c_str(),
				Op.OperationVar_ );

			Font_->draw( Canvas_, Position - Extents, Extents.y(), Buffer, RsColour::RED, BcFalse, 11 );
			Position += MaVec2d( 0.0f, Extents.y() );
		}
	}

	Canvas_->popMatrix();

	// Handle mouse event.
	for( auto& MouseEvent : MouseEvents_ )
	{
		MouseEvents_.clear();
	}


	Canvas_->popMatrix();
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWorldComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >();
	Material_ = Parent->getComponentAnyParent( "GuiMaterialComponent_0" );
	View_ = ScnCore::pImpl()->findEntity( "CameraEntity_0" )->getComponentByType< ScnViewComponent >();
	Font_ = Parent->getComponentAnyParentByType< ScnFontComponent >();


	BcU32 Idx = 0;

	auto spawnRobot = [ this, &Idx ]( MaVec3d Position, MaVec3d Rotation )
	{
		ScnEntitySpawnParams EntityParams = 
		{
			"default", BcName( "RobotEntity", Idx % 2 ), BcName( "RobotEntity", Idx ),
			MaMat4d(),
			getParentEntity()
		};

		EntityParams.Transform_.rotation( Rotation );
		EntityParams.Transform_.translation( Position );
		auto Entity = ScnCore::pImpl()->spawnEntity( EntityParams );
		auto RobotComponent = Entity->getComponentByType< GaRobotComponent >();
		if( RobotComponent != nullptr && ( Idx % 2 ) == 0 )
		{
			// Test program.
			Program_.push_back( GaRobotOperation( 0, "cond_far_enemy", 8, "op_target_enemy", 8 ) );
			Program_.push_back( GaRobotOperation( 0, "cond_energy_greater", 25, "op_set_state", 2 ) );
			Program_.push_back( GaRobotOperation( 0, "cond_far_start", 24, "op_set_state", 1 ) );
			Program_.push_back( GaRobotOperation( 0, "cond_always", 2, "op_avoid_attack", 32 ) );
			Program_.push_back( GaRobotOperation( 1, "cond_always", 0, "op_target_start", 0 ) );
			Program_.push_back( GaRobotOperation( 1, "cond_near_start", 2, "op_set_state", 0 ) );
			Program_.push_back( GaRobotOperation( 1, "cond_always", 2, "op_avoid_attack", 32 ) );
			Program_.push_back( GaRobotOperation( 2, "cond_always", 0, "op_avoid_attack", 32 ) );
			Program_.push_back( GaRobotOperation( 2, "cond_always", 0, "op_attack_a", 1 ) );
			Program_.push_back( GaRobotOperation( 2, "cond_energy_less", 5, "op_set_state", 0 ) );

			RobotComponent->setProgram( Program_ );
		}

		++Idx;
	};

	spawnRobot( MaVec3d( -32.0f, 0.0f, 0.0f ), MaVec3d( 0.0f, 0.0f, 0.0f ) );
	spawnRobot( MaVec3d( 32.0f, 0.0f, 0.0f ), MaVec3d( 0.0f, BcPI, 0.0f ) );

	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaWorldComponent, &GaWorldComponent::onMouseDown >( this );
	OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaWorldComponent, &GaWorldComponent::onMouseMove >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );	
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );	
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
	OsCore::pImpl()->unsubscribeAll( this );
}

//////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaWorldComponent::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	OsEventInputMouse NewEvent = Event;

	MouseEvents_.push_back( NewEvent );

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onMouseMove
eEvtReturn GaWorldComponent::onMouseMove( EvtID ID, const OsEventInputMouse& Event )
{
	MouseMoveEvent_ = Event;

	return evtRET_PASS;
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
