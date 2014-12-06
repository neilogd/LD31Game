/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Camera component.
*		
*
*
* 
**************************************************************************/

#ifndef __GACAMERACOMPONENT_H__
#define __GACAMERACOMPONENT_H__

#include "Psybrus.h"

#include "System/Os/OsEvents.h"
#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef ReObjectRef< class GaCameraComponent > GaCameraComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
class GaCameraComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( GaCameraComponent, ScnComponent );

	void								initialise();
	void								initialise( const Json::Value& Object );

	virtual void						preUpdate( BcF32 Tick );

	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	eEvtReturn							onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn							onMouseUp( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn							onMouseMove( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn							onMouseWheel( EvtID ID, const OsEventInputMouse& Event );

	eEvtReturn							onKeyDown( EvtID ID, const OsEventInputKeyboard& Event );
	eEvtReturn							onKeyUp( EvtID ID, const OsEventInputKeyboard& Event );

	MaMat4d								getCameraRotationMatrix() const;
	
private:
	MaVec3d								CameraTarget_;
	MaVec3d								CameraRotation_;
	BcF32								CameraDistance_;
	BcF32								CameraZoom_;

	MaVec3d								CameraRotationDelta_;

	enum CameraState
	{
		STATE_IDLE = 0,
		STATE_ROTATE,
		STATE_PAN
	};

	CameraState							CameraState_;
	CameraState							NextCameraState_;
	OsEventInputMouse					LastMouseEvent_;
	OsEventInputKeyboard				LastKeyboardEvent_;
};

#endif

