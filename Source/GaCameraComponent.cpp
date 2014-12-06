/**************************************************************************
*
* File:		GaCameraComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Camera component.
*		
*
*
* 
**************************************************************************/

#include "GaCameraComponent.h"

#include "System/Os/OsCore.h"
#include "System/Scene/ScnSpatialComponent.h"
#include "System/Scene/Rendering/ScnModel.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaCameraComponent );

void GaCameraComponent::StaticRegisterClass()
{	
	ReField* Fields[] = 
	{
		new ReField( "CameraTarget_",		&GaCameraComponent::CameraTarget_ ),
		new ReField( "CameraRotation_",		&GaCameraComponent::CameraRotation_ ),
		new ReField( "CameraDistance_",		&GaCameraComponent::CameraDistance_ ),
		new ReField( "CameraZoom_",			&GaCameraComponent::CameraZoom_ ),
		new ReField( "CameraState_",		&GaCameraComponent::CameraState_ ),
		new ReField( "NextCameraState_",	&GaCameraComponent::NextCameraState_ ),
	};

	ReRegisterClass< GaCameraComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaCameraComponent::initialise()
{
	Super::initialise();

	CameraState_ = STATE_IDLE;
	NextCameraState_ = STATE_IDLE;
	CameraDistance_ = 64.0f;
	CameraZoom_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaCameraComponent::initialise( const Json::Value& Object )
{
	Super::initialise();

	CameraState_ = STATE_IDLE;
	NextCameraState_ = STATE_IDLE;
	CameraDistance_ = 64.0f;
	CameraZoom_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual
void GaCameraComponent::preUpdate( BcF32 Tick )
{
	Super::update( Tick );

	// Update state.
	switch( CameraState_ )
	{
	case STATE_IDLE:
		{
			OsCore::pImpl()->getClient( 0 )->setMouseLock( BcFalse );
		}
		break;

	case STATE_ROTATE:
		{
			OsCore::pImpl()->getClient( 0 )->setMouseLock( BcTrue );
			BcF32 RotateSpeed = 0.25f;
			CameraRotation_ += MaVec3d( LastMouseEvent_.MouseDY_, LastMouseEvent_.MouseDX_, 0.0f ) * RotateSpeed * Tick;
		}
		break;

	case STATE_PAN:
		{
			OsCore::pImpl()->getClient( 0 )->setMouseLock( BcTrue );

			BcF32 PanSpeed = 4.0f;
			MaMat4d CameraRotationMatrix = getCameraRotationMatrix();
			MaVec3d OffsetVector = MaVec3d( LastMouseEvent_.MouseDX_, LastMouseEvent_.MouseDY_, 0.0f ) * CameraRotationMatrix;
			CameraTarget_ += OffsetVector * Tick * PanSpeed;
		}
		break;
	}

	// Keyboard rotation.
	CameraRotation_ += CameraRotationDelta_ * Tick;

	CameraDistance_ += CameraZoom_ * Tick;
	CameraDistance_ = BcClamp( CameraDistance_, 1.0f, 4096.0f );
	CameraZoom_ = 0.0f;

	MaMat4d Matrix;
	MaVec3d ViewDistance = MaVec3d( 0.0f, 0.0f, CameraDistance_ );
	MaMat4d CameraRotationMatrix = getCameraRotationMatrix();
	ViewDistance = ViewDistance * CameraRotationMatrix;
	MaVec3d ViewFromPosition = CameraTarget_ + ViewDistance;

	Matrix.lookAt( ViewFromPosition, CameraTarget_, MaVec3d( CameraRotationMatrix.row1().x(), CameraRotationMatrix.row1().y(), CameraRotationMatrix.row1().z() ) );
	Matrix.inverse();

#if 1
	MaAABB AABB;
	for( BcU32 Idx = 0; ; ++Idx )
	{
		auto Entity = ScnCore::pImpl()->findEntity( BcName( "RobotEntity", Idx ) );
		if( Entity != nullptr )
		{
			auto SpatialComponent = Entity->getComponentByType< ScnModelComponent >();
			if( SpatialComponent != nullptr )
			{
				AABB.expandBy( SpatialComponent->getAABB() );
			}
		}
		else
		{
			break;
		}
	}

	// Scale up AABB to maximum axis.
	MaVec3d Centre = AABB.centre();
	MaVec3d Dimensions = AABB.dimensions();
	BcF32 MaxDimensions = std::max( std::max( Dimensions.x(), Dimensions.y() ), Dimensions.z() );
	Dimensions = MaVec3d( MaxDimensions, MaxDimensions, MaxDimensions );
	AABB = MaAABB( Centre - Dimensions * 0.75f, Centre + Dimensions * 0.75f );

	MaVec3d TargetPosition = AABB.faceCentre( MaAABB::FRONT ) + MaVec3d( 0.0f, MaxDimensions * 0.5f, 0.0f ); 

	TargetLookAt_ =
		( TargetLookAt_ * 0.95f ) +
		( AABB.centre() * 0.05f );

	TargetPosition_ =
		( TargetPosition_ * 0.95f ) +
		( TargetPosition * 0.05f );

	Matrix.lookAt( TargetPosition_, TargetLookAt_, MaVec3d( CameraRotationMatrix.row1().x(), CameraRotationMatrix.row1().y(), CameraRotationMatrix.row1().z() ) );
	Matrix.inverse();

#endif

	//Matrix.rotation( MaVec3d( BcPIDIV2 - ( BcPI / 16.0f ), 0.0f, 0.0f ) );
	//Matrix.translation( MaVec3d( 0.0f, -4.0f, -2.0f ) );
	getParentEntity()->setLocalMatrix( Matrix );

	CameraState_ = NextCameraState_;

	// clear event.
	BcMemZero( &LastMouseEvent_, sizeof( LastMouseEvent_ ) );

}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaCameraComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );


	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaCameraComponent, &GaCameraComponent::onMouseDown >( this );
	OsEventInputMouse::Delegate OnMouseUp = OsEventInputMouse::Delegate::bind< GaCameraComponent, &GaCameraComponent::onMouseUp >( this );
	OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaCameraComponent, &GaCameraComponent::onMouseMove >( this );
	OsEventInputMouse::Delegate OnMouseWheel = OsEventInputMouse::Delegate::bind< GaCameraComponent, &GaCameraComponent::onMouseWheel >( this );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEWHEEL, OnMouseWheel );

	OsEventInputKeyboard::Delegate OnKeyboardDown = OsEventInputKeyboard::Delegate::bind< GaCameraComponent, &GaCameraComponent::onKeyDown >( this );
	OsEventInputKeyboard::Delegate OnKeyboardUp = OsEventInputKeyboard::Delegate::bind< GaCameraComponent, &GaCameraComponent::onKeyUp >( this );

	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyboardDown );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyboardUp );

}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaCameraComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	OsCore::pImpl()->unsubscribeAll( this );
}

//////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaCameraComponent::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	if( NextCameraState_ == STATE_IDLE )
	{
		if( Event.ButtonCode_ == 2 )
		{
			NextCameraState_ = STATE_ROTATE;
		}
		else if( Event.ButtonCode_ == 1 )
		{
			NextCameraState_ = STATE_PAN;
		}

		LastMouseEvent_ = Event;
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onMouseUp
eEvtReturn GaCameraComponent::onMouseUp( EvtID ID, const OsEventInputMouse& Event )
{
	if( NextCameraState_ != STATE_IDLE )
	{
		NextCameraState_ = STATE_IDLE;
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onMouseMove
eEvtReturn GaCameraComponent::onMouseMove( EvtID ID, const OsEventInputMouse& Event )
{
	LastMouseEvent_ = Event;

	return evtRET_PASS;
}
//////////////////////////////////////////////////////////////////////////
// onMouseWheel
eEvtReturn GaCameraComponent::onMouseWheel( EvtID ID, const OsEventInputMouse& Event )
{
	if( Event.ButtonCode_ == 3 )
	{
		CameraZoom_ -= ( CameraDistance_ * CameraDistance_ ) * 0.02f;
	}
	else if( Event.ButtonCode_ == 4 )
	{
		CameraZoom_ += ( CameraDistance_ * CameraDistance_ ) * 0.02f;
	}

	CameraZoom_ = BcClamp( CameraZoom_, -4096.0f, 4096.0f );

	LastMouseEvent_ = Event;
	
	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onKeyDown
eEvtReturn GaCameraComponent::onKeyDown( EvtID ID, const OsEventInputKeyboard& Event )
{
	LastKeyboardEvent_ = Event;

	switch( Event.KeyCode_ )
	{
	case OsEventInputKeyboard::KEYCODE_LEFT:
		CameraRotationDelta_.y( -1.0f );
		break;
	case OsEventInputKeyboard::KEYCODE_RIGHT:
		CameraRotationDelta_.y(  1.0f );
		break;
	case OsEventInputKeyboard::KEYCODE_UP:
		CameraRotationDelta_.x( -1.0f );
		break;
	case OsEventInputKeyboard::KEYCODE_DOWN:
		CameraRotationDelta_.x(  1.0f );
		break;
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onKeyUp
eEvtReturn GaCameraComponent::onKeyUp( EvtID ID, const OsEventInputKeyboard& Event )
{
	LastKeyboardEvent_ = Event;

	switch( Event.KeyCode_ )
	{
	case OsEventInputKeyboard::KEYCODE_LEFT:
	case OsEventInputKeyboard::KEYCODE_RIGHT:
		CameraRotationDelta_.y( 0.0f );
		break;
	case OsEventInputKeyboard::KEYCODE_UP:
	case OsEventInputKeyboard::KEYCODE_DOWN:
		CameraRotationDelta_.x( 0.0f );
		break;
	}

	return evtRET_PASS;
}

	
//////////////////////////////////////////////////////////////////////////
// getCameraRotationMatrix
MaMat4d GaCameraComponent::getCameraRotationMatrix() const
{
	MaMat4d CameraPitchMatrix;
	MaMat4d CameraYawMatrix;
	CameraPitchMatrix.rotation( MaVec3d( CameraRotation_.x(), 0.0f, 0.0f ) );
	CameraYawMatrix.rotation( MaVec3d( 0.0f, CameraRotation_.y(), 0.0f ) );
	return CameraPitchMatrix * CameraYawMatrix;
}