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

#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnDebugRenderComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaRobotComponent );

void GaRobotComponent::StaticRegisterClass()
{
	ReRegisterClass< GaRobotComponent, Super >()
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaRobotComponent::initialise( const Json::Value& Object )
{

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaRobotComponent::update( BcF32 Tick )
{
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaRobotComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaRobotComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}
