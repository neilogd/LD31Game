/**************************************************************************
*
* File:		MainGame.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main Game Entrypoint.
*		
*
*
* 
**************************************************************************/

#include "Psybrus.h"

#include "GaCameraComponent.h"
#include "GaAnimationControllerComponent.h"
#include "GaLevelComponent.h"
#include "GaTestModelComponent.h"
#include "GaTestParticleComponent.h"
#include "GaTestShaderComponent.h"
#include "GaTestSelectionComponent.h"

#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Scene/ScnCore.h"

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Development Game", psySF_GAME_DEV, 1.0f / 60.0f );	

//////////////////////////////////////////////////////////////////////////
// PsyGameInit
void PsyGameInit()
{

}

//////////////////////////////////////////////////////////////////////////
// PsyLaunchGame
void PsyLaunchGame()
{
	ScnEntitySpawnParams ScreenEntityParams = 
	{
		"default", "MenuEntity", "MenuEntity_0",
		MaMat4d(),
		nullptr
	};

	ScnCore::pImpl()->spawnEntity( ScreenEntityParams );

	ScnEntitySpawnParams CameraEntityParams = 
	{
		"default", "CameraEntity", "CameraEntity_0",
		MaMat4d(),
		nullptr
	};

	ScnCore::pImpl()->spawnEntity( CameraEntityParams );
}
