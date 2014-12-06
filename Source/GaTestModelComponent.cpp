/**************************************************************************
*
* File:		GaTestModelComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaTestModelComponent.h"

#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaTestModelComponent );

void GaTestModelComponent::StaticRegisterClass()
{
	ReRegisterClass< GaTestModelComponent, Super >()
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaTestModelComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Material_ = this->getPackage()->getCrossRefResource( Object[ "material" ].asUInt() );	
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaTestModelComponent::update( BcF32 Tick )
{
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaTestModelComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	BcU32 Idx = 0;
	BcF32 Width = 32.0f;

	for( BcF32 X = -Width; X <= Width; X += 2.0f )
	{
		ScnEntitySpawnParams EntityParams = 
		{
			"default", "ModelEntity", BcName( "ModelEntity", Idx++ ),
			MaMat4d(),
			getParentEntity()
		};

		EntityParams.Transform_.translation( MaVec3d( 0.0f, 0.0f, X ) );
		ScnCore::pImpl()->spawnEntity( EntityParams );
	}

	BcPrintf( "Spawned %u model entities.\n", Idx );

}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaTestModelComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB GaTestModelComponent::getAABB() const
{
	return MaAABB();
}
