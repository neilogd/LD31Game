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

#include <algorithm>

namespace
{
	static std::vector< GaRobotCommandEntry > ConditionEntries_ = 
	{
		GaRobotCommandEntry( "cond_never",				"!",
			"Condition: Never do this.", BcFalse,
			{ } ),
		GaRobotCommandEntry( "cond_always",				"*",	
			"Condition: Always do this.", BcFalse,
			{ } ),
		GaRobotCommandEntry( "cond_near_enemy", 		"E<X",	
			"Condition: Is enemy nearer than (X) units?", BcTrue, 
			{ 1, 2, 4, 8, 16, 32, 64 } ),
		GaRobotCommandEntry( "cond_far_enemy",	 		"E>X",		
			"Condition: Is enemy farther than (X) units?", BcTrue,
			{ 1, 2, 4, 8, 12, 16, 24, 32 } ),
		GaRobotCommandEntry( "cond_near_start",	 		"St<X",		
			"Condition: Is start nearer than (X) units?", BcTrue,
			{ 1, 2, 4, 8, 12, 16, 24, 32 } ),
		GaRobotCommandEntry( "cond_far_start", 			"St>X",		
			"Condition: Is start farther than (X) units?", BcTrue,
			{ 1, 2, 4, 8, 12, 16, 24, 32 } ),
		GaRobotCommandEntry( "cond_incoming_attack", 	"At<X",		
			"Condition: Is an attack incoming within (X) units of us?", BcTrue,
			{ 1, 2, 4, 8, 12, 16, 24, 32 } ),
		GaRobotCommandEntry( "cond_health_less",	 	"He<X",
			"Condition: Is health lower than (X)%?", BcTrue,
			{ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 } ),
		GaRobotCommandEntry( "cond_health_greater", 	"Hw>X",
			"Condition: Is health greater than (X)%?", BcTrue,
			{ 10, 20, 30, 40, 50, 60, 70, 80	, 90, 100 } ),
		GaRobotCommandEntry( "cond_energy_less",	 	"En<X",
			"Condition: Is energy lower than (X)%?", BcTrue,
			{ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 } ),
		GaRobotCommandEntry( "cond_energy_greater", 	"En>X",
			"Condition: Is energy greater than (X)%?", BcTrue,
			{ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 } ),
	};

	static std::vector< GaRobotCommandEntry > OperationEntries_ = 
	{
		GaRobotCommandEntry( "op_set_state", 			"Set",
			"Operation: Set State.", BcTrue,
			{ 0, 1, 2, 3, 4, 5, 6, 7 } ),
		GaRobotCommandEntry( "op_target_enemy", 		"M-X",
			"Operation: Move to within (X) units of enemy.", BcTrue,
			{ 1, 2, 4, 8, 12, 16, 24, 32 } ),
		GaRobotCommandEntry( "op_target_start", 		"M-St",
			"Operation: Move to within (X) units of start.", BcTrue,
			{ 1, 2, 4, 8, 12, 16, 24, 32 } ),
		GaRobotCommandEntry( "op_avoid_attack", 		"AvAtX",
			"Operation: Move to (X) units away from incoming attack.", BcTrue, 
			{ 1, 2, 4, 8, 12, 16, 24, 32 } ),
		GaRobotCommandEntry( "op_attack_a", 			"At-WpA",
			"Operation: Attack enemy with Weapon A, spread of (X) units.", BcTrue,
			{ 0, 1, 2, 4, 8 } ),
		GaRobotCommandEntry( "op_attack_b", 			"At-WpB",
			"Operation: Attack enemy with Weapon B, spread of (X) units.", BcTrue,
			{ 0, 1, 2, 4, 8 } ),
	};
}

OsEventInputMouse GaWorldComponent::MouseMoveEvent_;


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaWorldComponent );

void GaWorldComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "GuiState_", &GaWorldComponent::GuiState_ ),
		new ReField( "SelectionPosition_", &GaWorldComponent::SelectionPosition_ ),
		new ReField( "SelectedID_", &GaWorldComponent::SelectedID_ ),
		new ReField( "HotspotType_", &GaWorldComponent::HotspotType_ ),
		new ReField( "PlayerRobot_", &GaWorldComponent::PlayerRobot_, bcRFF_TRANSIENT ),
		new ReField( "EnemyRobot_", &GaWorldComponent::EnemyRobot_, bcRFF_TRANSIENT ),
	};
	
	ReRegisterClass< GaWorldComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( 0 ) );

	ReEnumConstant* HotspotTypeConstants[] = 
	{
		new ReEnumConstant( "STATE", (BcU32)HotspotType::STATE ),
		new ReEnumConstant( "CONDITION", (BcU32)HotspotType::CONDITION ),
		new ReEnumConstant( "CONDITION_VAR", (BcU32)HotspotType::CONDITION_VAR ),
		new ReEnumConstant( "OPERATION", (BcU32)HotspotType::OPERATION ),
		new ReEnumConstant( "OPERATION_VAR", (BcU32)HotspotType::OPERATION_VAR ),
		new ReEnumConstant( "ADD_OP", (BcU32)HotspotType::ADD_OP ),
		new ReEnumConstant( "DEL_OP", (BcU32)HotspotType::DEL_OP ),
		new ReEnumConstant( "STATE_SELECTION", (BcU32)HotspotType::STATE_SELECTION ),
		new ReEnumConstant( "CONDITION_SELECTION", (BcU32)HotspotType::CONDITION_SELECTION ),
		new ReEnumConstant( "CONDITION_VAR_SELECTION", (BcU32)HotspotType::CONDITION_VAR_SELECTION ),
		new ReEnumConstant( "OPERATION_SELECTION", (BcU32)HotspotType::OPERATION_SELECTION ),
		new ReEnumConstant( "OPERATION_VAR_SELECTION", (BcU32)HotspotType::OPERATION_VAR_SELECTION ),
		new ReEnumConstant( "STATE_MAIN", (BcU32)HotspotType::STATE_MAIN ),
		new ReEnumConstant( "INVALID", (BcU32)HotspotType::INVALID ),
	};
	ReRegisterEnum< HotspotType >( HotspotTypeConstants );


	ReEnumConstant* GuiStateEnumConstants[] = 
	{
		new ReEnumConstant( "MAIN", (BcU32)GuiState::MAIN ),
		new ReEnumConstant( "SELECTION_STATE", (BcU32)GuiState::SELECTION_STATE ),
		new ReEnumConstant( "SELECTION", (BcU32)GuiState::SELECTION ),
		new ReEnumConstant( "SELECTION_VAR", (BcU32)GuiState::SELECTION_VAR ),
	};
	ReRegisterEnum< GuiState >( GuiStateEnumConstants );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaWorldComponent::initialise( const Json::Value& Object )
{
	GuiState_ = GuiState::MAIN;
	SelectedID_ = BcErrorCode;
	HotspotType_ = HotspotType::INVALID;

	PlayerRobot_ = nullptr;
	EnemyRobot_ = nullptr;

	BcMemZero( &MouseMoveEvent_, sizeof( MouseMoveEvent_ ) );
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

	Font_->setAlphaTestStepping( MaVec2d( 0.45f, 0.46f ) );

	Canvas_->clear();

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

	std::vector< Hotspot > Hotspots;

	Hotspot HSStateMain = 
	{
		MaVec2d( -Width, -Height ), MaVec2d( Width, Height ) * 2.0f,
		BcErrorCode,
		HotspotType::STATE_MAIN
	};
	Hotspots.push_back( HSStateMain );

	MaMat4d PanelOffset;

	PanelOffset.translation( MaVec3d( -( Width - Extents.x() ) * 0.8f, -Height * 0.8f, 0.0f ) );
	Canvas_->pushMatrix( PanelOffset );

	// Draw background.	
	{
		MaVec2d Position( 0.0f, 0.0f );
		
		for( BcU32 Idx = 0; Idx < Program_.size(); ++Idx )
		{
			// Background.
			MaVec2d TLCorner = -Extents + Position;
			MaVec2d BRCorner = Extents + Position;

			MaVec2d StateIcon( 64.0f, 64.0f );

			MaVec2d TLStateCorner = TLCorner - MaVec2d( 64.0f, 0.0f );
			MaVec2d TotalExtents = Extents + MaVec2d( 64.0f, 0.0f );

			Canvas_->setMaterialComponent( Material_ );

			static std::array< RsColour, 8 > StateColours =
			{
				RsColour( 1.0f, 0.0f, 0.0f, 1.0f ),// + RsColour::GRAY * 0.5f,
				RsColour( 1.0f, 0.5f, 0.0f, 1.0f ),// + RsColour::GRAY * 0.5f,
				RsColour( 1.0f, 1.0f, 0.0f, 1.0f ),// + RsColour::GRAY * 0.5f,
				RsColour( 0.0f, 1.0f, 0.0f, 1.0f ),// + RsColour::GRAY * 0.5f,
				RsColour( 0.0f, 1.0f, 1.0f, 1.0f ),// + RsColour::GRAY * 0.5f,
				RsColour( 0.0f, 0.0f, 1.0f, 1.0f ),// + RsColour::GRAY * 0.5f,
				RsColour( 0.5f, 0.0f, 1.0f, 1.0f ),// + RsColour::GRAY * 0.5f,
				RsColour( 1.0f, 0.0f, 0.5f, 1.0f ),// + RsColour::GRAY * 0.5f,
			};

			BcAssert( Program_[ Idx ].State_ < 8 );
			RsColour BgColour = StateColours[ Program_[ Idx ].State_ ];

			if( PlayerRobot_ != nullptr )
			{
				if( PlayerRobot_->CurrentState_ != Program_[ Idx ].State_ )
				{
					BgColour = BgColour * 0.5f;
					BgColour.a( 1.0f );
				}
			}

			Canvas_->drawSprite( TLCorner, Extents, 1, BgColour, 10 );
			Canvas_->drawSprite( TLStateCorner, StateIcon, 2, BgColour, 10 );

			if( PlayerRobot_ != nullptr )
			{
				if( PlayerRobot_->CurrentOp_ == Idx )
				{
					Canvas_->drawSpriteCentered( TLStateCorner + TotalExtents * 0.5f, TotalExtents * 1.1f, 0, RsColour( 1.0f, 1.0f, 1.0f, 1.0f ), 9 );
				}

			}

			BcF32 FontSize = 32.0f;
			RsColour Colour = RsColour::BLACK;
			MaVec2d Button( 128.0f, 64.0f );
			BcChar Buffer[ 1024 ];

			/**
			 * STATE HOTSPOT.
			 */
			Hotspot HSState = 
			{
				TLStateCorner * PanelOffset, StateIcon,
				Idx,
				HotspotType::STATE
			};

			BcSPrintf( Buffer, "%u", Program_[ Idx ].State_ );
			Font_->drawCentered( Canvas_, TLStateCorner + StateIcon * 0.5f, FontSize, Buffer, Colour, 12 );

			Hotspots.push_back( HSState );

			/**
			 * CONDITION HOTSPOT.
			 */
			Hotspot HSCondition = 
			{
				TLCorner * PanelOffset, Button,
				Idx,
				HotspotType::CONDITION
			};

			auto FoundCondIt = std::find_if( ConditionEntries_.begin(), ConditionEntries_.end(),
				[ & ]( const GaRobotCommandEntry& Entry )
				{
					return Entry.Name_ == Program_[ Idx ].Condition_;
				} );

			if( FoundCondIt != ConditionEntries_.end() )
			{
				Font_->drawCentered( Canvas_, TLCorner + Button * 0.5f, FontSize, FoundCondIt->Shorthand_, Colour, 12 );
			}

			Hotspots.push_back( HSCondition );


			if( FoundCondIt->Name_ != "cond_never" )
			{
				TLCorner.x( TLCorner.x() + Button.x() );
				if( FoundCondIt != ConditionEntries_.end() &&
					FoundCondIt->HasVar_ )
				{
					Hotspot HSConditionVar = 
					{
						TLCorner * PanelOffset, Button,
						Idx,
						HotspotType::CONDITION_VAR
					};

					BcSPrintf( Buffer, "%u", Program_[ Idx ].ConditionVar_ );
					Font_->drawCentered( Canvas_, TLCorner + Button * 0.5f, FontSize, Buffer, Colour, 12 );
					Hotspots.push_back( HSConditionVar );
				}

				TLCorner.x( TLCorner.x() + Button.x() );
				Hotspot HSOperation = 
				{
					TLCorner * PanelOffset, Button,
					Idx,
					HotspotType::OPERATION
				};

				auto FoundOperationIt = std::find_if( OperationEntries_.begin(), OperationEntries_.end(),
					[ & ]( const GaRobotCommandEntry& Entry )
					{
						return Entry.Name_ == Program_[ Idx ].Operation_;
					} );

				if( FoundOperationIt != OperationEntries_.end() )
				{
					Font_->drawCentered( Canvas_, TLCorner + Button * 0.5f, FontSize, FoundOperationIt->Shorthand_, Colour, 12 );
					Hotspots.push_back( HSOperation );
				}

				TLCorner.x( TLCorner.x() + Button.x() );
				if( FoundOperationIt != OperationEntries_.end() &&
					FoundOperationIt->HasVar_ )
				{
					Hotspot HSOperationVar = 
					{
						TLCorner * PanelOffset, Button,
						Idx,
						HotspotType::OPERATION_VAR
					};

					BcSPrintf( Buffer, "%u", Program_[ Idx ].OperationVar_ );
					Font_->drawCentered( Canvas_, TLCorner + Button * 0.5f, FontSize, Buffer, Colour, 12 );
					Hotspots.push_back( HSOperationVar );
				}

				TLCorner.x( TLCorner.x() + Button.x() );
			}


			// Foreground.
			//Canvas_->drawSprite( -Extents + Position, Extents + Position, RsColour( 1.0f, 1.0f, 1.0f, 0.6f ), 10 );

			Position += MaVec2d( 0.0f, Extents.y()  );
		}
	}

	{
		MaVec2d Position( 0.0f, 0.0f );
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

			//Font_->draw( Canvas_, Position - Extents, Extents.y(), Buffer, RsColour::RED, BcFalse, 11 );
			Position += MaVec2d( 0.0f, Extents.y() );
		}
	}

	Canvas_->popMatrix();

	// Draw panel over the top of what we have already.
	if( GuiState_ == GuiState::SELECTION ||
		GuiState_ == GuiState::SELECTION_STATE ||
		GuiState_ == GuiState::SELECTION_VAR )
	{
		PanelOffset.translation( MaVec3d( SelectionPosition_.x(), SelectionPosition_.y(), 0.0f ) );
		Canvas_->pushMatrix( PanelOffset );

		////////////////////////////////////////////////////
		// Condition + operation.
		if( HotspotType_ == HotspotType::STATE )
		{
			MaVec2d Position( 0.0f, 0.0f );
			MaVec2d TotalSize( 0.0f, 0.0f );

			for( BcU32 Idx = 0; Idx < 8; ++Idx )
			{
				Hotspot HSSelection = 
				{
					Position * PanelOffset, MaVec2d(),
					Idx,
					HotspotType::STATE_SELECTION
				};

				RsColour Colour = RsColour::BLACK;

				if( HSSelection.ID_ == LastHighlightedHotspot_.ID_ &&
					HSSelection.Type_ == LastHighlightedHotspot_.Type_ )
				{
					Colour = RsColour::GREEN;
				}

				BcF32 FontSize = Extents.y() * 0.5f;

				BcChar Buffer[ 1024 ];
				BcSPrintf( Buffer, "%u", Idx );
				auto Size = Font_->draw( Canvas_, Position, FontSize, Buffer, Colour, BcFalse, 21 );
				Position.y( Position.y() + FontSize );

				TotalSize.x( std::max( Size.x(), TotalSize.x() ) );
				TotalSize.y( TotalSize.y() + FontSize );

				HSSelection.Extents_ = Size;

				Hotspots.push_back( HSSelection );
			}

			Canvas_->setMaterialComponent( Material_ );
			Canvas_->drawSprite( MaVec2d( 0.0f, 0.0f ), TotalSize, 0, RsColour( 1.0f, 1.0f, 1.0f, 0.8f ), 20 );
		}

		////////////////////////////////////////////////////
		// Condition + operation.
		if( HotspotType_ == HotspotType::CONDITION ||
			HotspotType_ == HotspotType::OPERATION )
		{
			std::vector< GaRobotCommandEntry > CommandEntries =
				HotspotType_ == HotspotType::CONDITION ? 
					ConditionEntries_ : OperationEntries_;

			MaVec2d Position( 0.0f, 0.0f );
			MaVec2d TotalSize( 0.0f, 0.0f );

			for( BcU32 Idx = 0; Idx < CommandEntries.size(); ++Idx )
			{
				Hotspot HSSelection = 
				{
					Position * PanelOffset, MaVec2d(),
					Idx,
					HotspotType_ == HotspotType::CONDITION ?
						HotspotType::CONDITION_SELECTION : HotspotType::OPERATION_SELECTION
				};

				RsColour Colour = RsColour::BLACK;

				if( HSSelection.ID_ == LastHighlightedHotspot_.ID_ &&
					HSSelection.Type_ == LastHighlightedHotspot_.Type_ )
				{
					Colour = RsColour::GREEN;
				}

				BcF32 FontSize = Extents.y() * 0.5f;

				const auto& Entry = CommandEntries[ Idx ];
				auto Size = Font_->draw( Canvas_, Position, FontSize, Entry.Doc_, Colour, BcFalse, 21 );
				Position.y( Position.y() + FontSize );

				TotalSize.x( std::max( Size.x(), TotalSize.x() ) );
				TotalSize.y( TotalSize.y() + FontSize );

				HSSelection.Extents_ = Size;

				Hotspots.push_back( HSSelection );
			}

			Canvas_->setMaterialComponent( Material_ );
			Canvas_->drawSprite( MaVec2d( 0.0f, 0.0f ), TotalSize, 0, RsColour( 1.0f, 1.0f, 1.0f, 0.8f ), 20 );
		}

		////////////////////////////////////////////////////
		// Condition + operation vars
		if( HotspotType_ == HotspotType::CONDITION_VAR ||
			HotspotType_ == HotspotType::OPERATION_VAR )
		{
			std::vector< BcU32 > VarOptions;

			MaVec2d Position( 0.0f, 0.0f );
			MaVec2d TotalSize( 0.0f, 0.0f );

			if( HotspotType_ == HotspotType::CONDITION_VAR )
			{
				auto FoundCondIt = std::find_if( ConditionEntries_.begin(), ConditionEntries_.end(),
					[ & ]( const GaRobotCommandEntry& Entry )
					{
						return Entry.Name_ == Program_[ SelectedID_ ].Condition_;
					} );

				if( FoundCondIt != ConditionEntries_.end() )
				{
					VarOptions = FoundCondIt->VarOptions_;
				}
			}
			else if( HotspotType_ == HotspotType::OPERATION_VAR )
			{
				auto FoundCondIt = std::find_if( OperationEntries_.begin(), OperationEntries_.end(),
					[ & ]( const GaRobotCommandEntry& Entry )
					{
						return Entry.Name_ == Program_[ SelectedID_ ].Operation_;
					} );

				if( FoundCondIt != OperationEntries_.end() )
				{
					VarOptions = FoundCondIt->VarOptions_;
				}
			}

			for( BcU32 Idx = 0; Idx < VarOptions.size(); ++Idx )
			{
				Hotspot HSSelection = 
				{
					Position * PanelOffset, MaVec2d(),
					VarOptions[ Idx ],
					HotspotType_ == HotspotType::CONDITION_VAR ?
						HotspotType::CONDITION_VAR_SELECTION : HotspotType::OPERATION_VAR_SELECTION
				};

				RsColour Colour = RsColour::BLACK;

				if( HSSelection.ID_ == LastHighlightedHotspot_.ID_ &&
					HSSelection.Type_ == LastHighlightedHotspot_.Type_ )
				{
					Colour = RsColour::GREEN;
				}

				BcF32 FontSize = Extents.y() * 0.5f;

				BcChar Buffer[ 1024 ];
				BcSPrintf( Buffer, "%u", HSSelection.ID_ );
				auto Size = Font_->draw( Canvas_, Position, FontSize, Buffer, Colour, BcFalse, 21 );

				Position.y( Position.y() + FontSize );

				TotalSize.x( std::max( Size.x(), TotalSize.x() ) );
				TotalSize.y( TotalSize.y() + FontSize );

				HSSelection.Extents_ = Size;

				Hotspots.push_back( HSSelection );
			}

			Canvas_->setMaterialComponent( Material_ );
			Canvas_->drawSprite( MaVec2d( 0.0f, 0.0f ), TotalSize, 0, RsColour( 1.0f, 1.0f, 1.0f, 0.8f ), 20 );

		}

		Canvas_->popMatrix();
	}


	// Handle mouse events.
	MaVec2d MouseHoverPos( MouseMoveEvent_.NormalisedX_ * Width, MouseMoveEvent_.NormalisedY_ * Height );
	for( int Idx = Hotspots.size() - 1; Idx >= 0; --Idx )
	{
		const auto& Hotspot( Hotspots[ Idx ] );
		if( MouseHoverPos.x() > Hotspot.Position_.x() && 
			MouseHoverPos.y() > Hotspot.Position_.y() &&
			MouseHoverPos.x() < ( Hotspot.Position_.x() + Hotspot.Extents_.x() ) && 
			MouseHoverPos.y() < ( Hotspot.Position_.y() + Hotspot.Extents_.y() ) )
		{
			LastHighlightedHotspot_ = Hotspot;
			break;
		}
	}


	for( const auto& MouseEvent : MouseEvents_ )
	{
		MaVec2d MousePos( MouseEvent.NormalisedX_ * Width, MouseEvent.NormalisedY_ * Height );

		for( int Idx = Hotspots.size() - 1; Idx >= 0; --Idx )
		{
			const auto& Hotspot( Hotspots[ Idx ] );
			if( MousePos.x() > Hotspot.Position_.x() && 
				MousePos.y() > Hotspot.Position_.y() &&
				MousePos.x() < ( Hotspot.Position_.x() + Hotspot.Extents_.x() ) && 
				MousePos.y() < ( Hotspot.Position_.y() + Hotspot.Extents_.y() ) )
			{
				onClick( Hotspot, MousePos );
				break;
			}
		}
		MouseEvents_.clear();
	}

	Hotspots.clear();


	Canvas_->popMatrix();
}

//////////////////////////////////////////////////////////////////////////
// onClick
void GaWorldComponent::onClick( const Hotspot& ClickedHotspot, MaVec2d MousePosition )
{
	BcBool UpdateRobotProgram = BcFalse;

	switch( ClickedHotspot.Type_ )
	{
	case HotspotType::STATE_MAIN:
		GuiState_ = GuiState::MAIN;
		break;

	case HotspotType::STATE:
		GuiState_ = GuiState::SELECTION_STATE;
		SelectionPosition_ = MousePosition;
		break;

	case HotspotType::CONDITION:
		GuiState_ = GuiState::SELECTION;
		SelectionPosition_ = MousePosition;
		break;

	case HotspotType::CONDITION_VAR:
		GuiState_ = GuiState::SELECTION_VAR;
		SelectionPosition_ = MousePosition;
		break;

	case HotspotType::OPERATION:
		GuiState_ = GuiState::SELECTION;
		SelectionPosition_ = MousePosition;
		break;

	case HotspotType::OPERATION_VAR:
		GuiState_ = GuiState::SELECTION_VAR;
		SelectionPosition_ = MousePosition;
		break;
		
	case HotspotType::STATE_SELECTION:
		GuiState_ = GuiState::MAIN;
		BcAssert( SelectedID_ < Program_.size() );
		BcAssert( ClickedHotspot.ID_ < 8 );
		Program_[ SelectedID_ ].State_ = ClickedHotspot.ID_;
		UpdateRobotProgram = BcTrue;
		break;

	case HotspotType::CONDITION_SELECTION:
		GuiState_ = GuiState::MAIN;
		BcAssert( SelectedID_ < Program_.size() );
		BcAssert( ClickedHotspot.ID_ < ConditionEntries_.size() );
		Program_[ SelectedID_ ].Condition_ = ConditionEntries_[ ClickedHotspot.ID_ ].Name_;
		UpdateRobotProgram = BcTrue;
		break;

	case HotspotType::CONDITION_VAR_SELECTION:
		GuiState_ = GuiState::MAIN;
		BcAssert( SelectedID_ < Program_.size() );
		Program_[ SelectedID_ ].ConditionVar_ = ClickedHotspot.ID_;
		UpdateRobotProgram = BcTrue;
		break;

	case HotspotType::OPERATION_SELECTION:
		GuiState_ = GuiState::MAIN;
		BcAssert( SelectedID_ < Program_.size() );		
		BcAssert( ClickedHotspot.ID_ < OperationEntries_.size() );
		Program_[ SelectedID_ ].Operation_ = OperationEntries_[ ClickedHotspot.ID_ ].Name_;
		UpdateRobotProgram = BcTrue;
		break;

	case HotspotType::OPERATION_VAR_SELECTION:
		GuiState_ = GuiState::MAIN;
		BcAssert( SelectedID_ < Program_.size() );
		Program_[ SelectedID_ ].OperationVar_ = ClickedHotspot.ID_;
		UpdateRobotProgram = BcTrue;
		break;


	default:
		break;
	}

	if( UpdateRobotProgram )
	{
		PlayerRobot_->setProgram( Program_ );
	}

	SelectedID_ = ClickedHotspot.ID_;
	HotspotType_ = ClickedHotspot.Type_;
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

	auto spawnRobot = [ this, &Idx ]( MaVec3d Position, MaVec3d Rotation )->GaRobotComponent*
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

		++Idx;

		return RobotComponent;
	};

	PlayerRobot_ = spawnRobot( MaVec3d( -32.0f, 0.0f, 0.0f ), MaVec3d( 0.0f, 0.0f, 0.0f ) );
	EnemyRobot_ = spawnRobot( MaVec3d( 32.0f, 0.0f, 0.0f ), MaVec3d( 0.0f, 0.0f, 0.0f ) );

	if( PlayerRobot_ != nullptr )
	{
		// Test program.
		Program_.push_back( GaRobotOperation( 0, "cond_far_enemy", 1, "op_target_enemy", 24 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 25, "op_set_state", 2 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 24, "op_set_state", 1 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 2, "op_avoid_attack", 32 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 0, "op_target_start", 0 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 2, "op_set_state", 0 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 2, "op_avoid_attack", 32 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 0, "op_avoid_attack", 32 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 0, "op_attack_a", 1 ) );
		Program_.push_back( GaRobotOperation( 0, "cond_never", 10, "op_set_state", 0 ) );

		PlayerRobot_->setProgram( Program_ );
		EnemyRobot_->setProgram( Program_ );
	}


#if 1
	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaWorldComponent, &GaWorldComponent::onMouseDown >( this );
	OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaWorldComponent, &GaWorldComponent::onMouseMove >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
#endif
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
